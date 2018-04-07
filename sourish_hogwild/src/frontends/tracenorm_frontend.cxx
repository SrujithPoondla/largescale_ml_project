#include "frontend_util.h"
#include "examples.h"

#include "test_handler.h"
#include "file_scanner.h"
#include "file_scanner_util.h"
#include "tracenorm_factor.h"
#include <cstdlib>
#include <iostream>

test_handler<struct tracenorm_factor::tracenorm_model, struct example>*
create_local_test_handler(const char *szTestFile, struct example* &data) {
  int nRows, nCols, nExamples;
  DEBUG_ONLY(timer load_timer; load_timer.start(); );
  DEBUG_ONLY(std::cout << "[TEST] Loading the test set" << std::endl);
  data = load_examples(szTestFile, nRows, nCols, nExamples);
  test_handler<struct tracenorm_factor::tracenorm_model, struct example>* t =
    new test_handler<struct tracenorm_factor::tracenorm_model,struct example>(data, nExamples, ( double(*)(const struct tracenorm_factor::tracenorm_model&, const struct example&)) tracenorm_factor::compute_loss);
  DEBUG_ONLY(load_timer.stop(); );
  DEBUG_ONLY(std::cout << "[TEST] Finished Loading " << nExamples << " examples in " << load_timer.elapsed() << "s" << std::endl);
  return t;
}



int 
main(int argc, char* argv[]) {
  struct freeforall_template::training_parameters<tracenorm_factor::tracenorm_model, struct example, struct tracenorm_factor::tracenorm_parameters> tp;
  // defaults
  tp.nEpochs = 20; 
  tp.seed = 0;
  tp.initial_step_size = 1e-1;
  tp.step_diminish = 1.0;
  tp.nSplits = 1;
  tp.nShufflers = 1;

  tp.params           = new struct tracenorm_factor::tracenorm_parameters();
  tp.params->mu       = -1.0;
  tp.params->max_rank = 30;
  tp.t                = NULL;

  bool loadBinary = false, bFileScan = false;
  int  nFileBuffer    = 1 << 26;
  DEBUG_ONLY(nFileBuffer = 3;);
  char *testFile = NULL, *outputTestFile = NULL;
  static struct extended_option long_options[] = {
    {"mu", required_argument, NULL, 'u', "the maxnorm"},
    {"maxrank", required_argument, NULL, 'm', "size of factorization of L and R"},
    {"epochs"    ,required_argument, NULL, 'e', "number of epochs (default is 20)"},
    {"stepinitial",required_argument, NULL, 'i', "intial stepsize (default is 5e-2)"},
    {"step_decay",required_argument, NULL, 'd', "stepsize decay per epoch (default is 0.8)"},
    {"seed", required_argument, NULL, 's', "random seed (o.w. selected by time, 0 is reserved)"},
    {"splits", required_argument, NULL, 'r', "number of threads"},
    {"binary", required_argument,NULL, 'v', "load the file in a binary fashion"},
    {"file_scan", required_argument,NULL, 'f', "load the file in a scan (binary)"},
    {NULL,0,NULL,0,0} 
  };
  char usage_str[] = "<train file> <test file>";
  int c = 0, option_index = 0;
  option* opt_struct = convert_extended_options(long_options);
  while( (c = getopt_long(argc, argv, "", opt_struct, &option_index)) != -1) 
    {
      switch (c) {      
      case 'f':
	bFileScan     = true;
	break;
      case 'w':
	testFile = optarg;
	break;
      case 'o':
	outputTestFile = optarg;
	break;
      case 'v':
	loadBinary = (atoi(optarg) != 0);
	break;

      case 'q':
	tp.nShufflers      = atoi(optarg);
	break;

      case 'u':
	tp.params->mu       = atof(optarg);
	break;

      case 'm':
	tp.params->max_rank  = atoi(optarg);
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


  if(tp.params->mu < 0.0) {
    std::cout << "mu is required parameter for the tracenorm" << std::endl;
    print_usage(long_options, argv[0], usage_str);
    exit(-1);
  }

  int nRows = 0, nCols = 0, nExamples = 0;
  struct example* exs = NULL;
  if(bFileScan) { // Must be binary
    tp.examples = load_file_scanner_examples(szExampleFile,nFileBuffer, nRows, nCols, nExamples);
    std::cerr << "[File Scan] Examples Loaded" << std::endl;    
  } else {    
    exs = (loadBinary) ? load_binary_examples(szExampleFile, nRows, nCols, nExamples) : load_examples(szExampleFile, nRows, nCols, nExamples);  
    tp.examples  = new memory_scanner<struct example>(nExamples, exs);    
    std::cerr << "[No File Scan] Examples Loaded" << std::endl;  
  }
  assert(nExamples > 0);
  tp.params->setup(nRows, nCols, nExamples);
  scanner_map<struct example, struct tracenorm_factor::tracenorm_parameters>(tp.examples, *tp.params, tracenorm_factor::parameter_map);
  std::cerr << "[Scanning] Parameters Loaded mean=" << tp.params->mean << " nExamples=" << nExamples << std::endl;    

  // Setup the model parametesr
  tp.initial_guess       = new tracenorm_factor::tracenorm_model(nRows, nCols, tp.params->max_rank, tp.params->mean);
  tp.batch_gradfunction  = &tracenorm_factor::tracenorm_gradient_thread;
  tp.loss_function       = &tracenorm_factor::compute_loss;

  // Create the testing infrastructure
  struct example *e   = NULL;
  tp.t = create_local_test_handler(szTestFile, e);
  
  std::cout << "Starting Training..." << std::endl;
 
  freeforall_template::freeforall<tracenorm_factor::tracenorm_model, struct example, struct tracenorm_factor::tracenorm_parameters>(tp);

  /* if(testFile != NULL && outputTestFile != NULL ) {
    std::cout << "Writing Predictions to " << outputTestFile << std::endl;
    make_predictions(testFile, outputTestFile, m);
    } */
  
  // delete t;
  delete [] opt_struct;
  return 0;
}
