#ifndef _MODEL_H__
#define _MODEL_H__
#include "fvector.h"
// This is our LR Model
struct model {
  int nRows, nCols;
  double moviemean;
  FVector *L, *R;

  model() { L = NULL; R = NULL; }
  model(int nr, int nc, double mm, FVector *_l, FVector *_r);
  ~model() {
    // if(L != NULL) { delete [] L; }
    // if(R != NULL) { delete [] R; } 
  }
};

struct model_with_means : public model {
  double *l_means, *r_means;
  model_with_means() : model() { l_means = NULL; r_means = NULL; } 
  model_with_means(int nr, int nc, double mm, FVector *l, FVector *r, double *_l_means, double *_r_means) : model(nr,nc,mm,l,r) {
    l_means = _l_means;
    r_means = _r_means;
  }
};


// Testing and prediction code
double predict(const struct model &m, int row, int col);
double compute_loss(const struct model &m, const struct example &e);
double model_mse(const struct model &m, const struct example &e);

double predict(const struct model_with_means &m, int row, int col);
double compute_loss(const struct model_with_means &m, const struct example &e);
double model_with_means_rmse(const struct model_with_means &m, const struct example &e);

struct model *
build_model(int _nRows, int _nCols, double _moviemean,
	    FVector *l, FVector *r);
struct model_with_means *
build_model_with_means(int _nRows, int _nCols, double _moviemean,
		       FVector *l, FVector *r, double* l_means, double *r_means);
#endif
