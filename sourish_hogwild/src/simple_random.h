#ifndef __simple_random_h
#define __simple_random_h
#include "global_macros.h"
#include <time.h>

#ifdef __APPLE__
// This is just to redefine clock_gettime.
#include "timer.h"
#endif

class simple_random {
  unsigned int _seed;
  void gen_seed();
  void init();
 public:
  simple_random(); // generate our own seed.
  simple_random(unsigned int);// fixed seed
  int rand_int(int nMax); // an integer in [0,nMax]
  double rand_double();   // random double in [0.0,1.0]
};
#endif
