#include "frontend_util.h"
#include "freeforall_template.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <set>


// Specific Includes
#include "fvector.h"
#include "cut_mainloop.h"
#include "file_scanner_util.h"

std::set<int>
load_intlist(char *fname) {
  std::set<int> s;
  ifstream f_in;
  int i;
  f_in.open(fname, ios::in);
  while(!f_in.eof()) {
    f_in >> i;
    s.insert(i - 1);
  }
  f_in.close();
  return s;
}

int 
main(int argc, char* argv[]) {
  struct freeforall_template::training_parameters<cut_namespace::CutModel, struct example, struct cut_namespace::p> tp;
  // defaults
  tp.nEpochs            = 20; 
  tp.seed               = 0;
  tp.initial_step_size  = 1e-1;
  tp.step_diminish      = 1.0;
  tp.nSplits            = 1;
  tp.nShufflers         = 1;
  tp.examples = NULL;

  bool loadBinary = false, bFileScan = false;
  int nFileBuffer = 1 << 27;

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
    {"file_scan",required_argument, NULL, 'f', "use a file scan (for larger examples larger than main memory)"},
    {NULL,0,NULL,0,0} 
  };
  char usage_str[] = "<edge file> <source file> <sink_file>";
  int c = 0, option_index = 0;
  option* opt_struct = convert_extended_options(long_options);
  while( (c = getopt_long(argc, argv, "", opt_struct, &option_index)) != -1) 
    {
      switch (c) {
      case 'f':
	bFileScan     = true;
	break;
      
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
      case ':':
      case '?':
	print_usage(long_options, argv[0], usage_str);
      exit(-1);
      break;
      }
    }

  char *szExampleFile, *sourceFile, *sinkFile;
  
  if(optind == argc - 3) {
    szExampleFile = argv[optind];
    sourceFile    = argv[optind+1];
    sinkFile      = argv[optind+2];
  } else {
    print_usage(long_options, argv[0], usage_str);
    exit(-1);
  }

  std::cerr << "Edges from " << szExampleFile << std::endl;
  int nRows = 0, nCols = 0, nExamples = 0;
  struct example* exs = NULL;
  if(bFileScan) {
    tp.examples = load_file_scanner_examples(szExampleFile,nFileBuffer, nRows, nCols, nExamples);
    std::cerr << "[File Scan] Examples Loaded" << std::endl;    
  } else {    
    exs = (loadBinary) ? load_binary_examples(szExampleFile, nRows, nCols, nExamples) : load_examples(szExampleFile, nRows, nCols, nExamples);  
    tp.examples  = new memory_scanner<struct example>(nExamples, exs);    
    std::cerr << "[No File Scan] Examples Loaded" << std::endl;  
  }

  int dim      = std::max(nRows, nCols);
  
  tp.t = NULL; // No testing really.
  std::cout << "Starting Training..." << std::endl;
  std::set<int> S = load_intlist(sourceFile);
  std::set<int> T = load_intlist(sinkFile);  

  cut_namespace::CutModel *m = new cut_namespace::CutModel(dim, S, T);
  tp.initial_guess           = m;
  tp.batch_gradfunction      = &cut_namespace::cut_gradient_thread;
  tp.loss_function           = &cut_namespace::compute_loss;
  m = freeforall_template::freeforall<cut_namespace::CutModel, struct example, struct cut_namespace::p>(tp);

  delete [] opt_struct;
  return 0;
}
