#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "sparse_svm_generic.h"
// NB: We assume the values are \pm 1.
using namespace std;
class FVector;

#ifdef __APPLE__
// This is just to redefine clock_gettime.
#include "timer.h"
#endif

namespace sparsesvm_templates {

  double 
  compute_loss(const struct svm_model& m, const sparse_example &e) {
    double wx     = std::max(1 - FVector::dot(*(e.sv), m.w)*e.value, 0.0);
    return wx;
  }


  void compute_degrees(struct svm_p &x, const sparse_example &e) {
    SparseVector const &sv  = *e.sv;
    for(int i = sv.nValues - 1; i >= 0; i--) {
      x.degrees[sv.indexes[i]] += 1.0;
    }
  }


  struct sparse_example*
  parse_examples(int nExamples, struct example * e, int &nVectors) {
    // First find the number of examples.
    nVectors = 0;
    int last_row = e[0].row;
    for(int i = 0; i < nExamples; i++) {
      if(last_row != e[i].row) nVectors++;
      last_row = e[i].row;
    }

    // Assume sorted by (row,col,rating)
    // if col == -1, then this is the value
    struct sparse_example * ret = new struct sparse_example[nVectors];
    int i = 0, curVec = 0;
    const int local_max_size = 65536;
    double dCache[local_max_size];
    int    iCache[local_max_size];
    double rating = 0.0;
    while(i < nExamples) {
      // find the current span
      int span = 0, delay = 0;
      last_row = e[i].row;
      while(i+span < nExamples && e[i+span].row == last_row) span++;
      assert(span < local_max_size);
      
      // Copy out.
      for(int k = 0; (k < span); k++) {
	if(e[i+k].col < 0) {
	  assert(delay == 0);
	  rating = e[i+k].rating;
	  delay  = 1;
	} else {
	  dCache[k-delay] = e[i+k].rating;
	  iCache[k-delay] = e[i+k].col;
	  //std::cerr << "  -- " << e[i+k].col << std::endl;
	}
      }
      assert(delay == 1 && (span - 1) > 0);
      ret[curVec].sv    = new SparseVector(dCache, iCache, span - 1);
      ret[curVec].value = rating;
      // Advance
      i += span;
      curVec++;
    }
    return ret;
  }
  // OR ATOMIC_LOCKING

#ifdef _DELAY_ONLY
inline
void 
busy_wait(long nsec) {
  struct timespec _start, _stop;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_start);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_stop);
  while( ((_stop.tv_sec - _start.tv_sec)*((long)(1e9))  + (double) (_stop.tv_nsec - _start.tv_nsec)) < nsec) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_stop);
  }; 
}
#endif
  // WARNING: This relies on the indexes being in sorted order!
void
acquire_locks_in_order(pthread_mutex_t *locks, int *indexes, int nIndexes) {
  for(int i = nIndexes - 1; i >= 0; i--) {
    lock_error_check("In order locks", pthread_mutex_lock(&locks[indexes[i]]));
  }
}

void
release_locks_in_order(pthread_mutex_t *locks, int *indexes, int nIndexes) {
  for(int i = nIndexes - 1; i >= 0; i--) {
    lock_error_check("In order unlocks", pthread_mutex_unlock(&locks[indexes[i]])); 
  }
}


