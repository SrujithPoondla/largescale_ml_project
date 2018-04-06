#include "tracenorm_factor.h"

// *****************************
// *THREADING
namespace tracenorm_factor {
void 
parameter_map(struct tracenorm_parameters &tnp, const struct example &e) {
  tnp.L_degree[e.row] ++;
  tnp.R_degree[e.col] ++;
  tnp.mean += e.rating/((double) tnp.nExamples);
}

double 
compute_loss(const tracenorm_model &m, const example &e) {
  double err     = FVector::dot(m.L[e.row],m.R[e.col]) + m.mean - e.rating;
  return err*err;
}



void*
tracenorm_gradient_thread(freeforall_template::batch_gradient_parameters<struct tracenorm_model, struct example, struct tracenorm_parameters> *gti) {
  // read the worker params
  int id               = gti->id;
  int nWorkers         = gti->nWorkers;
  int *perm            = gti->perm;
  const struct example* example = gti->examples;
  int nExamples        = gti->nExamples;
  double stepsize      = gti->stepsize;

  // setup the fixed parameters
  double mu            = gti->params->mu;
  int *L_degree        = gti->params->L_degree;
  int *R_degree        = gti->params->R_degree;  

  // setup the model parameters
  struct tracenorm_model*model = gti->model;
  FVector *L                   = model->L;
  FVector *R                   = model->R;
  double mean                  = model->mean;

  // get to work.
  DEBUG_ONLY(timer part_timer(true););
  FVector _tempL;
 
  int start_offset     = id*(nExamples/nWorkers);
  int last_offset      = (id+1)*(nExamples/nWorkers);
  int end_offset       = min(nExamples, last_offset);
  int i                = start_offset;
  DEBUG_ONLY(cout << "start=" << start_offset << " -- " << end_offset << " " << nExamples << " nWorkers=" << nWorkers << " id=" << id << endl;);

  for(i = start_offset; i < end_offset; i++) {
    int pi = perm[i];
    int row_index = example[pi].row;
    int col_index = example[pi].col;
    double rating = example[pi].rating;
     
    // Global Locking Protocol. Grab whole time.
    GLOBAL_LOCK_ONLY(   lock_error_check("Read Tracenorm", pthread_mutex_lock(&global_lock)); );
    GLOBAL_RWLOCK_ONLY( lock_error_check("Read Tracenorm", pthread_rwlock_rdlock(&global_lock)); );

    ROUNDROBIN_ONLY( roundrobin_acquire(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); );


#ifdef _ROUND_ROBIN
    FVector Li  = L[row_index];
    FVector Rj  = R[col_index];
#else
    FVector &Li = L[row_index];
    FVector &Rj = R[col_index];
#endif
  
    ROUNDROBIN_ONLY( roundrobin_release(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); );
    ATOMIC_LOCKING_ONLY( acquire_two_locks( &(Li._lock), &(Rj._lock) ); );

    double err     = FVector::dot(Li,Rj) + mean - rating;
    GLOBAL_RWLOCK_ONLY( lock_error_check("Unlock Readlock Tracenorm", pthread_rwlock_unlock(&global_lock)); );

    double e       = -(stepsize * err);
    
    GLOBAL_RWLOCK_ONLY( lock_error_check("Read Tracenorm", pthread_rwlock_rdlock(&global_lock)); );      
    _tempL = Li;
    _tempL.scale(1 - mu*stepsize/((double) L_degree[row_index]));
    _tempL.scale_and_add(Rj,e);	 
    GLOBAL_RWLOCK_ONLY( lock_error_check("Unlock Readlock Tracenorm", pthread_rwlock_unlock(&global_lock)); );
    
     // Upgrade to write locks
    GLOBAL_RWLOCK_ONLY( lock_error_check("Write lock Tracenorm", pthread_rwlock_wrlock(&global_lock)); );
    Rj.scale(1 - mu*stepsize/((double) R_degree[col_index])); 
    Rj.scale_and_add(Li,e);	      
    // Store back L
    Li  = _tempL;

#ifdef _ROUND_ROBIN
    roundrobin_acquire(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); 
    L[row_index] = Li;
    R[col_index] = Rj;
    roundrobin_release(id, &model->turn_value, nWorkers, &model->global_lock, &model->cond); 
#endif
    GLOBAL_RWLOCK_ONLY(  lock_error_check("Unlock Writelock Tracenorm", pthread_rwlock_unlock(&global_lock)); );
    GLOBAL_LOCK_ONLY(    lock_error_check("Unlock", pthread_mutex_unlock(&global_lock)); );

    ATOMIC_LOCKING_ONLY( release_two_locks( &L[row_index]._lock, &R[col_index]._lock ); );
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
   
  DEBUG_ONLY(cout << " part id=" << id << " took " << part_timer << " on " << (end_offset - start_offset) << " examples " << endl;);  
  return NULL;
}
}
