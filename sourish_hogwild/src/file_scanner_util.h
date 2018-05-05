#include "file_scanner.h"
#include "examples.h"

scanner<struct example> * load_file_scanner_examples(const char *szExample, int nFileBuffer, int &nRows, int &nCols, int &nExamples);
