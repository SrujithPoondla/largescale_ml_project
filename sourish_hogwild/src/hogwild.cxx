#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "global_macros.h"
#include "timer.h"
#include "examples.h"
#include "fvector.h"
#include "pthread.h"
#include "simple_random.h"
#include "hogwild.h"
#include "lock_util.h"

#include <vector>
using namespace std;
class FVector;

typedef vector<struct example> example_vec;
typedef vector<struct example>::iterator example_vec_it;

namespace hogwild {
  // THIS CHANGE IS NOT PUSHED THROUGH YET!!!! I switched to freeforall
  GLOBAL_LOCK_ONLY(pthread_rw_lock_t _global_lock;);

void
inline
find_bin(int row,        int col, 
	 int row_divide, int col_divide, 
	 int &k,         int &l) {
  k = (row >= row_divide) ? 1 : 0;
  l = (col >= col_divide) ? 1 : 0;
}

void 
permute(simple_random &rand, int *d, int n) {
  for(int i = n-1; i > 0; i--) {
    int rand_index = rand.rand_int(i); // pick something [0,i]
    int temp       = d[i];
    d[i]           = d[rand_index];
    d[rand_index]  = temp;    
  }
  VERBOSE_ONLY(
	       for(int i =0; i < n; i++) {
		 cout << i << " -> " << d[i] << endl;
	       }
	       )
}

struct permute_thread_info {simple_random &rand; int *r, n;
  permute_thread_info(simple_random &_rand, int *_r, int _n) : rand(_rand), r(_r), n(_n) { } 
};

void*
permute_thread( void* p ) {
  struct permute_thread_info *pti = (struct permute_thread_info*) p;
  permute(pti->rand, pti->r, pti->n);
  return NULL;
}

// *****************************
// *THREADING
struct gradient_thread_info_new {
  int id, nWorkers;
  FVector *L, *R;
  double *L_means, *R_means;

