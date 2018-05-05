#ifndef _sparse_vector_h
#define _sparse_vector_h 
#include "global_macros.h"

class SparseVector {
 public:
    double *values;
    int    *indexes;
    int nValues;
 
  // Assumes the indexes of the vectors are in ascending order.
  SparseVector(double *_values, int *_indexes, int _nValues) : nValues(_nValues) {
      values  = new double[_nValues];
      indexes = new int[_nValues];
      memcpy(values , _values , sizeof(double)*nValues);
      memcpy(indexes, _indexes, sizeof(int)*nValues);
      // This structural assertion is used to prevent deadlocks!
      DEBUG_ONLY( for(int i = 1; i < nValues; i++) { assert( indexes[i-1] < indexes[i] ); } );
    }

    static inline double dot(SparseVector &x, SparseVector &y) {
      int xi = x.nValues - 1;
      int yi = y.nValues - 1;
      double ret = 0.0;
      while(xi > 0 && yi > 0) {
	int diff = x.indexes[xi] - y.indexes[yi];
	if(diff == 0) {
	  ret += x.values[xi]*y.values[yi];
	  xi--; yi--;
	} else {  
	  if(diff > 0) {xi--;}  else { yi--; }
	}
      }
      return ret;
  };
    friend class FVector;
};

#endif
