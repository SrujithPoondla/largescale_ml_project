#ifndef _EXAMPLES_H__
#define _EXAMPLES_H__
#include "global_macros.h"

struct example {
  int row, col; double rating;
};

void
write_binary_examples(struct example* ex, int nExamples, const char *szFile);

struct example * 
load_binary_examples(const char *szExample, int &nRows, int &nCols, int &nExamples);

struct example * 
load_examples(const char *szExample, int &nRows, int &nCols, int &nExamples);
#endif