  double &moviemean, cur_step_size;
  // Optimization problem
  bool tracenorm; // false we do ball project
  // *************************************
  // Information for proximal points.
  int *L_degree, *R_degree; /// possibly null
  double B, mu;
  struct example* ex;
  int nExamples;
  int *perm;
  gradient_thread_info_new(int _id, int _nWorkers, int *_perm, struct example*_ex, int _nExamples, 
			   FVector *_L, int *_L_degree, double *_L_means,
			   FVector *_R, int *_R_degree, double *_R_means,
			   double &_moviemean, double _cur_step_size, double _B, double _mu, bool _tracenorm)  : moviemean(_moviemean)
  { id = _id; nWorkers = _nWorkers; ex = _ex; nExamples = _nExamples;
    L = _L; R= _R; perm = _perm;
    L_degree = _L_degree; R_degree = _R_degree; 
    L_means  = _L_means ; R_means = _R_means;
    moviemean = _moviemean; cur_step_size = _cur_step_size; B = _B; mu = _mu;
    tracenorm = _tracenorm; 
  }
};
void*
gradient_thread_main(void * params) {
  gradient_thread_info_new *gti = (struct gradient_thread_info_new*) params;
  // read the params
  int id               = gti->id;
  int nWorkers         = gti->nWorkers;

  int *perm            = gti->perm;
  FVector *L           = gti->L;
  FVector *R           = gti->R;
  struct example* example = gti->ex;
  int nExamples        = gti->nExamples;
  double &moviemean     = gti->moviemean;
  double cur_step_size = gti->cur_step_size;
  double B             = gti->B;
  double mu            = gti->mu;
  bool  _tracenorm     = gti->tracenorm;

  int     *L_degree    = gti->L_degree;
  int     *R_degree    = gti->R_degree;  
  double  *L_means     = gti->L_means;
  double  *R_means     = gti->R_means;
  // Make sure that we are good.
  assert((R_degree != NULL && L_degree != NULL) || !_tracenorm);

  // get to work.
  DEBUG_ONLY(timer part_timer(true););
  double B_squared     = B*B;
  FVector _tempL;
 
  int start_offset     = id*(nExamples/nWorkers);
  int end_offset       = min(nExamples, (id+1)*(nExamples/nWorkers));
  DEBUG_ONLY(cout << "start=" << start_offset << " -- " << end_offset << " " << nExamples << " nWorkers=" << nWorkers << " id=" << id << endl;)

  // HERE !!!! 
  if(_tracenorm) {
    for(int i = start_offset; i < end_offset; i++) {
      int pi = perm[i];
      int row_index = example[pi].row;
      int col_index = example[pi].col;
      double rating = example[pi].rating;
       
      //    err = sum(L(row_idx,:).*R(col_idx,:),2)+moviemean-double(rating);
      GLOBAL_LOCK_ONLY( check_result("Read Tracenorm", pthread_rw_lock_rdlock(&global_lock)); );
      double err     = FVector::dot(L[row_index],R[col_index]) + (L_means[row_index] + R_means[col_index])/2.0 + moviemean - rating;     
      // double err     = FVector::dot(L[row_index],R[col_index]) + moviemean - rating;
      // E = double(stepsize(epoch)*err);	
      double e       = -(cur_step_size * err);

      if( (i+ id*31) % 1000 == 0) {
	moviemean          += e / (double) nExamples * 1000.0;
      }
      
      L_means[row_index] += e / (double) L_degree[row_index] / 2.0;
      R_means[col_index] += e / (double) R_degree[col_index] / 2.0;
      // L_means[row_index] += e / 3.0;
      // R_means[col_index] += e / 3.0;

      
     _tempL = L[row_index];
     _tempL.scale(1 - mu*cur_step_size/((double) L_degree[row_index]));
     _tempL.scale_and_add(R[col_index],e);	 
     GLOBAL_LOCK_ONLY( check_result("Unlock Readlock Tracenorm", pthread_rwlock_unlock(&global_lock)); );

     // Upgrade to write lock
     GLOBAL_LOCK_ONLY( check_result("Write lock Tracenorm", pthread_rw_lock_wrlock(&global_lock)); );
     R[col_index].scale(1 - mu*cur_step_size/((double) R_degree[col_index])); 
     R[col_index].scale_and_add(L[row_index],e);	      
     // Store back L
     L[row_index] = _tempL;
     GLOBAL_LOCK_ONLY( check_result("Unlock Readlock Tracenorm", pthread_rwlock_unlock(&global_lock)); );
     }

  } else {
    for(int i = start_offset; i < end_offset; i++) {
      int pi        = perm[i];
      int row_index = example[pi].row;
      int col_index = example[pi].col;
      double rating = example[pi].rating;
      
      GLOBAL_LOCK_ONLY( check_result("READ Maxnorm", pthread_rw_lock_rdlock(&global_lock)); );
      //    err = sum(L(row_idx,:).*R(col_idx,:),2)+moviemean-double(rating);
      double err     = FVector::dot(L[row_index],R[col_index]) + moviemean - rating;
      
	// E = double(stepsize(epoch)*err);	
      double e       = -(cur_step_size * err);
	
      _tempL = L[row_index];
      _tempL.scale_and_add(R[col_index],e);	 
      R[col_index].scale_and_add(L[row_index],e);      
      // Store back L
      L[row_index] = _tempL;
	
      // Do the projection if needed
      L[row_index].ball_project(B, B_squared);	  
      R[col_index].ball_project(B, B_squared);
    }
  }
  DEBUG_ONLY(cout << " part id=" << id << " took " << part_timer << " on " << (end_offset - start_offset) << " examples " << endl;);
  //delete gti;
  return NULL;
}

int*
init_permutation(int nSize) {
  int *ret = new int[nSize];
  for(register int i = nSize - 1; i >= 0; i--) { ret[i] = i; }
  return ret;
}


