#ifndef _fvector_h_
#define _fvector_h_ 1
#include <iostream>
#include "global_macros.h"


#ifdef _ATOMIC_LOCKING
#include <pthread.h>
#include "lock_util.h"
#endif

#include "sparse_vector.h"

class FVector {
  double* _p; 
  int _n;
  void init(int n);

public:
  // Atom level locking. Made public to get better preformance
  ATOMIC_LOCKING_ONLY(pthread_mutex_t _lock;);
  static int _default_n;
  // This is really disgusting, but the default constructor intializes (initialization to 30)
  // This is is a hack to make stacking easy.
  FVector();  
  FVector(int n); 

  FVector(const FVector &x);
  FVector& operator=(const FVector &x);
  ~FVector();

  void zero(); 
  void set(int i, double v);
  double get(int i) const;
  inline void scale(double w) { 
    for(register int i = _n - 1; i >=0; i--) { _p[i] *= w; } 
  }
  void subtract(const FVector &them);


  inline void scale_and_add(const FVector &them, double w) {
      for(register int i = 0; i < _n; i ++) {
        _p[i] += them._p[i] * w;
      }    
  }
  void ball_project(double B);
  void ball_project(double B, double B_squared);
  static inline double dot(const FVector &v, const FVector &v2) {
    if(v._n != v2._n) {
      std::cout << "dot of two unequal vectors! " << v._n << " v. " << v2._n << std::endl;
      exit(-1);
    }
    double ret = 0.0;
    for(register int i = 0; i < v._n; i++) {
      ret += v._p[i] * v2._p[i];
    }
    return ret;
  }
  inline void scale_and_add(SparseVector sv, double s) {
    for(int i = sv.nValues - 1; i >= 0; i--) {
      _p[sv.indexes[i]] += s*sv.values[i];
    }
  }

  void simplex_project();
  // use the elements of SV as a mask
  void read_mask(const FVector &x, const SparseVector &sv);

  static inline double dot(const SparseVector &x, const ::FVector &y) {
    double ret = 0.0;
    double *xv = &(x.values[x.nValues - 1]);
    for(register int i = x.nValues - 1; i >= 0; i--, xv--) {
      ret += (*xv)*y.get(x.indexes[i]);
    }
    return ret;
  }
  

  int get_size();
  double norm();
  // Helpers for NumPy code
  void set_size(int n) { FVector::_default_n = n;} 
};
FVector* allocate_fvector_array(int ns);


#endif
