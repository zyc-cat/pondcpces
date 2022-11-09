#include "randomGenerator.h"


#define NUM_RANDS 1000000
#define NUM_CALLS 1000000000

int main(){
  
  srand(time(NULL));

  randomGenerator r (NUM_RANDS);


  for(int i = 0; i < NUM_CALLS; i++){
    //cout << "i = " << i << " : " << r.myrand() << endl;
    r.myrand();
  }

  return 0;
}
