#include "file_scanner_util.h"
#include <assert.h>
struct row_col_struct { int row, col, nExamples; };
void max_row_col(struct row_col_struct &rcs, const struct example &e) {
  rcs.row = std::max(e.row, rcs.row);
  rcs.col = std::max(e.col, rcs.col);
  rcs.nExamples ++;
}

scanner<struct example> *
load_file_scanner_examples(const char *szExampleFile, int nFileBuffer, int &nRows, int &nCols, int &nExamples) {
  scanner<struct example> * fs = new file_scanner<struct example>(szExampleFile, nFileBuffer);
  // we need to scan it to get the parameters
  struct row_col_struct rcs = {0,0,0};
  scanner_map<struct example, struct row_col_struct>(fs, rcs, max_row_col);
  nRows     = rcs.row + 1;
  nCols     = rcs.col + 1;
  nExamples = rcs.nExamples;
  return fs;
}
