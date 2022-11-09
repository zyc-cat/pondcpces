#ifndef EHC_H
#define EHC_H

#include "search.h"

#include "globals.h"            // StateHash

class EHC : public Search{
public:
  EHC();

  virtual void search();
  
protected:
  bool f_dominance(StateNode* lhs, StateNode* rhs);
  bool card_less_than(StateNode* a, StateNode* b);
  void printPlan(StateNode* tail);
  void setBestAct(StateNode* s);
  void backupSet(__gnu_cxx::StateHash* nodes);
  StateNode* findBetterNode(StateNode* root);
  StateNode* recursive(StateNode* root);

  __gnu_cxx::StateHash NodesExpanded;
  int ExpandedStates;
  bool reEvaluateChildren;
  bool INITLUG;// 是否使用LUG
  double MOMENTUM;
};

#endif
