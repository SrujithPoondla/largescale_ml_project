#include "frontend_util.h"
#include "examples.h"
#include <cstdlib>
#include <iostream>

using namespace std;

int 
main(int argc, const char* argv[]) {
  if(argc != 3) {
    cout << "need a file to translate" << endl;
    return 0;
  }
  const char* szFile = argv[1];
  const char* szOut  = argv[2];
  cout << "loading " << szFile << endl;
  int nRows = 0, nCols = 0, nExamples = 0;
  struct example* r = load_examples(szFile, nRows, nCols, nExamples);
  cout << "writing " << nExamples << " examples " << endl;
  write_binary_examples(r, nExamples, szOut);
}
