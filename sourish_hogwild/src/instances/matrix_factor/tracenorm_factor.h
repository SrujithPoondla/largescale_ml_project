#include "fvector.h"
#include "examples.h"
#include "freeforall_template.h"

namespace tracenorm_factor {
struct tracenorm_model {
  FVector *L, *R;
  double mean;
  ROUNDROBIN_ONLY( pthread_mutex_t global_lock; pthread_cond_t cond; volatile int turn_value; );
  tracenorm_model(int nRows, int nCols, int max_rank, double _mean) {
    ::FVector::_default_n = max_rank; // this makes allocation easy. 
    L = new ::FVector[nRows];
    R = new ::FVector[nCols];
    mean = _mean;

    ROUNDROBIN_ONLY( lock_error_check("Global Lock Initialize", pthread_mutex_init(&global_lock, NULL)) ; );
    ROUNDROBIN_ONLY( lock_error_check("Global Cond Initialize", pthread_cond_init(&cond, NULL)) ; );
    ROUNDROBIN_ONLY( turn_value = 0; );
  }
  ~tracenorm_model() {
      ROUNDROBIN_ONLY   ( lock_error_check("Global lock destroy", pthread_mutex_destroy(&global_lock) ); );  
      ROUNDROBIN_ONLY  ( lock_error_check("Global lock destroy", pthread_cond_destroy(&cond) ); );  
  }
};

struct tracenorm_parameters {
  int nExamples, max_rank;
  int *L_degree, *R_degree;
  double mean, mu;
  // Expects to have the map applied below.
  void setup(int nRows, int nCols, int _nExamples) { 
    nExamples = _nExamples;
    L_degree = new int[nRows]; memset(L_degree, 0, sizeof(int)*nRows);
    R_degree = new int[nCols]; memset(R_degree, 0, sizeof(int)*nCols);
    mean     = 0.0;
  }
  tracenorm_parameters() {
    L_degree = NULL;
    R_degree = NULL;    
    nExamples = -1;
  }
  // trace_norm_parameters(int *_L_degree, int *_R_degree, double _mean) : L_degree(_L_degree), R_degree(_R_degree), mean(_mean)  { }
};

 void  parameter_map(struct tracenorm_parameters &tnp, const struct example &e);
 void* tracenorm_gradient_thread(freeforall_template::batch_gradient_parameters<struct tracenorm_model, struct example, struct tracenorm_parameters> *gti);
 double compute_loss(const tracenorm_model &m, const example &e);
}
