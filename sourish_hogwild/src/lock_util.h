#include <pthread.h>
void lock_error_check(const char *msg, int rc);
void pthread_check(const char*msg, int rc);

// Deadlock avoidance. Our rule is always lock the lock that has the lower memory address first.    
// This guaranetees a total order.
// This only crops up in the binary operations
void acquire_two_locks(pthread_mutex_t *l1, pthread_mutex_t *l2);
void release_two_locks(pthread_mutex_t *l1, pthread_mutex_t *l2);

void roundrobin_acquire(int id, volatile int *current_value, int round_size, pthread_mutex_t *mutex,  pthread_cond_t *cond);
void roundrobin_release(int id, volatile int *current_value, int round_size, pthread_mutex_t *mutex,  pthread_cond_t *cond);
