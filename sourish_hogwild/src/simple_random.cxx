#include "simple_random.h"
#include <cstdlib>
#include <iostream>

void  
simple_random::gen_seed() {
  struct timespec _x;
  clock_gettime(CLOCK_REALTIME, &_x);
  _seed = (unsigned int) (_x.tv_nsec * _x.tv_sec );
}

void
simple_random::init() {
  DEBUG_ONLY(std::cout << "[simple_random] random seed initialized to " << _seed << std::endl;);
  srand(_seed);
}

simple_random::simple_random() {
  gen_seed();
  init();
}

simple_random::
simple_random(unsigned int _s) {
  _seed = _s;
  if(_seed == 0) { gen_seed(); } 

  init();
}

int 
simple_random::rand_int(int nMax) {
  return (int) (((double) nMax) * (rand_r(&_seed) / (RAND_MAX + 1.0)));
}

double
simple_random::rand_double() { return drand48(); } 
