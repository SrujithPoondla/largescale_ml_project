#include "frontend_util.h"
#include "freeforall.h"
#include "sparse_svm.h"
#include <cstdlib>
#include <iostream>

int 
main(int argc, char* argv[]) {
  struct sparsesvm_freeforall::svm_training_parameters tp;
  // defaults
  tp.nEpochs = 20; 
  tp.seed = 0;
  tp.initial_step_size = 1e-1;
  tp.step_diminish = 1.0;
  tp.nSplits = 1;
  tp.nShufflers = 1;
  tp.mu = -1.0;   tp.B=1.5;
  tp.examples = NULL;
  //tp.bPrint   = true;
  bool loadBinary = false;
  static struct extended_option long_options[] = {
    {"B", required_argument, NULL, 'b', "maximum row norm of L and R (default is 1.5)"},
    {"mu", required_argument, NULL, 'u', "the maxnorm"},
    {"epochs"    ,required_argument, NULL, 'e', "number of epochs (default is 20)"},
    {"stepinitial",required_argument, NULL, 'i', "intial stepsize (default is 5e-2)"},
    {"step_decay",required_argument, NULL, 'd', "stepsize decay per epoch (default is 0.8)"},
    {"seed", required_argument, NULL, 's', "random seed (o.w. selected by time, 0 is reserved)"},
    {"splits", required_argument, NULL, 'r', "number of threads"},
    {"shufflers", required_argument, NULL, 'q', "number of shufflers"},
    {"binary", required_argument,NULL, 'v', "load the file in a binary fashion"},
    {NULL,0,NULL,0,0} 
  };
  char usage_str[] = "<train file> <test file>";
  int c = 0, option_index = 0;
  option* opt_struct = convert_extended_options(long_options);
  while( (c = getopt_long(argc, argv, "", opt_struct, &option_index)) != -1) 
    {
      switch (c) {      
      case 'v':
	loadBinary = (atoi(optarg) != 0);
	break;
  
      case 'q':
	tp.nShufflers      = atoi(optarg);
	break;

      case 'u':
	tp.mu               = atof(optarg);
	break;

      case 'b':
	tp.B        = atof(optarg);
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
      case ':':
      case '?':
	print_usage(long_options, argv[0], usage_str);
      exit(-1);
      break;
      }
    }

  char * szTestFile, *szExampleFile;
  
  if(optind == argc - 2) {
    szExampleFile = argv[optind];
    szTestFile  = argv[optind+1];
  } else {
    print_usage(long_options, argv[0], usage_str);
    exit(-1);
  }

  std::cerr << "Training with " << szExampleFile << " Testing with " << szTestFile << std::endl;
  int nRows = 0, nCols = 0, nExamples = 0;
  struct example* exs = (loadBinary) ? load_binary_examples(szExampleFile, nRows, nCols, nExamples) : load_examples(szExampleFile, nRows, nCols, nExamples);
  std::cerr << "Examples Loaded" << std::endl;
  tp.examples  = sparsesvm_freeforall::parse_examples(nExamples, exs);
  tp.nExamples = nRows;
  
  tp.nDims     = nCols;

  //tp.t = new test_handler<struct sparsesvm_freeforall::svm_model>(szTestFile);
  tp.t = NULL;
  std::cout << "Starting Training..." << std::endl;
  struct sparsesvm_freeforall::svm_model m = sparsesvm_freeforall::sparse_svm(tp);

  // delete t;
  delete [] opt_struct;
  return 0;
}
