#include <cstdlib>
#include "model.h"
#include "examples.h"
#include "global_macros.h"

// Testing and prediction code
double 
predict(const struct model &m, int row, int col) {  
  VERBOSE_ONLY(std::cout << "\t row=" << row << " col=" << col << std::endl;);
  return FVector::dot(m.L[row],m.R[col]) + m.moviemean;
}

double
compute_loss(const struct model &m, const struct example &e) {  
  double v = predict(m, e.row, e.col) - e.rating;
  return v*v;
}


double
model_mse(const struct model &m, const struct example &e) {  
  double v = predict(m, e.row, e.col) - e.rating;
  return v*v;
}


// Testing and prediction code
double 
predict(const struct model_with_means &m, int row, int col) {  
  VERBOSE_ONLY(std::cout << "\t row=" << row << " col=" << col << std::endl;);
  return FVector::dot(m.L[row],m.R[col]) + (m.l_means[row] + m.r_means[col])/2.0 + m.moviemean;
}


double
compute_loss(const struct model_with_means &m, const struct example &e) {  
  double v = predict(m, e.row, e.col) - e.rating;
  return v*v;
}

double
model_with_means_rmse(const struct model_with_means &m, const struct example &e) {  
  double v = predict(m, e.row, e.col) - e.rating;
  return v*v;
}



model::
model(int nr, int nc, double mm, FVector *_l, FVector *_r) : nRows(nr), nCols(nc), moviemean(mm), L(_l), R(_r) { }

struct model *
build_model(int _nRows, int _nCols, double _moviemean,
	    FVector *l, FVector *r) {
  return new struct model(_nRows, _nCols, _moviemean, l, r);
}


struct model_with_means *
build_model_with_means(int _nRows, int _nCols, double _moviemean,
		       FVector *l, FVector *r, double *l_means, double* r_means) {
  return new model_with_means(_nRows, _nCols, _moviemean, l, r, l_means, r_means);
}
