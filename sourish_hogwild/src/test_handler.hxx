#include "file_scanner.h"
#include "examples.h"


// Does not copy
template <class T, class E> 
void
test_handler<T,E>::
init(E *exs, int nExamples)  {_scanner      = new memory_scanner<E>(nExamples, exs);}

template <class T, class E>
struct 
rmse_helper {
  // The Data to read
  const E* examples;
  int start, end;

  T &m; // the model
  test_handler<T,E> *t;
  double mse;
  rmse_helper(int _start, int _end, 
	      T &_m, test_handler<T,E> *_t, const E* e) : examples(e), m(_m)  {
    start = _start;
    end   = _end  ;
    mse   = 0.0   ;
    t     = _t;
  }
};

template <class T, class E>
void*
rmse_run_thread(void *p) {
  struct rmse_helper<T,E> *h = (struct rmse_helper<T,E> *) p;
  double error = 0.0;
  test_handler<T,E> *t = h->t;
  for(int i = h->start ; i < h->end; i++) {
    error += t->compute_loss(h->m, h->examples[i]);
  }
  h->mse = error;
  return NULL;
}


template <class T, class E>
double 
test_handler<T,E>::
test(T &m) {
  int nTestExamples = 0;
  DEBUG_ONLY(std::cout << "[Test] Starting...[single thread]" << std::endl;);  
  double error = 0.0;
  do {
    int nExamples = 0;
    const E* exs = _scanner->get_active_segment(nExamples);
    nTestExamples += nExamples;
    for(int i = 0 ; i < nExamples; i++) {
      error += compute_loss(m, exs[i]);
    }
    int next_examples;
    _scanner->retrieve_next_segment(next_examples);
  } while(!_scanner->completed_scan());

  double rmse = sqrt(error)/( sqrt((double) nTestExamples)) ;
  DEBUG_ONLY(std::cout << "[Test] RMSE: " << rmse << std::endl;);
  return rmse;
}

template<class T, class E>
void* th_shuffler_thread(void *p) {
  int _temp = 0;
  ((test_handler<T,E>*) p)->_scanner->retrieve_next_segment(_temp);
  return NULL;
}


template<class T, class E>
double 
test_handler<T,E>::
test_multithread(T &m, int nThreads) {
  DEBUG_ONLY(std::cout << "[Test] Starting... [threads=" << nThreads << "]" << std::endl;);  
  if(nThreads == 1) {
    return test(m);
  }
  assert(nThreads > 1);
  int nWorkers = nThreads - 1; // 1 thread for IO
  double error = 0.0, waste_time =0.0;
  struct rmse_helper<T,E>* rmse_helpers[nWorkers];
  pthread_t helper_threads[nWorkers];
  pthread_t shuffler_thread;

  int nTestExamples = 0;
  do {
    int nExamples  = 0;
    const E* exs         = _scanner->get_active_segment(nExamples);
    nTestExamples += nExamples;
    int bucket_size = nExamples / nWorkers;
    
    // Kickoff the IO thread
    pthread_create( &shuffler_thread, NULL, th_shuffler_thread<T,E>, (void*) this);

    if(bucket_size == 0) {
      // This can happen if nExamples < nWorkers
      int start = 0;
      int end   = nExamples;
      rmse_helper<T,E> *r = new rmse_helper<T,E>(start, end, m, this, exs);
      rmse_run_thread<T,E>((void*) r);
      error += r->mse;
      delete r;          
    } else {
      // common case
      // Kickoff the worker threads
      for(int i = 0; i < nWorkers; i++) {
	int start = bucket_size * i;
	int end   = std::min(bucket_size * (i+1), nExamples);
	rmse_helpers[i] = new rmse_helper<T,E>(start, end, m, this, exs);
	pthread_create( &(helper_threads[i]), NULL, 
		    rmse_run_thread<T,E>, rmse_helpers[i]);
      }

      // Join on the workers
      for(int i = 0 ; i < nWorkers; i++) {     
	pthread_join( helper_threads[i], NULL);
	error += rmse_helpers[i]->mse;
	delete rmse_helpers[i];
      }
    }
    // Join on the shuffler
    timer waste_timer(true);
    pthread_join( shuffler_thread, NULL );
    waste_time += waste_timer.stop_elapsed();

  } while(!_scanner->completed_scan());


  double rmse = sqrt(error)/( sqrt((double) nTestExamples)) ;
  DEBUG_ONLY(std::cout << "[Test] RMSE: " << rmse << " [IO delay: " << waste_time << "]" << std::endl;);
  return rmse;
}

