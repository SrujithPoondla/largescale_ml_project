#include <cstdlib>
#include "timer.h"

#ifdef __APPLE__
// This just hacks in a redefine of clock_gettime with a low_resolution clock.
#include <sys/time.h>
//const int CLOCK_REALTIME = 1; // we don't use the other clocks.
void
clock_gettime(int ignored, struct timespec *tp) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  tp->tv_sec  = tv.tv_sec;
  tp->tv_nsec = tv.tv_usec*1000;
}
#endif

timer::timer(bool b) { if(b) {start();} }
timer::timer() { bStarted = false; } 
void timer::start() { bStarted = true; clock_gettime(CLOCK_REALTIME, &_start); } 
void timer::stop () { assert(bStarted); clock_gettime(CLOCK_REALTIME, &_stop); }

// seconds
double timer::elapsed () { 
  clock_gettime(CLOCK_REALTIME, &_stop);
  return (double) (_stop.tv_sec - _start.tv_sec)  + (double) (_stop.tv_nsec - _start.tv_nsec) * 1e-9; 
} 
double timer::stop_elapsed() {
  stop(); return elapsed();
}

std::ostream& operator<<(std::ostream &out, timer &t) {
  out << t.stop_elapsed() << "s" ;
  return out;
}
