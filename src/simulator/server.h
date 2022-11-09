#ifndef __SERVER_H
#define __SERVER_H

#include "cudd/dd.h"
//#include "cudd/cudd.h"

#ifndef FOREVER
#define FOREVER 0 // if the server lasts forever
#endif

class server{
 private:
  int port;
  DdNode* current_state;

 public: 
  server(int p) : port(p) {current_state=0;};
  int start();
  int session(int, struct sockaddr_in);
  DdNode* pickState(DdNode*);
  void setState(DdNode*);
  DdNode* getState();
  DdNode* updateState(char*);
  

};


#endif
