#ifndef PCHOP_H
#define PCHOP_H

#include "sampled_search.h"
#include "pchop_advance.h"

class PCHOP : public SampledSearch, public PCHOPAdvance{
public:
  PCHOP(int numSamples = 10, int maxDepth = INT_MAX);

protected:
  // Return best action to take for the state, according to sampling
  ActionNode* findBestAction(StateNode* state) { return PCHOPAdvance::findBestAction(state); }
};

#endif  // PCHOP_H
