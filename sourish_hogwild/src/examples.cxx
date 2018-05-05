#include "examples.h"
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

void
write_binary_examples(struct example* ex, int nExamples, const char *szFile) {
  FILE *f = fopen( szFile, "wb");
  assert(f != NULL);
  fwrite(&nExamples, sizeof(int), 1, f);
  fwrite(ex, sizeof(struct example), nExamples, f);
  fclose(f);
}

struct example *
load_binary_examples(const char *szExample, int &nRows, int &nCols, int &nExamples) {
  FILE *f = fopen( szExample, "rb");
  assert(f != NULL);
  assert(fread(&nExamples, sizeof(int), 1, f) == 1);
  struct example *ret = new example[nExamples];
  assert(fread( ret, sizeof(struct example), nExamples, f) == (size_t) nExamples);
  fclose(f);
  nRows = 0;
  nCols = 0;
  for(int i = nExamples - 1; i >= 0; i--) { 
    nRows = max(ret[i].row, nRows); 
    nCols = max(ret[i].col, nCols); 
  } 
  nRows ++;
  nCols ++;
  return ret;
}

struct example * 
load_examples(const char *szExample, int &nRows, int &nCols, int &nExamples) {
  vector<struct example> rets;
  ifstream f_in;
  int row,col;
  double rating;
  f_in.open(szExample, ios::in);
  assert(f_in.is_open());
  nExamples = 0;
  int max_row = 0;
  int max_col = 0;
  
  while(!f_in.eof()) {
    row = -1; col = -1; rating = -1.0;
    f_in >> row >> col >> rating;
    example e;
    col--; row--; // correct the matlab code offsets
    VERBOSE_ONLY(cout << " ... " << col << " " << row << endl;);
    if(col < 0 && row < 0 && rating < 0) {
      continue;
    }
    if( col < -2 || row < 0 ) {
      cout << " ... " << row << " " << col << endl;
      assert(false);
    }
    e.row = row;
    e.col = col;
    e.rating = (double) rating;
    rets.push_back(e); 
    max_row = max(max_row, row);
    max_col = max(max_col, col);    
    nExamples++;
  }
  nRows = max_row + 1;
  nCols = max_col + 1;
  
  f_in.close();

  struct example *ret = new example[nExamples];
  for(int i =0; i< nExamples ; i++) {ret[i] = rets[i];}
  return ret;
}
