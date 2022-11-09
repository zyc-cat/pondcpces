#ifndef MOASTAR_H
#define MOASTAR_H

#include "search.h"

#include <list>
#include <set>

class MOValueFunction;
class MOValueFunctionPoint;
class StateNode;
class ActionNode;

class MOAStar : public Search{
public:
  MOAStar();

  virtual void search();
  
protected:
  void addNonDominatedPointsMOAstar(MOValueFunction* newPts, MOValueFunction* existingPts, struct StateNode* newPtsState);
  bool updateStateValue(ActionNode* actionNode, StateNode* successor, StateNode* n, MOValueFunction* NDpoints);
  void findNDKungRecurse(std::list<MOValueFunctionPoint*>* population);
  bool findNDKung(std::set<StateNode*>* NDstates, MOValueFunction* NDpoints, std::set<StateNode*>* Open, MOValueFunction* NDsolutions);
  bool findND(std::set<StateNode*>* NDstates, MOValueFunction* NDpoints, std::set<StateNode*>* Open, MOValueFunction* NDsolutions);

  void selectFirstNodeToExpand(std::set<StateNode*>* NDstates, std::list<StateNode*>* nodesToExpand);
  void selectRandomNodeToExpand(std::set<StateNode*> *NDstates, std::list<StateNode*> *nodesToExpand);
  void selectMostNDNodeToExpand(std::set<StateNode*>* NDstates, MOValueFunction* NDpoints, std::list<StateNode*>* nodesToExpand);
  void selectLargestHvalue(std::set<StateNode*>* NDstates, MOValueFunction* NDpoints, std::list<StateNode*>* nodesToExpand);
  void selectNodesToExpand(std::set<StateNode*>* NDstates, MOValueFunction* NDpoints, std::list<StateNode*>* nodesToExpand);
};

#endif
