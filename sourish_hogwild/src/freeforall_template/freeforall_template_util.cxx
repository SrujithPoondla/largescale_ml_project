#include "freeforall_template_util.h"
#include <cstdlib>
#include <stdlib.h>

void 
permute(simple_random &rand, int *d, int n) {
  for(int i = n-1; i > 0; i--) {
    int rand_index = rand.rand_int(i); // pick something [0,i]
    int temp       = d[i];
    d[i]           = d[rand_index];
    d[rand_index]  = temp;    
  }
  VERBOSE_ONLY(
	       for(int i =0; i < n; i++) {
		 cout << i << " -> " << d[i] << endl;
	       }
	       )
}

int*
init_permutation(int nSize) {
  int *ret = new int[nSize];
  for(register int i = nSize - 1; i >= 0; i--) { ret[i] = i; }
  return ret;
}
