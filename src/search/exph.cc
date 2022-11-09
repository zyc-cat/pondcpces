#include "exph.h"

#include "globals.h"					// verbosity, gEpsilon
#include "graph.h"						// StateNode, ActionNode
#include "../ppddl/actions.h"

#include <limits>
#include <queue>
#include <deque>
#include <set>
#include <cassert>

using namespace std;

ExpH::ExpH()
: StepSearch(EXPH){
}

ExpH::~ExpH(){
}

void ExpH::setup(StateNode* start){
	StepSearch::setup(start);
	bestState = NULL;
}

bool ExpH::step(){
	assert(start != NULL);

	if(bestState != NULL)
		return false;

	if(start->Solved > 0)				// Shouldn't happen
		return false;

	start->addAllActions();
	start->BestAction = NULL;

	for(ActionNodeList::iterator act_it = start->NextActions->begin(); act_it != start->NextActions->end(); act_it++){
		ActionNode* action = *act_it;
		StateNode* child = action->newSample();
/*	// Poor performance?  (No lookahead on goalSatisfaction)
		if(start->BestAction == NULL || bestState->goalSatisfaction < child->goalSatisfaction
		|| (bestState->goalSatisfaction == child->goalSatisfaction && bestState->h > child->h)
		|| (bestState->goalSatisfaction == child->goalSatisfaction && bestState->h == child->h && bestState->randPriority < child->randPriority)){
*/
///*
		if(start->BestAction == NULL || bestState->h > child->h
		|| (bestState->h == child->h && bestState->goalSatisfaction < child->goalSatisfaction)
		|| (bestState->h == child->h && bestState->goalSatisfaction == child->goalSatisfaction && bestState->randPriority < child->randPriority)){
//*/
			start->BestAction = action;
			bestState = child;
		}
	}

	return true;
}

double ExpH::getQuality(){
	assert(bestState != NULL);
	return bestState->goalSatisfaction;		// Assuming conformant search
}

void ExpH::cleanup(){
	if(start->Solved <= 0)
		start->BestAction = NULL;
	bestState = NULL;
}
