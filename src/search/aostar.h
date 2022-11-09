#ifndef AOSTAR_H
#define AOSTAR_H

#include "search.h"
#include "step_search.h"
#include "graph.h"
#include <queue>
#include <list>


class AOStar : public StepSearch
{
public:
   AOStar();
   void setup(StateNode *start);
   bool step() { return true; }
   virtual void search();// override from StepSearch
   bool expand(StateNode* node);
   bool new_action_transition(StateNode* cur, ActionNode* act, DdNode* successor);
   bool new_sensing_transition(StateNode *cur, ActionNode *act, DdNode *c1, DdNode *c2);
   StateNode *first_unexpanded();
   void goal_propagate(StateNode *n1, StateNode *n2);// TO.DO.
   void goal_propagate(StateNode *par, StateNode *n1, StateNode *n2);// TO.DO
   void reconnection_propagate(StateNode *node);
   void isolation_propagate(StateNode *node);
   void dead_propagate(StateNode *node);
   void solution_print(StateNode*, int);
   void depth_propagate(StateNode *);
   double getQuality() {};

protected:
   std::set<StateNode *> closed;
   std::set<StateNode *, StateComparator> open;
   std::map<StateNode *, int> m_commandNo;
   int m_depth;
};
#endif
