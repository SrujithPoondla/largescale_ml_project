#include "fvector.h"
#include "cut_mainloop.h"
namespace cut_namespace {  
  double clip(double v) { return std::min(std::max(v, 0.0),1.0); }

#ifdef _ATOMIC_LOCKING
void
acquire_two_locks(CutModel *m, int r, int c) {
  if(r == c) {
    lock_error_check("Cut Lock", pthread_mutex_lock(&m->locks[r]));
  } else {        
    lock_error_check("Cut Lock", pthread_mutex_lock(&m->locks[std::min(r,c)]));
    lock_error_check("Cut Lock", pthread_mutex_lock(&m->locks[std::max(r,c)]));
  }
}

void 
release_two_locks(CutModel *m, int r, int c) {
  lock_error_check("Cut Lock", pthread_mutex_unlock(&m->locks[r]));
  lock_error_check("Cut Lock", pthread_mutex_unlock(&m->locks[c]));
}
#endif

#ifdef _ROUNDROBIN_LOCKING
  int turn_value = 0;
#endif
void*
cut_gradient_thread(freeforall_template::batch_gradient_parameters<CutModel, struct example, struct p> *gti) {
  int id                  = gti->id;
  int nWorkers            = gti->nWorkers;
  int *perm               = gti->perm;
  const struct example* example = gti->examples;
  int nExamples           = gti->nExamples;
  CutModel *model         = gti->model;
  double stepsize         = gti->stepsize;

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
    GLOBAL_RWLOCK_ONLY( lock_error_check("Read vectors", pthread_rwlock_rdlock(&global_lock)); );
    double   x = model->get(row_index);
    double   y = model->get(col_index);
    GLOBAL_RWLOCK_ONLY( lock_error_check("Unlock Readlock", pthread_rwlock_unlock(&global_lock)); );
    
    double err = weight*(x - y);
    
    
    GLOBAL_RWLOCK_ONLY( lock_error_check("Read Tracenorm", pthread_rwlock_rdlock(&global_lock)); );  
    ROUNDROBIN_ONLY( roundrobin_acquire(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); );
    model->set(row_index, clip(x-stepsize*err));
    model->set(col_index, clip(y+stepsize*err));
    ROUNDROBIN_ONLY( roundrobin_release(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); );

    GLOBAL_RWLOCK_ONLY(  lock_error_check("Unlock Writelock Tracenorm", pthread_rwlock_unlock(&global_lock)); );
    GLOBAL_LOCK_ONLY(    lock_error_check("Unlock", pthread_mutex_unlock(&model->global_lock)); );    
    ATOMIC_LOCKING_ONLY( release_two_locks(model, row_index, col_index); );
  }
#ifdef _ROUND_ROBIN
  // to handle roundrobin
  while(i < last_offset) {
    roundrobin_acquire(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); 
    roundrobin_release(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); 
    i++;
  }
#endif
  return NULL;
}


double
compute_loss(const CutModel &m, const example &e) {
  int row_index = e.row;
  int col_index = e.col;
  double weight = e.rating;
  double   x = m.get(row_index);
  double   y = m.get(col_index);
  double err = weight*(x - y);
  return abs(err);
}

}
