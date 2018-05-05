
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

#include "fvector.h"

int FVector::_default_n = 30;

void FVector::init(int n) {
  ATOMIC_LOCKING_ONLY( lock_error_check("FVector Lock Initialize", pthread_mutex_init(&_lock, NULL)) ; );
  _n = n;
  _p = new double[n];
  for(int i = 0; i < _n; i++) {
    _p[i] = (drand48() - 0.5) * 1e-3;
  }
}


FVector::FVector()  { init(_default_n); }
FVector::FVector(int n) { init(n); }

void FVector::zero() {
  for(int i = 0; i < _n; i++) { _p[i] = 0.0;}
}

FVector::~FVector() {
  if(_p != NULL) { delete [] _p; } 
  ATOMIC_LOCKING_ONLY( lock_error_check("FVector lock destroy", pthread_mutex_destroy(&_lock) ); )
}
  

FVector::FVector(const FVector &x) {
  ATOMIC_LOCKING_ONLY( lock_error_check("FVector Lock Initialize", pthread_mutex_init(&_lock, NULL)) ; );
  _n = x._n; _p = new double[_n]; 
  memcpy(_p, x._p, sizeof(double) * _n);
}

void
FVector::read_mask(const FVector &x, const SparseVector &sv) {
  for(int i = sv.nValues - 1; i >= 0; i--) {
    _p[sv.indexes[i]] = x._p[sv.indexes[i]];
  }
}


FVector&
FVector::operator=(const FVector &x) {
  if(&x != this) { 
    if(_n != x._n) {
      _n = x._n;
      // This should never happen...
      if(_p) { delete [] _p; }
      _p = new double[_n];
    }
    
    // simply overwrite.
    memcpy(_p, x._p, sizeof(double)*_n); 
  } 
  return *this;
}


  
  

void 
FVector::
set(int i, double v) { 
  if(i >= 0 && i < _n) { _p[i] = v;} else { std::cout << "[set] asked for " << i << std::endl; assert(false); } 
}

double 
FVector::
get(int i) const { 
  if(i >= 0 && i < _n)  {return _p[i];} else { std::cout << "[get] asked for " << i << std::endl; assert(false); } 
}
// void 
// FVector::scale(double w) { for(int i = 0; i < _n; i++) { _p[i] *= w; } }

void 
FVector::
subtract(const FVector &them) {
  for(int i = 0; i < _n; i ++) {
    _p[i] -= them._p[i];
  }
}


void 
FVector::ball_project(double B) {
  ball_project(B, B*B);
}

void 
FVector::ball_project(double B, double B_squared) {
  double norm_square = 0.0;
  for(int i = 0; i < _n; i++) {
    norm_square += _p[i]*_p[i];
  }
  if (norm_square > B_squared) {
    double norm = sqrt(norm_square);
    for(int i = 0; i < _n; i++) {
      _p[i] *= B/norm;
    }
  }
}

void
FVector::
simplex_project() {
  std::vector<double> v(_p, _p+_n);
  std::sort(v.begin(), v.end());
  int i     = _n-2;
  double ti = 0.0, ti_sum = 0.0;

  while(i >= 0) {
    ti_sum += v[i+1];
    ti  = (ti_sum - 1)/(_n - 1 -i);
    if(ti > v[i]) break;
    i--;
  }

  for(int k = 0; k < _n; k++) {
    _p[k] = std::max(0.0, _p[k] - ti);
  }
}

int 
FVector::
get_size() { return _n; } 

double
FVector::
norm() { 
  return sqrt(dot(*this,*this)); 
}

FVector* 
allocate_fvector_array(int ns) { return new FVector[ns]; }