  struct model_with_means
//szTrainFile, t, epochs, max_rank, initial_stepsize, stepsize_diminish, nRowSlices, nColSlices
hogwild_jellyfish(struct training_parameters<struct model_with_means> tp) {
    test_handler<struct model_with_means, struct example> *t = tp.t;
  int nEpochs = tp.nEpochs;
  int max_rank =tp.max_rank;
  double initial_step_size = tp.initial_step_size, step_diminish = tp.step_diminish;  
  double divergent_series_rule = 0.0;
  bool bTraceNorm = tp.bTraceNorm;
  double B = tp.B;
  double mu = tp.mu;
  int nShufflers = tp.nShufflers;
  int nWorkers   = tp.nSplits; 
  simple_random _rand(tp.seed);

  // Initialize locking if needed
  GLOBAL_LOCK_ONLY( lock_error_check("Global Lock Initialize", pthread_rwlock_init(&global_lock, NULL)) ; );


  // parameters
  double cur_step_size = initial_step_size;
  cout << "# [hogwild]  Parameters:" << endl;
  cout << "# [hogwild]  nepochs=" << nEpochs << " B=" << B << " max_rank=" << max_rank << " nSplits=" << tp.nSplits << " nShufflers=" << nShufflers << endl;
  cout << "# [hogwild]  initial_stepsize=" << initial_step_size << " step_diminish=" << step_diminish << " divergent_rule=" << divergent_series_rule << endl;
  cout << "# [hogwild]  tracenorm=" << bTraceNorm << " mu=" << mu << endl;
  // int nSteps = 0;
  timer whole_jellyfish(true);
  
  // set by load examples

  DEBUG_ONLY(timer data_load(true););
 
  int nRows = tp.nRows, nCols = tp.nCols, nExamples = tp.nExamples; 
  struct example *examples    = tp.examples;
  cout << "# [hogwild]  nExamples=" << nExamples << " nRows=" << nRows << " nCols=" << nCols << endl;
  // Create the Test Handler
  assert(examples != NULL);
  //cout << " \t tp.examples=" << tp.examples << endl;

  test_handler<struct model_with_means, struct example>  train_tester(examples, nExamples, &model_with_means_rmse);

  
  // ***
  // intialize L and R
  // **************************
  // Divide up the data
  // Need a double array of FVectors(30)s L, R
  FVector::_default_n = max_rank; // this helps make allocation easy. 
 
  // TODO: Remove these for an extra copy.
  // Build the model
  struct model_with_means m, *initial_guess;
  initial_guess = tp.initial_guess;
  FVector *L,*R;
  double *L_means = NULL, *R_means = NULL;
  if(initial_guess == NULL) {
    m.nRows = nRows; m.nCols = nCols;
    L  = new FVector[nRows];
    R  = new FVector[nCols];      
    L_means  = new double[nRows]; bzero(L_means, sizeof(double)*nRows);
    R_means  = new double[nCols]; bzero(R_means, sizeof(double)*nCols);
  } else {
    m.nRows = nRows; m.nCols = nCols;
    assert(initial_guess->nRows == nRows);
    assert(initial_guess->nCols == nCols);
    L = initial_guess->L;
    R = initial_guess->R;
    m.moviemean = initial_guess->moviemean;
  }
  m.L = L;
  m.R = R;
  m.l_means = L_means;
  m.r_means = R_means;

  // *** Compute Stats about the Examples
  double moviemean = 0.0;
  int *L_degree = NULL, *R_degree = NULL;
  if(bTraceNorm) { 
    // the outdegree of each row
    L_degree = new int[nRows]; bzero(L_degree, sizeof(int)*nRows); 
    R_degree = new int[nCols]; bzero(R_degree, sizeof(int)*nCols); 
  }

  for(int i = 0; i < nExamples; i++) {
    if(bTraceNorm) { // TODO: Hoist this computation.
      L_degree[examples[i].row] ++;
      R_degree[examples[i].col] ++;
      L_means[examples[i].row] += examples[i].rating;
      R_means[examples[i].col] += examples[i].rating;
    }
    moviemean += examples[i].rating;
  }
 

  moviemean /= ((double) nExamples);
 
  if(bTraceNorm) {
    for(int i = 0; i < nRows; i++) {
      if(L_degree[i] > 0) { L_means[i] /= (double) L_degree[i]; L_means[i] -= moviemean; }
    }
    for(int i = 0; i < nCols; i++) {
      if(R_degree[i] > 0) { R_means[i] /= (double) R_degree[i]; R_means[i] -= moviemean; }
    }
  }
  if(initial_guess == NULL) {m.moviemean = moviemean;}
 
  DEBUG_ONLY(data_load.stop(););
  DEBUG_ONLY(cout  << "Data Loaded: N=" << nExamples << " c=" << nCols << " r=" << nRows << " in " << data_load.elapsed() << "s" << endl;);


  int *shared_perm[2];
  for(int i = 0; i < 2; i++) { shared_perm[i] = init_permutation(nExamples); }
  struct permute_thread_info* ptis[2] = { new permute_thread_info(_rand, shared_perm[0], nExamples), new permute_thread_info(_rand, shared_perm[1], nExamples) };

  gradient_thread_info_new* wtis[nWorkers];
  for(int i = 0; i< nWorkers; i++) {
    wtis[i] = new gradient_thread_info_new(i, nWorkers, shared_perm[0], examples, nExamples,
					   L, L_degree, L_means, R, R_degree, R_means, m.moviemean, cur_step_size, B, mu, bTraceNorm);
  }

  permute(_rand, shared_perm[0], nExamples);
  DEBUG_ONLY(cout  << "Model built. Ready to start." << endl;);
  for(int epoch = 0; epoch < nEpochs; epoch++) {
    timer perm_timer(true);
    timer epoch_timer(true);
    double perm_time = perm_timer.stop_elapsed();
    DEBUG_ONLY(cout << "Permutations Completed in " << perm_timer << endl;);
    DEBUG_ONLY(timer vs_timer(true););
    pthread_t shuffler_t;
    int cur_epoch = epoch % 2;
    int next_epoch = (epoch + 1) % 2;

    int ret = pthread_create( &shuffler_t, NULL, permute_thread , (void*) ptis[next_epoch]);
    if(ret != 0  ) { 
	cout << "Error in pthread_create: " << ret << endl;
	exit(-1); 
    }

    pthread_t workers[nWorkers];
    for(int i = 0; i < nWorkers; i++) {
      wtis[i]->perm = ptis[cur_epoch]->r;
      int ret = pthread_create( &workers[i], NULL, gradient_thread_main, (void*) wtis[i]);
      if(ret != 0  ) { 
	cout << "Error in pthread_create: " << ret << endl;
	exit(-1); 
      }
    }
    // Thread Join
    for(int i = 0; i < nWorkers; i++) {
	pthread_join(workers[i], NULL);
    }
    pthread_join( shuffler_t, NULL );
    // bookkeeping on end of each epoch
    cur_step_size *= step_diminish;
  
    // End Epoch
    double epoch_duration = epoch_timer.stop_elapsed();
    DEBUG_ONLY(cout << "[TRAIN] epoch " << epoch + 1 << " took " << epoch_timer << " running time: " << whole_jellyfish << endl; );

    // Now compute the RMSEs
    double train_rmse = 0.0, test_rmse = 0.0;
    train_rmse = (nWorkers > 1) ? train_tester.test_multithread(m, nWorkers) : train_tester.test(m);
    DEBUG_ONLY(cout << "[TRAIN] " << endl; );
    if(t) test_rmse = (nWorkers > 1) ? t->test_multithread(m, nWorkers) : t->test(m);
    
    if(tp.bPrint) {
      cout << "[Epoch Stats] epoch: " << epoch + 1 << " epoch_time: " << epoch_duration << " total_time: " << whole_jellyfish << " perm_time " << perm_time << " train_rmse: " << train_rmse << " test_rmse: " << test_rmse << endl;      
    }
  }
  // Cleanup code.


  GLOBAL_LOCK_ONLY( lock_error_check("Global lock destroy", pthread_rwlock_destroy(&rwlock) ); );
  
  // quiet down the threads.
  //delete [] shared_perm;
  // delete wtis;

  DEBUG_ONLY(cout << "[hogwild] finished in " << whole_jellyfish << endl;);
  return m;
}

}
