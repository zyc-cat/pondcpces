#ifndef SAMPLED_SEARCH_H
#define SAMPLED_SEARCH_H

#include "anytime_search.h"

#include <set>

// Forward Declarations
class StateNode;
class ActionNode;

// Sampled Search class
class SampledSearch : public AnytimeSearch{
protected:
  SampledSearch(SearchType inheriting_type);

protected:
  virtual bool solve(StateNode* start);

  // Return best action to take for the state, according to sampling
  virtual ActionNode* findBestAction(StateNode* state);
};

#endif  // SAMPLED_SEARCH_H
