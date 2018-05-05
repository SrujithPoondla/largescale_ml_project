#include "global_macros.h"
#include "timer.h"
#include "examples.h"
#include "fvector.h"
#include "pthread.h"
#include "simple_random.h"
#include "freeforall_template.h"
#include "lock_util.h"
#include "sparse_vector.h"

namespace sparsesvm_templates {

  struct svm_model {
    ::FVector w;
    ROUNDROBIN_ONLY( pthread_mutex_t global_lock; pthread_cond_t cond; volatile int turn_value; );
    ATOMIC_LOCKING_ONLY( pthread_spinlock_t *locks; );
  svm_model(::FVector _w, double _bias) : w(_w) { 
    ATOMIC_LOCKING_ONLY( locks = new pthread_spinlock_t[w.get_size()]; for(int i = 0; i < w.get_size(); i++) { lock_error_check("create in svm", pthread_spin_init(&locks[i], PTHREAD_PROCESS_PRIVATE)); } );
    ROUNDROBIN_ONLY( lock_error_check("Global Lock Initialize", pthread_mutex_init(&global_lock, NULL)) ; );
    ROUNDROBIN_ONLY( lock_error_check("Global Cond Initialize", pthread_cond_init(&cond, NULL)) ; );
  } 
  svm_model(int dim) : w(::FVector(dim))  { 
    ATOMIC_LOCKING_ONLY( locks = new pthread_spinlock_t[w.get_size()]; for(int i = 0; i < w.get_size(); i++) { lock_error_check("Create in svm (dim)", pthread_spin_init(&locks[i], PTHREAD_PROCESS_PRIVATE)); } );
    ROUNDROBIN_ONLY( lock_error_check("Global Lock Initialize", pthread_mutex_init(&global_lock, NULL)) ; );
    ROUNDROBIN_ONLY( lock_error_check("Global Cond Initialize", pthread_cond_init(&cond, NULL)) ; );
  }
    ~svm_model() {
      ATOMIC_LOCKING_ONLY( for(int i = 0; i < w.get_size(); i++) { lock_error_check("Destroying in svm", pthread_spin_destroy(&locks[i])); } );
    }
  };

  
  struct sparse_example {
    SparseVector *sv;
    double value;
  sparse_example(SparseVector *_sv, double v) : sv(_sv), value(v) { }
  sparse_example() : sv(NULL), value(0.0) {} 
  };
  struct sparse_example*parse_examples(int nExamples, struct example*e, int &nSparseExamples);

  double compute_loss(const struct svm_model& m, const sparse_example &e);

  struct svm_p {
    double mu;
    double *degrees, nDims; // the degree of every edge. 
  };

  void compute_degrees(struct svm_p &x, const sparse_example &e);
  void* sparse_svm_gradient(freeforall_template::batch_gradient_parameters<svm_model, struct sparse_example, struct svm_p> *gti);
};
