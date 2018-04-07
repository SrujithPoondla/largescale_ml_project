#include "frontend_util.h"
#include "freeforall_template.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <set>


// Specific Includes
#include "fvector.h"
#include "sparse_svm_generic.h"
struct timespec  tsgradient_delay;
bool _is_gradient_delay        = false;

int 
main(int argc, char* argv[]) {
  struct freeforall_template::training_parameters<sparsesvm_templates::svm_model, sparsesvm_templates::sparse_example, struct sparsesvm_templates::svm_p> tp;
  // defaults
  tp.nEpochs            = 20; 
  tp.seed               = 0;
  tp.initial_step_size  = 1e-1;
  tp.step_diminish      = 1.0;
  tp.nSplits            = 1;
  tp.nShufflers         = 1;
  tp.examples = NULL;
  tp.params             = new struct sparsesvm_templates::svm_p;
  tp.params->mu         = 1e-1;

  bool loadBinary = false;
  static struct extended_option long_options[] = {
    {"mu", required_argument, NULL, 'u', "regularization parameter"},
    {"epochs"    ,required_argument, NULL, 'e', "number of epochs (default is 20)"},
    {"stepinitial",required_argument, NULL, 'i', "intial stepsize (default is 5e-2)"},
    {"step_decay",required_argument, NULL, 'd', "stepsize decay per epoch (default is 0.8)"},
    {"seed", required_argument, NULL, 's', "random seed (o.w. selected by time, 0 is reserved)"},
    {"splits", required_argument, NULL, 'r', "number of threads"},
    {"shufflers", required_argument, NULL, 'q', "number of shufflers"},
    {"binary", required_argument,NULL, 'v', "load the file in a binary fashion"},
#ifdef _DELAY_ONLY
    {"delay", required_argument,NULL, 'l', "delay the gradient computation in usec"},
#endif
    {NULL,0,NULL,0,0} 
  };
  char usage_str[] = "<test file> <train file>";
  int c = 0, option_index = 0;
  option* opt_struct = convert_extended_options(long_options);
  while( (c = getopt_long(argc, argv, "", opt_struct, &option_index)) != -1) 
    {
      switch (c) {      
#ifdef _DELAY_ONLY
      case 'l':
	_is_gradient_delay       = true;
	tsgradient_delay.tv_sec  = 0;
	tsgradient_delay.tv_nsec = atol(optarg); // nanooseconds
	struct timespec ts;
	clock_getres(CLOCK_THREAD_CPUTIME_ID, &ts);
	std::cerr << "REALTIME Highres=" << ts.tv_nsec << std::endl;
	assert(ts.tv_nsec <= tsgradient_delay.tv_nsec);
	assert(ts.tv_sec == 0);
	break;
#endif
      case 'v':
	loadBinary = (atoi(optarg) != 0);
	break;
  
      case 'q':
	tp.nShufflers      = atoi(optarg);
	break;
  
      case 'e':
	tp.nEpochs = atoi(optarg);
	break;
      case 'i':
	tp.initial_step_size    = atof(optarg);
	break;
      case 'd':
	tp.step_diminish  = atof(optarg);
	break;
      case 's':
	tp.seed              = atol(optarg);
	break;
      case 'r':
	tp.nSplits         = atoi(optarg);
	break;
      case 'u':
	tp.params->mu         = atof(optarg);
	break;

      case ':':
      case '?':
	print_usage(long_options, argv[0], usage_str);
      exit(-1);
      break;
      }
    }

  char *szExampleFile, *trainFile = NULL;
  
  if(optind == argc - 2) {
    szExampleFile = argv[optind]; 
    trainFile     = argv[optind+1];
  } else {
    print_usage(long_options, argv[0], usage_str);
    exit(-1);
  }

  std::cerr << "Edges from " << szExampleFile << std::endl;
  int nRows = 0, nCols = 0, nExamples = 0, nSparseExamples = 0;

  struct example* exs = (loadBinary) ? load_binary_examples(szExampleFile, nRows, nCols, nExamples) : load_examples(szExampleFile, nRows, nCols, nExamples);
  std::cerr << "Examples Loaded" << std::endl;
  sparsesvm_templates::sparse_example *se = sparsesvm_templates::parse_examples(nExamples, exs, nSparseExamples);
  tp.examples  = new memory_scanner<sparsesvm_templates::sparse_example>(nSparseExamples, se);

  int dim       = nCols;
  int nTestRows = 0, nTestCols = 0, nTestExamples = 0, nTestSparseExamples;
  struct example *test_examples = load_examples(trainFile, nTestRows, nTestCols, nTestExamples);
  sparsesvm_templates::sparse_example *test_se = sparsesvm_templates::parse_examples(nTestExamples, test_examples, nTestSparseExamples);
  tp.t = new test_handler<sparsesvm_templates::svm_model, sparsesvm_templates::sparse_example>(test_se, nTestSparseExamples, sparsesvm_templates::compute_loss);
  std::cout << "Starting Training..." << dim << " rows=" << nRows << " nExamples=" << nExamples << std::endl;
  
  tp.params->degrees = new double[dim];
  for(int i = dim - 1; i >= 0; i--) { tp.params->degrees[i] = 0.0; }
  tp.params->nDims   = dim;
  
  scanner_map<struct sparsesvm_templates::sparse_example, struct sparsesvm_templates::svm_p>(tp.examples, *tp.params, sparsesvm_templates::compute_degrees);
  struct sparsesvm_templates::svm_model *m = new sparsesvm_templates::svm_model(dim);
  tp.initial_guess           = m;
  tp.batch_gradfunction      = &sparsesvm_templates::sparse_svm_gradient;
  tp.loss_function           = &sparsesvm_templates::compute_loss;
  m = freeforall_template::freeforall<sparsesvm_templates::svm_model, sparsesvm_templates::sparse_example, struct sparsesvm_templates::svm_p>(tp);

  delete [] opt_struct;
  return 0;
}
