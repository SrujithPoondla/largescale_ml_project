#include "frontend_util.h"
#include "freeforall.h"
#include <cstdlib>
#include <iostream>

int 
main(int argc, char* argv[]) {
  struct training_parameters<struct model> tp;
  // defaults
  tp.nEpochs = 20, tp.max_rank = 30; 
  tp.seed = 0;
  tp.initial_step_size = 1e-1;
  tp.step_diminish = 1.0;
  tp.nSplits = 1;
  tp.nShufflers = 1;
  tp.bTraceNorm = false; 
  tp.mu = -1.0;   tp.B=1.5;
  tp.initial_guess = NULL;
  tp.examples = NULL;
  tp.bPrint   = true;
  bool loadBinary = false;
  static struct extended_option long_options[] = {
    {"tracenorm", required_argument, NULL, 't', "if set we run tracenorm otherwise we run maxnorm (maxnorm is default)"},
    {"B", required_argument, NULL, 'b', "maximum row norm of L and R (default is 1.5)"},
    {"mu", required_argument, NULL, 'u', "the maxnorm"},
    {"maxrank", required_argument, NULL, 'm', "size of factorization of L and R"},
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
      case 't':
	tp.bTraceNorm       = (atoi(optarg) != 0);
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
      case 'm':
	tp.max_rank  = atoi(optarg);
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

  if(tp.mu < 0.0 && tp.bTraceNorm) {
    std::cout << "mu is required when tracenorm is selected" << std::endl;
    print_usage(long_options, argv[0], usage_str);
    exit(-1);
  }

  std::cerr << "Training with " << szExampleFile << " Testing with " << szTestFile << std::endl;
  int nRows = 0, nCols = 0, nExamples = 0;
  tp.examples = (loadBinary) ? load_binary_examples(szExampleFile, nRows, nCols, nExamples) : load_examples(szExampleFile, nRows, nCols, nExamples);

  tp.nRows = nRows;
  tp.nCols = nCols;
  tp.nExamples = nExamples;

  struct example *exs = NULL;
  create_test_handler<struct model>(szTestFile, exs, tp.t);

  std::cout << "Starting Training..." << std::endl;
  struct model m = freeforall::freeforall_jellyfish(tp);
  std::cout << "Second lap..." << std::endl;
  m = freeforall::freeforall_jellyfish(tp);
  double rmse = tp.t->test(m);
  std::cout << "Final RMSE: " << rmse << std::endl;


  delete tp.t;
  delete [] exs;
  //delete [] tp.examples;
  delete [] opt_struct;
  return 0;
}
