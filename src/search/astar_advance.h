#ifndef ASTAR_ADVANCE_H
#define ASTAR_ADVANCE_H

#include "graph.h"									// StateComparator
#include <set>
#include <deque>

class AStarAdvance{
protected:
  AStarAdvance(bool determinized = false);// 默认非确定

  StateNode* advance(StateNode* start);
  StateNode* advance(std::set<StateNode*>& states);

  void updateSuccessors(StateNode* start, double delta_g, double oldPrReached, double newPrReached);

  void setBestAct(StateNode* state);

	std::set<StateNode*, StateComparator> open;// 待查找的状态节点，采用启发值比较的方式

  bool determinized;

  int iteration;
};

#endif  // ASTAR_ADVANCE
