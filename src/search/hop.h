#ifndef HOP_H
#define HOP_H

#include "step_search.h"

#include <vector>
#include <map>

// Forward Declarations
class StateNode;
class ActionNode;
class HOPAdvance;

class HOP : public StepSearch{
public:
  HOP(int numSamples = 1);

	void setup(StateNode* start);
	bool step();
	double getQuality() { return bestQuality; }
	void cleanup();

protected:
	std::map<ActionNode*, std::vector<HOPAdvance*>*> allSamples;
	int numSamples;
	double bestQuality;
	int bestRand;
	bool done;

};

#endif  // HOP_H
