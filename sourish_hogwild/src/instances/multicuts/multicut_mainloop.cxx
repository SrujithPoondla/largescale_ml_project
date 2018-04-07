#include "fvector.h"
#include "multicut_mainloop.h"
namespace multicut_namespace {  
  double clip(double v) { return std::min(std::max(v, 0.0),1.0); }
  double sign(double v) { if(v == 0.0) return 0.0; return (v > 0.0) ? 1.0 : -1.0; }
#ifdef _ATOMIC_LOCKING
void
acquire_two_locks(MultiCutModel *m, int r, int c) {
  if(r == c) {
    lock_error_check("Cut Lock", pthread_mutex_lock(&m->locks[r]));
  } else {        
    lock_error_check("Cut Lock", pthread_mutex_lock(&m->locks[std::min(r,c)]));
    lock_error_check("Cut Lock", pthread_mutex_lock(&m->locks[std::max(r,c)]));
  }
}

void 
release_two_locks(MultiCutModel *m, int r, int c) {
  lock_error_check("Cut Lock", pthread_mutex_unlock(&m->locks[r]));
  lock_error_check("Cut Lock", pthread_mutex_unlock(&m->locks[c]));
}
#endif

void*
cut_gradient_thread(freeforall_template::batch_gradient_parameters<MultiCutModel, struct example, struct p> *gti) {
  int id                  = gti->id;
  int nWorkers            = gti->nWorkers;
  int *perm               = gti->perm;
  const struct example* example = gti->examples;
  int nExamples           = gti->nExamples;
  MultiCutModel *model    = gti->model;
  double stepsize         = gti->stepsize;

  // as horrible as this is, roundrobin relies on the rounding
  // to be correct.
  int start_offset        = id*(nExamples/nWorkers);
  int last_offset         = (id+1)*(nExamples/nWorkers);
  int end_offset          = std::min(nExamples, last_offset);
  int i                   = start_offset;
  for(i = start_offset; i < end_offset; i++) {
    int pi = perm[i];
    int row_index = example[pi].row;
    int col_index = example[pi].col;
    double weight = example[pi].rating;
    GLOBAL_LOCK_ONLY(   lock_error_check("Cut Lock", pthread_mutex_lock(&model->global_lock)); );
    ATOMIC_LOCKING_ONLY( acquire_two_locks(model, row_index, col_index); );
    ROUNDROBIN_ONLY( roundrobin_acquire(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); );
#ifdef _ROUND_ROBIN
    FVector row_model  = model->get(row_index);
    FVector col_model  = model->get(col_index);
#else
    FVector &row_model = model->get(row_index);
    FVector &col_model = model->get(col_index);
#endif
    ROUNDROBIN_ONLY( roundrobin_release(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); );

    for(int d = model->get_dim() - 1; d >= 0; d--) {
      double   x = row_model.get(d);
      double   y = col_model.get(d);
      double err = weight*sign(x - y);
      VERBOSE_ONLY(printf("d=%d x=(%d,%f) y=(%d,%f) err=%f step=%f\n", d, row_index, x, col_index, y, err, stepsize);)
	row_model.set(d, x-stepsize*err);
      col_model.set(d, y+stepsize*err);     
    }
    row_model.simplex_project();
    col_model.simplex_project();
    ROUNDROBIN_ONLY( roundrobin_acquire(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); );
    ROUNDROBIN_ONLY( model->set(row_index, row_model); model->set(col_index, col_model); );
    ROUNDROBIN_ONLY( roundrobin_release(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); );

    GLOBAL_LOCK_ONLY(    lock_error_check("Unlock", pthread_mutex_unlock(&model->global_lock)); );    
    ATOMIC_LOCKING_ONLY( release_two_locks(model, row_index, col_index); );
  }
#ifdef _ROUND_ROBIN
  // to handle roundrobin
  while(i < last_offset) {
    roundrobin_acquire(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); 
    roundrobin_release(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); 
    roundrobin_acquire(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); 
    roundrobin_release(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); 
    i++;
  }
#endif
  return NULL;
}


double
compute_loss(const MultiCutModel &m, const example &e) {
    int dmax         = m.get_dim();
    int row_index    = e.row;
    int col_index    = e.col;
    double weight    = e.rating;

    double err_total = 0.0;
    for(int d = 0; d < dmax; d++) {
      double   x = m.get(row_index, d);
      double   y = m.get(col_index, d);
      err_total += weight*std::abs(x - y);
    }
    return err_total;
}


double
zero_one_loss(const MultiCutModel &m, const example &e) {
    int dmax         = m.get_dim();
    int row_index    = e.row;
    int col_index    = e.col;
    double weight    = e.rating;

    int row_max_index = 0, col_max_index  = 0;
    double row_max = m.get(row_index,0);
    double col_max = m.get(col_index,0);
    for(int d = 0; d < dmax; d++) {
      double rv = m.get(row_index, d);
      double cv = m.get(col_index, d);
      if(rv > row_max) { 
	row_max_index = d ;
	row_max       = rv;
      }
      if(cv > col_max) {
	col_max_index = d;
	col_max       = cv;
      }
    }

    double err_total = 0.0;
    if(row_max_index != col_max_index) {
      err_total = weight;
    }
    return err_total;
}


}
