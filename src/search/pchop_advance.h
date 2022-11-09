#ifndef PCHOP_ADVANCE_H
#define PCHOP_ADVANCE_H

#include <limits.h>             // INT_MAX
#include <set>

// Forward Declarations
class StateNode;
class ActionNode;
class PolicyState;

class PCHOPAdvance{
protected:
  PCHOPAdvance(int numSamples = 10, int maxDepth = INT_MAX);

  ActionNode* findBestAction(StateNode* state);

  void renewFringe(std::set<PolicyState*> &fringe, PolicyState& root);

  int numSamples;
};

#endif  // PCHOP_ADVANCE_H
