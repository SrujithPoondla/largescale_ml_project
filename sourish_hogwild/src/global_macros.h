#include <assert.h>
#include <string.h>
#include <cstdlib>

#ifdef __DEBUG
#define DEBUG_ONLY(x) x;
#else
#define DEBUG_ONLY(x);
#endif

#ifdef __VERBOSE
#define VERBOSE_ONLY(x) x;
#else
#define VERBOSE_ONLY(x);
#endif



#ifdef _GLOBAL_LOCK
#define GLOBAL_LOCK_ONLY(x) x;
#else
#define GLOBAL_LOCK_ONLY(x);
#endif


#ifdef _GLOBAL_RWLOCK
#define GLOBAL_RWLOCK_ONLY(x) x;
#else
#define GLOBAL_RWLOCK_ONLY(x);
#endif


#ifdef _ATOMIC_LOCKING
#define ATOMIC_LOCKING_ONLY(x) x;
#else
#define ATOMIC_LOCKING_ONLY(x);
#endif


#ifdef _ROUND_ROBIN
#define ROUNDROBIN_ONLY(x) x;
#else
#define ROUNDROBIN_ONLY(x);
#endif


#ifdef _DELAY_ONLY
#define DELAY_ONLY(x) x;
#include <time.h>
extern struct timespec tsgradient_delay;
extern bool  _is_gradient_delay;
#else
#define DELAY_ONLY(x);
#endif


