#ifndef MOLAO_H
#define MOLAO_H

#include "search.h"
#include "graph.h"

#include <list>

class MOLAOStar : public Search{
public:
  MOLAOStar();

  virtual void search();
  
protected:
  void expandSolution(struct StateNode *node, int currHorizon, int dfs_depth, double path_pr);
  void expandNode(struct StateNode *node, int currHorizon);
  StateList* findAncestors(StateList *ExpandedStateList);
  bool convergenceTest();
  bool convergenceTestRecursive(struct StateNode *node, int currHorizon, double path_pr, bool checkingNonSolutionPt);

  int NumExpandedStates;
  int Iteration;
  int NumExpandedStatesIter;    /* number of states expanded this iteration */
  int NumAncestorStatesIter;
  int NumSolutionStates;
  StateList *ExpandedStateList;
  double Residual;
  int ExitFlag;

  std::list<StateNode*> terminals;
  StateNode* lastBackup;
};

#endif
