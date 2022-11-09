#ifndef EXPH_H
#define EXPH_H

#include "step_search.h"

class ExpH : public StepSearch{
public:
	ExpH();
	~ExpH();

	void setup(StateNode* start);
	bool step();
	double getQuality();
	void cleanup();

protected:
	StateNode* bestState;
};

#endif	// EXPH_H
