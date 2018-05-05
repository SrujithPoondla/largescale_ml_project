#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

void
lock_error_check(const char *msg, int rc)
{
  if(rc == 0) {return;} 
  else {
    printf("Pthread error code %d with message %s\n", rc, msg);
    assert(false);
  }
}
void pthread_check(const char* msg, int rc) { lock_error_check(msg, rc); }

// Deadlock avoidance. Our rule is always lock the lock that has the lower memory address first.    
// This guaranetees a total order.
// This only crops up in the binary operations
void 
acquire_two_locks(pthread_mutex_t *l1, pthread_mutex_t *l2) {
  if(l1 == l2) {
     lock_error_check("Two Locks", pthread_mutex_lock(l1));
  } else {
    if(l1 < l2) {
      lock_error_check("Two Locks", pthread_mutex_lock(l1));
      lock_error_check("Two Locks", pthread_mutex_lock(l2));
    } else {
      lock_error_check("Two Locks", pthread_mutex_lock(l2));
      lock_error_check("Two Locks", pthread_mutex_lock(l1));
    }
  }
}
void
release_two_locks(pthread_mutex_t *l1, pthread_mutex_t *l2) {
  if(l1 == l2) {
    lock_error_check("Two unLocks (1)", pthread_mutex_unlock(l1));
  } else {
    lock_error_check("Two unLocks (2)", pthread_mutex_unlock(l1));
    lock_error_check("Two unLocks (2)", pthread_mutex_unlock(l2));
  }
}

void
roundrobin_acquire(int id, volatile int *tv, int round_size, pthread_mutex_t *mutex, pthread_cond_t *cond) {

  int last_value = (id > 0) ? id - 1: round_size - 1;
#ifdef _BUSY_WAIT
  while( *tv != last_value );
#else
  lock_error_check("Round Robin Lock", pthread_mutex_lock(mutex));
  while( *tv != last_value ) {
    lock_error_check("Cond wait check", pthread_cond_wait(cond, mutex));
  }
#endif  
}

void 
roundrobin_release(int id, volatile int *tv, int round_size, pthread_mutex_t *mutex, pthread_cond_t *cond) {
  (*tv) = id;
#ifndef _BUSY_WAIT
  lock_error_check("Round Robin Release"  , pthread_cond_broadcast(cond));
  lock_error_check("Round Robin Exit Lock", pthread_mutex_unlock(mutex));
#endif
}
