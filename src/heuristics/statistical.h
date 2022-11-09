#ifndef STATISTICAL_H
#define STATISTICAL_H

std::pair<int, int>* guess_num_samples(double alpha,
				 double beta, 
				 double goal_threshold, 
				 double delta);
double samples_ratio(int dm, int m, double theta, double delta);
int expected_samples(int n, int c,
		     double beta, double alpha, 
		     double p, double theta, double delta);
int next_batch_size();
int SPRT(StateNode *a, StateNode *b, bool use_g_val = true);
#endif
