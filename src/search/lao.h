#ifndef LAO_H
#define LAO_H

#include "search.h"
#include "graph.h"

#include <list>

class LAOStar : public Search{
public:
  LAOStar();

  virtual void search();
  
protected:
  void uncheckAncestors(struct StateNode *node);
  void DisplaySolution();

  void ExpandSolution(struct StateNode *node, int currHorizon, int dfs_depth, double path_pr);
  void ExpandNode(struct StateNode *node, int);
  StateList *findAncestors(StateList *ExpandedStateList);
  int ConvergenceTest();
  int ConvergenceTestRecursive(struct StateNode *node, int currHorizon, double path_pr);
  StateList *DisplaySolutionRecursive(struct StateNode *node);

  int NumExpandedStates;        // 拓展的结点数
  int Iteration;                // 迭代次数
  int NumExpandedStatesIter;    // 本次迭代拓展结点数
  int NumAncestorStatesIter;    // 本次迭代祖先结点数
  int NumSolutionStates;        // solution中的状态数
  StateList *ExpandedStateList; // 拓展结点
  int CountBackups;
  double Residual;
  int ExitFlag;
  int converged_to_goal;

  int dfs_limit;

  std::list<StateNode*> terminals;

  StateNode* lastBackup;
};

#endif
