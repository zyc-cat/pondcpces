#ifndef REPLAN_H
#define REPLAN_H

#include "anytime_search.h"
#include "astar_advance.h"

#include <deque>
#include <set>

// Forward Declarations
class StateNode;

class Replan : public AnytimeSearch, AStarAdvance{
public:
  Replan(bool determinized = false);
  virtual ~Replan();

  bool solve(StateNode* start);
};

#endif
