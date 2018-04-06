#include "freeforall_template.h"
#include <set>
#include "fvector.h"

namespace multicut_namespace {
  struct p {int g;}; // disused.

  class MultiCutModel {
    int n, dim;
    std::set<int> Terminals;
    FVector *weights;
  public:
    GLOBAL_LOCK_ONLY  (pthread_mutex_t global_lock;);    
    ATOMIC_LOCKING_ONLY( pthread_mutex_t *locks; );
    ROUNDROBIN_ONLY( pthread_mutex_t global_lock; pthread_cond_t cond; int turn_value; )
    
    MultiCutModel(int _n, std::set<int> _terminals) : n(_n), Terminals(_terminals) {
      GLOBAL_LOCK_ONLY( lock_error_check("Global Lock Initialize", pthread_mutex_init(&global_lock, NULL)) ; );
      ATOMIC_LOCKING_ONLY( locks = new pthread_mutex_t[n]; for(int i = 0; i < n; i++) { pthread_mutex_init(&locks[i], NULL); } );
      ROUNDROBIN_ONLY( lock_error_check("Global Lock Initialize", pthread_mutex_init(&global_lock, NULL)) ; );
      ROUNDROBIN_ONLY( lock_error_check("Global Cond Initialize", pthread_cond_init(&cond, NULL)) ; );

      dim = (int ) Terminals.size();
      ::FVector::_default_n = dim;
      weights = ::new FVector[n];

      std::cout << "Beginning simplex projects" << std::endl;
      for(int j = 0; j < n; j++) {weights[j].simplex_project();}

      std::cout << "Zeroing " << dim << " terminals" << std::endl;      
      int counter = 0;
      for(std::set<int>::iterator i = Terminals.begin(); i != Terminals.end(); i++) {
	weights[*i].zero();
	weights[*i].set(counter,1.0);
	counter++;
      }
    }

    ~MultiCutModel() {
      delete weights;
      GLOBAL_LOCK_ONLY  ( lock_error_check("Global lock destroy", pthread_mutex_destroy(&global_lock) ); );  
      ATOMIC_LOCKING_ONLY( delete [] locks; );
    }
    // Accessors
    void set(int i, int j, double w) {
      if(Terminals.find(i) == Terminals.end()) { weights[i].set(j,w); }	
    }
    FVector& get(int i) { return weights[i]; }
    void set(int i, const FVector &x) { weights[i] = x; }
    double get(int i, int j) const { assert(i >= 0 && i < n); return weights[i].get(j); }
    void project(int i) {
      if(Terminals.find(i) == Terminals.end()) { weights[i].simplex_project(); }	
    }
    int get_dim() const { return dim; }  
  };

  void* cut_gradient_thread(freeforall_template::batch_gradient_parameters<MultiCutModel, struct example, struct p> *gti);
  double compute_loss(const MultiCutModel &m, const example &e);  
  double zero_one_loss(const MultiCutModel &m, const example &e);
}


