#ifndef _freeforall_template_h
#define _freeforall_template_h
#include "test_handler.h"
#include "model.h"
#include "examples.h"
#include "training_params.h"
#include "file_scanner.h"
#include "freeforall_template_util.h"

namespace freeforall_template {

  template <class M, class E, class P>
  struct batch_gradient_parameters {
    int id, nWorkers;
    int *perm; // set after creation
    const E* examples; int nExamples; // Set after creation
    M* model;
    P* params;
    double stepsize; // set after creation
  batch_gradient_parameters(int _id, int _nWorkers, M* _model, P* _params) : 
    id(_id), nWorkers(_nWorkers), perm(NULL), examples(NULL), nExamples(0), model(_model), params(_params), stepsize(0.0) {
      //perm      = NULL;
      //stepsize  = 0.0; // EMPAHSIZING THAT THESE ARE SET AFTER CREATION!!!
      //examples  = NULL;
      //nExamples = 0;
    }
  };

  template <class M, class E, class P>
  struct training_parameters {
    scanner<E>* examples;
    int nShufflers, seed, nSplits;
    test_handler<M, E> *t; // possibly null but allows us to test on a data
    int nEpochs; // Number of epochs to run for
    double initial_step_size, step_diminish; // for exponential rules C \rho^{k}
    M* initial_guess; // Cannot be NULL.
    P* params;
    void* (*batch_gradfunction)(batch_gradient_parameters<M,E,P> *bp);    
    double (*loss_function)(const M&,const E&);
  };


  template<class M, class E, class P> M* freeforall(training_parameters<M,E,P> gp);
}
#include "freeforall_template.hxx"
#endif
