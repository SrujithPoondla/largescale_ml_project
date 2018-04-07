#include "freeforall_template.h"
#include <set>
#include <algorithm>

namespace cut_namespace {
  struct p {int g;}; // disused.


  class CutModel {

    int n;
    std::set<int> S,T,U;
    double *weights;
  public:
    GLOBAL_LOCK_ONLY  (pthread_mutex_t global_lock;);    
    ATOMIC_LOCKING_ONLY( pthread_mutex_t *locks; );
    ROUNDROBIN_ONLY( pthread_mutex_t global_lock; pthread_cond_t cond; int turn_value; )
    CutModel(int _n, std::set<int> _S, std::set<int> _T) : n(_n), S(_S), T(_T) {
      GLOBAL_LOCK_ONLY( lock_error_check("Global Lock Initialize", pthread_mutex_init(&global_lock, NULL)) ; );
      ATOMIC_LOCKING_ONLY( locks = new pthread_mutex_t[n]; for(int i = 0; i < n; i++) { pthread_mutex_init(&locks[i], NULL); } );
      ROUNDROBIN_ONLY( lock_error_check("Global Lock Initialize", pthread_mutex_init(&global_lock, NULL)) ; );
      ROUNDROBIN_ONLY( lock_error_check("Global Cond Initialize", pthread_cond_init(&cond, NULL)) ; );

      weights = new double[n];
      std::set_union(S.begin(), S.end(), T.begin(), T.end(), insert_iterator< std::set<int> >(U, U.begin()));
      for(int i = 0; i < n; i++) {
	if(S.find(i) != S.end()) { weights[i] = 0; }
	else {
	  if(T.find(i) != T.end()) { weights[i] = 1; }
	  else { weights[i] = 0.5; }      }
      }
    }
    ~CutModel() {
      delete weights;
      GLOBAL_LOCK_ONLY  ( lock_error_check("Global lock destroy", pthread_mutex_destroy(&global_lock) ); );  
      ROUNDROBIN_ONLY   ( lock_error_check("Global lock destroy", pthread_mutex_destroy(&global_lock) ); );  
      ROUNDROBIN_ONLY  ( lock_error_check("Global lock destroy", pthread_cond_destroy(&cond) ); );  
      ATOMIC_LOCKING_ONLY( for(int i = 0; i < n; i++) pthread_mutex_destroy(&locks[i]); );
      ATOMIC_LOCKING_ONLY( delete [] locks; );
    }
    void set(int i, double w) {
      if(U.find(i) == U.end()) { weights[i] = w; }	
    }
    double get(int i) const { assert(i >= 0 && i < n); return weights[i]; }
  };

  void* cut_gradient_thread(freeforall_template::batch_gradient_parameters<CutModel, struct example, struct p> *gti);
  double compute_loss(const CutModel &m, const example &e);  

}


