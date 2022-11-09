#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <utility>          // pair
#include "globals.h"
#include "lug.h"
#include "solve.h"
using namespace std;

double factorial(int k){
  double result = 1.0;
  for(int i = 2; i <= k ; i++){
    result *= (double)i;
  }
  return result;
}

double choose(int n, int k){
  //return (factorial(n)/(factorial(k)*factorial(n-k)));

  double result = 1.0;

  //  cout << "choose(" << n << ", " << k << ")" << endl;

  for(int i = k+1, j = 2; (i <= n || j <= n-k); i++, j++){
    //    cout << "i = " << i << " j = " << j << endl;
    if(i <= n && j <= n-k){
      result *= ((double)i/(double)j);
    }
    else if(i <= n){
      result *= (double)i;
    }
    else if(j <= n-k){
      result *= 1.0/(double)j;
    }

  }
  //    cout << "result = " << result << endl;
  return result;


}

double F(double c, int n, double p){
  double result = 0.0;
  for(int i = 0; i <= c; i++){
    result += choose(n, i) * pow(p, i) * pow(1.0 - p, n - i);
    //    cout << "result = " << result <<endl;
  }
  //  cout << "F(" << c << ", " << n << ", " << p << ") = "
  //       << result << endl;
  return result;
}

double Ftilde(double x, int n, double p){
  double result = (F(floor(x), n, p) + F(ceil(x), n, p))/2.0;
  return result;
}

double FtildeInv(double y, int n, double p){
 double result = 0.0;
 int i = 0;
 double t1, t2, t3;

 for(; result < y; i++){
   t1 = choose(n, i);
   t2 = pow(p, i);
   t3 = pow(1.0 - p, n - i);

   //    cout << "t1 = " << t1 
   // 	<< " t2 = " << t2 
   // 	<< " t3 = " << t3
   // 	<< endl;

   result += t1 * t2 * t3;

     //    result += choose(n, i) * pow(p, i) * pow(1.0 - p, n - i);
   //   cout << "F: " << result <<endl;
  }
 return i;//result;
}

double invCumulative(double x){
  double a0 = 2.30753,
    a1 = 0.27061,
    b1 = 0.99229,
    b2 = 0.04481,
    eta = pow(-1 * log(pow(x, 2)), 0.5),
    result = -1 * eta + (a0 + (a1 * eta))/(1 + (b1 * eta) + (b2 * pow(eta, 2)));

  return result;
}

