#ifndef _timer_h
#define _timer_h
#include "global_macros.h"
#include <iostream>

#ifdef __APPLE__
// This just hacks in a redefine of clock_gettime with a low_resolution clock.
#include <sys/time.h>
const int CLOCK_REALTIME = 1; // we use this for course-grained timing.
const int CLOCK_THREAD_CPUTIME_ID = 2; // this is used for fine-grained experiments. I wouldn't trust it under OSX.
void clock_gettime(int ignored, struct timespec *tp);
#endif

class timer {
  struct timespec _start, _stop;
  bool bStarted;
public:
  // don't start the timer
  timer();
  // automatically start the timer too
  timer(bool b);
  // start the timer
  void start(); 
  // stop the timer
  void stop ();
  // seconds
  double elapsed ();
  // stop and elapsed
  double stop_elapsed ();
  // elapsed
  double elapsed_minutes() { return elapsed()/60.0; } 
};

std::ostream& operator<<(std::ostream &out, timer &t);
#endif
