#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "global_macros.h"
#include "timer.h"
#include "examples.h"
#include "fvector.h"
#include "pthread.h"
#include "simple_random.h"
#include "freeforall_template.h"
#include "lock_util.h"
#include "freeforall_template_util.h"

#include <vector>
using namespace std;
class FVector;

namespace freeforall_template {
  // Only one of these defines should ever be active
  // OR ATOMIC_LOCKING

  
  template <class M, class E, class P>
  struct gradient_thread_dispatch {
    freeforall_template::batch_gradient_parameters<M,E,P>* gti;
    void* (*batch_gradfunction)(freeforall_template::batch_gradient_parameters<M,E,P> *);
    gradient_thread_dispatch(freeforall_template::batch_gradient_parameters<M,E,P>* _gti,
			     void* (*_batch_gradfunction)(freeforall_template::batch_gradient_parameters<M,E,P> *)) :
      gti(_gti), batch_gradfunction(_batch_gradfunction) { }
  };

  template <class M, class E, class P>
  void *
  gradient_thread_dispatch_function(void *params) {  
    struct gradient_thread_dispatch<M,E,P>* gtd  = (struct gradient_thread_dispatch<M,E,P>*) params;
    struct freeforall_template::batch_gradient_parameters<M,E,P>* gti = gtd->gti;        
    return gtd->batch_gradfunction(gti);
  }

  template <class M, class E, class P>
  M*
  freeforall(freeforall_template::training_parameters <M,E,P> tp) {
    simple_random _rand(tp.seed);
    test_handler<M, E> *t     = tp.t;
    int nEpochs               = tp.nEpochs;
    double initial_step_size  = tp.initial_step_size; 
    double step_diminish      = tp.step_diminish;  
    int nShufflers            = tp.nShufflers;
    int nWorkers              = tp.nSplits; 

    // Locking  setup
    GLOBAL_LOCK_ONLY   (cout << "Locking is Global Mutex" << endl;      );
    ATOMIC_LOCKING_ONLY(cout << "Locking is Atomic" << endl;            );

    // parameters
    double cur_step_size = initial_step_size;
    cout << "# [freeforall_generic]  Parameters:" << endl;
    cout << "# [freeforall_generic]  nepochs=" << nEpochs << " nSplits=" << tp.nSplits << " nShufflers=" << nShufflers << endl;
    cout << "# [freeforall_generic]  initial_stepsize=" << initial_step_size << " step_diminish=" << step_diminish << endl;
    timer whole_jellyfish(true);

    M *m = tp.initial_guess;
    

    scanner<E> *example_scanner = tp.examples;
    // TODO: print something about the scanner type
    // cout << "# [freeforall]  nExamples=" << nExamples << endl;
    
    // Create the Test Handler
    assert(example_scanner != NULL);
    // TODO: bring back training;
    test_handler<M, E>  train_tester(example_scanner, tp.loss_function);
    
    assert(nShufflers == 1); // hardcoded below still
    permutation_buffer     *shared_perm[nShufflers+1];
    struct permute_thread_info<E>* ptis[nShufflers+1];
    for(int i = 0; i < nShufflers+1; i++) { 
      shared_perm[i]      = new permutation_buffer(_rand, example_scanner->get_max_size());
      ptis[i]             = new struct permute_thread_info<E>(shared_perm[i], example_scanner);
    }
    
    freeforall_template::batch_gradient_parameters<M,E,P>*    wtis[nWorkers];
    freeforall_template::gradient_thread_dispatch<M,E,P>* dispatch[nWorkers];
    for(int i = 0; i< nWorkers; i++) {
      // The example pointer and the nExamples
      wtis[i]     = new struct batch_gradient_parameters<M,E,P>(i, nWorkers, m, tp.params);
      dispatch[i] = new struct gradient_thread_dispatch<M,E,P>(wtis[i], tp.batch_gradfunction);
    }
    
    // Need to fetch and run one step of the shuffler here
    permute_thread<E>( (void*) ptis[0] );

    DEBUG_ONLY(cout  << "Model built. Ready to start." << endl;);

    int batch_offset = 0;
    for(int epoch = 0; epoch < nEpochs; epoch++) {
      timer epoch_timer(true);
      pthread_t shuffler_t;
      double perm_time = 0.0;
      do {
	int current_examples = 0;
	const E* ex          = example_scanner->get_active_segment(current_examples);
	int cur_batch        = batch_offset % (nShufflers + 1);
	int next_batch       = (batch_offset + 1) % (nShufflers + 1);
      
	pthread_check ("create shuffler thread", pthread_create( &shuffler_t, NULL, permute_thread<E> , (void*) ptis[next_batch]));

	pthread_t workers[nWorkers];
	for(int i = 0; i < nWorkers; i++) {
	  int size_perm = 0;
	  wtis[i]->nExamples = current_examples;
	  wtis[i]->examples  = ex;
	  wtis[i]->stepsize  = cur_step_size;
	  wtis[i]->perm      = shared_perm[cur_batch]->get_permutation(size_perm);
	  if(size_perm != current_examples) {
	    int q = 0;
	    shared_perm[next_batch]->get_permutation(q);
	    std::cout << "size_permp=" << size_perm << " q=" << q << " current_examples=" << current_examples << std::endl;
	    assert(size_perm == current_examples);
	  }
	  pthread_check("worker create", pthread_create( &workers[i], NULL, gradient_thread_dispatch_function<M,E,P>, (void*) dispatch[i]));
	}
	// Shuffler thread Join
	for(int i = 0; i < nWorkers; i++) {pthread_join(workers[i], NULL);}
      
	timer shuffler_wait_timer(true);
	pthread_join( shuffler_t, NULL );      
	perm_time += shuffler_wait_timer.stop_elapsed(); 
	batch_offset ++;
      } while(!example_scanner->completed_scan()); // End of the scan
      // bookkeeping on end of each epoch
      cur_step_size *= step_diminish;
  
      // End Epoch
      double epoch_duration = epoch_timer.stop_elapsed();
      DEBUG_ONLY(cout << "[TRAIN] epoch " << epoch + 1 << " took " << epoch_timer << " running time: " << whole_jellyfish << endl; );

      // Now compute the RMSEs
      double train_rmse = 0.0, test_rmse = 0.0;
      train_rmse = (nWorkers > 1) ? train_tester.test_multithread(*m, nWorkers) : train_tester.test(*m);
      DEBUG_ONLY(cout << "[TRAIN] " << endl; );
      if(t) test_rmse = (nWorkers > 1) ? t->test_multithread(*m, nWorkers) : t->test(*m);
      

      cout << "[Epoch Stats] epoch: " << epoch + 1 << " epoch_time: " << epoch_duration << " total_time: " << whole_jellyfish << " perm_time " << perm_time << " train_rmse: " << train_rmse << " test_rmse: " << test_rmse << endl;      
    }
    // Cleanup code.
    DEBUG_ONLY(cout << "[freeforall] finished in " << whole_jellyfish << endl;);
    return m;
  }

}