int expected_samples(int n, int c, 
		     double beta, double alpha, 
		     double p, double theta, double delta){ 
  //2.17, pg. 29

  double numer, denom, lp, p0, p1, s;
  int result;

  p0 = (1.0 < theta + delta ? 1.0 : theta + delta);
  p1 = (0.0 > theta - delta ? 0.0 : theta - delta);

  s = (log((1.0 - p0) / (1.0 - p1)))/(log((p1 * (1.0 - p0)) / (p0 * (1.0 - p1))));


//     cout << "p0 = " << p0 
//          << " p1 = " << p1 
//  	<< " p = " << p
//  	<< endl;


   if(p == 0){
     lp = 0;
     numer = 
       log((1.0 - beta) / alpha);
     denom =
       log((1.0 - p1) / (1.0 - p0));
   }
   else if(p == p1){
     lp = beta;
     numer = 
       lp *       log(beta/(1.0 - alpha)) + 
       (1.0 - lp) * log((1.0 - beta)/alpha);
     denom = 
       p1 *       log(p1/p0) + 
       (1.0 - p1) * log((1.0 - p1)/(1.0 - p0));
   }
   else if(p == s){
     lp = log((1.0 - beta)/alpha) / (log((1.0 - beta)/alpha) - log(beta/(1.0 - alpha)));
     numer = 
       -1.0 * log(beta/(1.0 - alpha)) * log((1.0 - beta)/alpha);
     denom = 
       log(p1 / p0) * log((1.0 - p0) / (1.0 - p1));
   }
   else if(p == 1){
     lp = 1;
     numer = 
       log(beta / (1.0 - alpha));
     denom = 
       log(p1 / p0);
   }
   else if(p == p0){
     lp = 1.0 - alpha;
     numer = 
       (1.0 - alpha) * log(beta / (1.0 - alpha)) + alpha * log((1.0 - beta) / alpha);
     denom = 
       p0 * log(p1 / p0) + (1.0 - p0) * log((1.0 - p1) / (1.0 - p0));
   }
   else{
     lp = 1.0 - F(c, n, p);
     numer = 
       (lp *       log(beta/(1.0 - alpha))) + 
       ((1.0 - lp) * log((1.0 - beta)/alpha));
     denom = 
       (p *       log(p1/p0)) + 
       ((1.0 - p) * log((1.0 - p1)/(1.0 - p0)));


//      cout << (lp *       log(beta/(1.0 - alpha))) << " "
// 	  <<((1.0 - lp) * log((1.0 - beta)/alpha)) << " "
// 	  <<(p *       log(p1/p0))  << " "
// 	  << ((1.0 - p) * log((1.0 - p1)/(1.0 - p0))) << " "
// 	  << endl;




  }
     result = numer/denom;
 
//    if(result == 0)
//      result = 1;

//    cout << "numer = " << numer 
//         << " denom = " << denom 
//         << " result = " << result
//         << endl;

   if(result < 0)
     result = expected_sample_size + 1;
   else if(result > total_sample_size)
     result = total_sample_size;

  return result;

}


int next_batch_size(){
  return 1;
}

int approx_samples(double alpha,
		      double beta, 
		      double theta,
		      double delta){

  int n;
  double phiInvA = invCumulative(alpha), 
    phiInvB = invCumulative(beta),
    p0, p1, numer, denom;

  p0 = (1.0 < theta + delta ? 1.0 : theta + delta);
  p1 = (0.0 > theta - delta ? 0.0 : theta - delta);

  denom = pow(p0 - p1, 2);

  numer = pow((phiInvA * pow(p0 * (1 - p0), 0.5)) +
	      (phiInvB * pow(p1 * (1 - p1), 0.5)),
	      2);

  n = (int) (numer/denom);

  //  cout << "n = " << n  << endl;

  return n;

}


pair<int, int>* guess_num_samples(double alpha,
				 double beta, 
				 double theta,
				 double delta){


  int nmin = 1, nmax = -1, n, c0, c1, c;
  double x0, x1, p0, p1;
  

  p0 = (1.0 < theta + delta ? 1.0 : theta + delta);
  p1 = (0.0 > theta - delta ? 0.0 : theta - delta);

//    cout << "p0 = " << p0 
//         << " p1 = " << p1 << endl;

  if(p1 == 0 && p0 == 1){
    n = 1;
    c = 0;
  }
  else if(p1 == 0 && p0 < 1){
    n = ceil((log(alpha)) / (log(1-p0)));
    c = 0;
  }
  else if(p1 > 0 && p0 == 1){
    n = ceil((log(beta)) / (log(p1)));
    c = n - 1;    
  }
  else{
    if(1){
      n = approx_samples(alpha, beta, theta, delta);
      n--;
    }
    else{
      
      n = nmin;
      while(nmax < 0 || nmin < nmax){
	x0 = //1.0 - Ftilde(alpha, n, p0);
	  FtildeInv(alpha, n, p0);
	//(1.0/Ftilde(alpha, n, p0));
	x1 = //1.0 - Ftilde(1.0-beta, n, p1);
	  FtildeInv(1.0-beta, n, p1);
	//(1.0/Ftilde(1.0-beta, n, p1));
	
	//      cout << "x0 = " << x0 
	// 	   << " x1 = " << x1  
	// 	   << " n = " << n <<endl;
	
	if(x0 >= x1 && x0 >= 0.0)
	  nmax = n;
	else
	  nmin = n + 1;
	
	if(nmax < 0.0)
	  n *= 2;
	else
	  n = floor((nmin + nmax)/2.0);
      }
      n = nmax - 1;
    }
    //    cout << "n = " << n << endl;
    
    do{
      n++;
      c0 = floor(FtildeInv(alpha, n, p0));
      c1 = ceil(FtildeInv(1.0-beta, n, p1));
//       cout << "c0 = " << c0 
// 	   << " c1 = " << c1  
// 	   << " n = " << n <<endl;
    } while(c0 < c1);
    
    c = floor((c0 + c1)/2.0);
  }
  
  cout << "n = " << n 
       << " c = " << c 
       << endl;
  return new pair<int, int>(n, c);
}



