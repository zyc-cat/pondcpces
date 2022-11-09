#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
//using namespace std;
#ifndef RANDOMGEN_HH
#define RANDOMGEN_HH
class randomGenerator {

  public:
  randomGenerator(int c) : capacity(c) {
    current_rand = 0;
    rands = new int[capacity];
    for(int i = 0; i < capacity; i++){
      rands[i] = -1;
    }
  };

  ~randomGenerator(){ delete [] rands; }

  int myrand() {
    //cout << "calling rand" << endl;
    int ret;
    if(rands[current_rand] == -1){//generate random number
      rands[current_rand] = rand();
    }
    ret = rands[current_rand];
    if(current_rand == (capacity-1)){
      current_rand = 0;
      //cout << "Hit capacity" << endl;
    }
    else
      current_rand++;
    return ret;
  }

 private:
  int* rands;
  int current_rand;
  int capacity;

};

#endif
