#ifndef RTDP_H
#define RTDP_H

#include "search.h"

class StateNode;

class RTDP : public Search{
public:
  RTDP(int nTrial);

  virtual void search();
  
protected:
  void ExpandNode(struct StateNode *node);
  struct StateNode* SimulateNextState(struct StateNode *currentState);

  int nTrial;
  int trial;
  int stateExpanded;
  int stateExpandedTrial;
  int Iteration;
  int NumExpandedStates;
};

#endif