double samples_ratio(int dm, int m, double theta, double delta){

  double p1m, p0m, p0, p1, fm;

  p0 = (1.0 < theta + delta ? 1.0 : theta + delta);
  p1 = (0.0 > theta - delta ? 0.0 : theta - delta);
    
  return (dm * log(p1/p0)) + ((m - dm) * log((1-p1)/(1-p0)));

}

static int MAXITER = 0;
int SPRT(StateNode *a, StateNode *b, bool use_g_val){

  bool accept = false, reject = false;
  int iteration = -1, max_iterations = MAX_H_INCREMENT-1, used_iteration = -1;
  bool draw;
  bool outcome = false; //0 favors a, 1 favors b
  int outcomes = 0;
  double aval, bval;
  int awins = 0, bwins = 0;


  double A = log((1.0-beta)/alpha);
  double B = log(beta/(1.0-alpha));
  double ratio;


  double MIN_DIFF_FOR_SPRT = 0.1;

  // cout << endl << "-----------"<<endl;
  
  //accept means that one is better, reject means they are same
  while(!accept && !reject){
    iteration++;

    //cout << "iter = " << iteration << endl;
    if(iteration >= max_iterations){
      break;//return (awins > bwins);
    }

    //get outcome for iteration
    if(a->hIncrement <= iteration)
      increment_heuristic(a);
    if(b->hIncrement <= iteration)
      increment_heuristic(b);

    if(use_g_val){
      aval = gWeight * a->hValues[iteration] + a->g;
      bval = gWeight * b->hValues[iteration] + b->g;
    }
    else{
      aval = a->hValues[iteration];
      bval = b->hValues[iteration];
    }

    // cout << aval << " " << bval << endl;


 //    if(fabs(aval-bval) > MIN_DIFF_FOR_SPRT*(aval > bval ? aval : bval)){
//       accept = (aval < bval);
//       reject = !accept;
//       break;
//     }

    draw = (aval == bval);
    if(draw){
      continue;
      //outcome = rand()%1;
    }
    else{
      outcome = (aval < bval);
    }
    used_iteration++;

    

    awins = (outcome ? awins+1 : awins);
    bwins = (!outcome ? bwins+1 : bwins);
    outcomes = (outcome ? outcomes+1 : outcomes);

    ratio = samples_ratio( outcomes, used_iteration+1, 0.5, delta);
    //cout << ratio << endl;

    accept = (ratio <= B);
    reject = (ratio >= A);

  }
//   cout << endl <<"accept = " << accept 
//         << " reject = " << reject 
//         << " iter = " << iteration 
//         << endl;
    
   //  cout << "iter = " << iteration << endl;
//   if(iteration > MAXITER){
//     MAXITER = iteration;
//     //    cout << "MAX = " << MAXITER << flush;//endl;
//   }

  // cout <<"==============="<<endl;
  //cout << awins << " " << bwins << endl;

  if(accept){
     return -1;
  }
  else if (reject)
    return 1;
  else 
    return 0;//(a->hValues[iteration] < b->hValues[iteration] ? -1 : 1);
}