void*
sparse_svm_gradient(freeforall_template::batch_gradient_parameters<svm_model, struct sparse_example, struct svm_p> *gti) {
  // read the params
  int id               = gti->id;
  int nWorkers         = gti->nWorkers;
  svm_model *m         = gti->model;
  int *perm            = gti->perm;
  const struct sparse_example* example = gti->examples;
  int nExamples        = gti->nExamples;
  double cur_step_size = gti->stepsize;
  double mu            = gti->params->mu;
  double *degrees      = gti->params->degrees;

  // NB: the rounding is important for roundrobin.
  int start_offset     = id*(nExamples/nWorkers);
  int end_offset       = (id+1)*(nExamples/nWorkers);
  ROUNDROBIN_ONLY(int last_offset      = std::min(nExamples, end_offset););
  int i                = start_offset;
  DEBUG_ONLY(cout << "start=" << start_offset << " -- " << end_offset << " " << nExamples << " nWorkers=" << nWorkers << " id=" << id << endl;);
#if  defined(_ROUND_ROBIN) || defined(_ATOMIC_LOCKING)
  FVector w_local = m->w;
#endif
  for(i = start_offset; i < end_offset; i++) {
    int pi = perm[i];
    SparseVector &sv = *(example[pi].sv);
    // Global Locking Protocol. Grab whole time.
    GLOBAL_LOCK_ONLY(   lock_error_check("Read Global Lock", pthread_mutex_lock(&global_lock)); );

    ROUNDROBIN_ONLY(roundrobin_acquire(id, &m->turn_value, nWorkers, &m->global_lock, &m->cond); );
#if  defined(_ROUND_ROBIN) || defined(_ATOMIC_LOCKING)
    w_local.read_mask(m->w, sv);
#else
    FVector &w_local = m->w;
#endif
    ROUNDROBIN_ONLY(roundrobin_release(id, &m->turn_value, nWorkers, &m->global_lock, &m->cond); );

    double wxy       = FVector::dot(sv, w_local)*example[pi].value;  
    if(wxy < 1) { // hinge is active.
      double e       = cur_step_size * example[pi].value;
      w_local.scale_and_add(sv,e);
    }
    DELAY_ONLY( if(_is_gradient_delay) busy_wait(tsgradient_delay.tv_nsec); );
      
    ROUNDROBIN_ONLY(roundrobin_acquire(id, &m->turn_value, nWorkers, &m->global_lock, &m->cond););
    //ATOMIC_LOCKING_ONLY( acquire_locks_in_order(m->locks, sv.indexes, sv.nValues); )
    for(int i = sv.nValues - 1; i>= 0; i--) {
      int j = sv.indexes[i];
      ATOMIC_LOCKING_ONLY(lock_error_check("In order locks", pthread_spin_lock(&m->locks[j]));)
      m->w.set(j, w_local.get(j)* (1 - cur_step_size*mu/degrees[j]));
      ATOMIC_LOCKING_ONLY(lock_error_check("In order locks", pthread_spin_unlock(&m->locks[j]));)
    }
    ROUNDROBIN_ONLY(roundrobin_release(id, &m->turn_value, nWorkers, &m->global_lock, &m->cond); );
    //ATOMIC_LOCKING_ONLY( release_locks_in_order(m->locks, sv.indexes, sv.nValues); )

    // For delay experiments we compute here.
    // now handle the regularizer
    // w_i = w_i - mu*w_i/d_i = (1 - mu*w_i/d_i) w_i

    GLOBAL_LOCK_ONLY(    lock_error_check("Unlock", pthread_mutex_unlock(&global_lock)); );
  }
  //std::cerr << "EXIT " << id << " to " << m->turn_value << std::endl;
#ifdef _ROUND_ROBIN
  // to handle roundrobin
  while(i < last_offset) {
    //std::cerr << "Burning off extra " << i << " to " << last_offset << std::endl;
    // read the model
    roundrobin_acquire(id, &m->turn_value, nWorkers, &m->global_lock, &m->cond); 
    roundrobin_release(id, &m->turn_value, nWorkers, &m->global_lock, &m->cond); 
    // write back
    roundrobin_acquire(id, &m->turn_value, nWorkers, &m->global_lock, &m->cond); 
    roundrobin_release(id, &m->turn_value, nWorkers, &m->global_lock, &m->cond); 
    i++;
  }
#endif
  DEBUG_ONLY(cout << " part id=" << id << " on " << (end_offset - start_offset) << " examples " << endl;);
  //delete gti;
  return NULL;
}
}
