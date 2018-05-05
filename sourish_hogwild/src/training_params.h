#ifndef _training_params
#define _training_params
#include "examples.h"

template <class T>
struct 
training_parameters {
  struct example* examples; // holds a pointer to the actual examples;
  int nExamples, nRows, nCols; // rows and columns to use (determined from the data load)

  int nShufflers;
  test_handler<T, struct example> *t; // possibly null but allows us to test on a data
  int nEpochs; // Number of epochs to run for
  int max_rank; // maximum rank of the factors
  bool bTraceNorm; // if true trace norm, else max norm
  int nSplits; // Number of worker threads to use
  int seed; // the random seed
  // stepsize rules
  double initial_step_size, step_diminish; // for exponential rules C \rho^{k}
  double B, mu; // B is the constraint ball for max norm, mu is the general weight for trace norm.  
  bool bPrint; // should print out rmse etc
  T *initial_guess; // If NULL we rebuild the model.
};
#endif
