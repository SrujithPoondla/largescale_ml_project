#ifndef _test_handler_h__
#define _test_handler_h__ 1
#include "examples.h"
#include "model.h"
#include "pthread.h"
#include "timer.h"
#include <cstdlib>
#include <cmath>
#include "file_scanner.h"
#include "examples.h"

template <class T, class E>
void*  rmse_run_thread(void *p);

template <class T, class E>
  void* th_shuffler_thread(void *p);



template <class T, class E>
class test_handler {
  scanner<E> *_scanner;
  void init(E*, int nExamples);
public:
  double (*compute_loss)(const T&, const E &);
  // We do not free.
 test_handler(E* e, int nExamples, double (*_loss_function)(const T&, const E &)) : compute_loss(_loss_function) { init(e, nExamples); }
 test_handler(scanner<E>* s, double (*_compute_loss)(const T&, const E &)) : _scanner(s), compute_loss(_compute_loss) { }
  double test(T &m);
  double test_multithread(T &m, int nThreads);
  
  ~test_handler() {delete _scanner;}
  // friend functions for pthreads
  friend void* rmse_run_thread<T,E>(void *p);
  friend void* th_shuffler_thread<T,E>(void *p);
};


template <typename T>
void
create_test_handler(const char *szTestFile, struct example* &data, test_handler<T, struct example>* &t) {
  int nRows, nCols, nExamples;
  DEBUG_ONLY(timer load_timer; load_timer.start(); );
  DEBUG_ONLY(std::cout << "[TEST] Loading the test set" << std::endl);
  data = load_examples(szTestFile, nRows, nCols, nExamples);
  t = new test_handler<T,struct example>(data, nExamples, ( double(*)(const T&, const struct example&)) compute_loss);
  DEBUG_ONLY(load_timer.stop(); );
  DEBUG_ONLY(std::cout << "[TEST] Finished Loading " << nExamples << " examples in " << load_timer.elapsed() << "s" << std::endl);
}

#include "test_handler.hxx"

#endif
