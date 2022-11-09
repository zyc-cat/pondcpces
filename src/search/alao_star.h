#ifndef ALAO_STAR_H
#define ALAO_STAR_H

#include "step_search.h"

#include <set>
#include <time.h>               // clock
#include <iostream>
#include <fstream>

// Forward declarations
class StateNode;

class ALAOStar : public StepSearch{
public:
	ALAOStar();
	~ALAOStar();

	void setup(StateNode* start);
	bool step();
	double getQuality();
	void cleanup();

protected:
	StateNode* next;

	void valueIteration();
	StateNode* getBestTip();
};

#endif	// ALAO_STAR_H
