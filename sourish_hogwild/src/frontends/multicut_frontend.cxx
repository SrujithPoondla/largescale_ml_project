#include "frontend_util.h"
#include "freeforall_template.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <set>


// Specific Includes
#include "fvector.h"
#include "multicut_mainloop.h"
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
  struct freeforall_template::training_parameters<multicut_namespace::MultiCutModel, struct example, struct multicut_namespace::p> tp;
  // defaults
  tp.nEpochs            = 20; 
  tp.seed               = 0;
  tp.initial_step_size  = 1e-1;
  tp.step_diminish      = 1.0;
  tp.nSplits            = 1;
  tp.nShufflers         = 1;
  tp.examples = NULL;

  bool loadBinary     = false, bFileScan = false;
  bool zero_one_loss  = false;
  int  nFileBuffer    = 1 << 30;
  DEBUG_ONLY(nFileBuffer = 2;)

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
    {"zero_one", required_argument, NULL, 'z', "use zero one loss (rounded solution)"},
    {"file_scan",required_argument, NULL, 'f', "use a file scan (for larger examples larger than main memory)"},
    {NULL,0,NULL,0,0} 
  };
  char usage_str[] = "<edge file> <terminal file>";
  int c = 0, option_index = 0;
  option* opt_struct = convert_extended_options(long_options);
  while( (c = getopt_long(argc, argv, "", opt_struct, &option_index)) != -1) 
    {
      switch (c) {      
      case 'f':
	bFileScan     = true;
	break;

      case 'z':
	zero_one_loss = (atoi(optarg) != 0);
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

  char *szExampleFile, *terminalFile;
  
  if(optind == argc - 2) {
    szExampleFile = argv[optind];
    terminalFile    = argv[optind+1];
  } else {
    print_usage(long_options, argv[0], usage_str);
    exit(-1);
  }

  std::cerr << "Edges from " << szExampleFile << std::endl;
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
  tp.t = NULL; // No testing really.
  int dim      = std::max(nRows, nCols);
  std::cerr << "dim=" << dim << std::endl;
  std::set<int> terminals = load_intlist(terminalFile);
  std::cerr << "Terminals Loaded" << std::endl;

  multicut_namespace::MultiCutModel *m = new multicut_namespace::MultiCutModel(dim, terminals);
  tp.initial_guess           = m;
  tp.batch_gradfunction      = &multicut_namespace::cut_gradient_thread;
  tp.loss_function           = (zero_one_loss) ? &multicut_namespace::zero_one_loss : &multicut_namespace::compute_loss;
  std::cout << "Starting Training..." << std::endl;
  m = freeforall_template::freeforall<multicut_namespace::MultiCutModel, struct example, struct multicut_namespace::p>(tp);

  delete [] opt_struct;
  if(exs) delete exs;
  return 0;
}
