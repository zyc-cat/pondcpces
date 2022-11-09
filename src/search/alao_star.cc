#include "alao_star.h"

#include "globals.h"				// verbosity
#include "graph.h"
#include <set>
#include <iostream>
#include <queue>
#include <deque>

using namespace std;

ALAOStar::ALAOStar()
: StepSearch(ALAO_STAR){
	std::cout << "new ALAOStart()\n";
}

ALAOStar::~ALAOStar(){
}

void ALAOStar::setup(StateNode* start){
	StepSearch::setup(start);
	next = start;
}
/**
 * 父类StepSearch调用step进行更新，直到收敛或者没有结点可以更新
 */
bool ALAOStar::step(){
	if(next == NULL || next->isGoal())
		return false;

	next->expand();// 拓展结点
	valueIteration();// value迭代
	next = getBestTip();// 获取新的结点准备更新

	return true;
}

double ALAOStar::getQuality(){
	std::cout << "ALAOStar::getQuality()\n";
	assert(0);
	return next->goalSatisfaction; // Assuming conformant search
	//	return next->prReached * next->expDiscRew;
	//	return start->expDiscRew;
	//	static double c = 0.0;
	//	return ++c;
}

void ALAOStar::cleanup(){
	StepSearch::cleanup();
	next = NULL;
}

void ALAOStar::valueIteration(){
	deque<StateNode*> states;

	set<StateNode*> closed;
	queue<StateNode*> open_vi;
	open_vi.push(start);

	StateNode* front = NULL;

	while(!open_vi.empty()){
		front = open_vi.front();
		open_vi.pop();
		closed.insert(front);
		states.push_back(front);

		if(front->Expanded > 0){
			for(ActionNodeList::iterator act_it = front->NextActions->begin(); act_it != front->NextActions->end(); act_it++){
				ActionNode* action = *act_it;
				for(StateDistribution* dist = action->NextState; dist != NULL; dist = dist->Next){
					StateNode* successor = dist->State;
					if(closed.count(successor) <= 0)
						open_vi.push(successor);
				}
			}
		}
	}

	double maxChange = 0;
	int iterations = 0;
	
	//do until we are not changing anything
	do{
		if(iterations > 10)
			break;
		iterations++;

		maxChange = 0;

		// BACKWARD
		for(deque<StateNode*>::reverse_iterator state_it = states.rbegin(); state_it != states.rend(); state_it++) {
			double change = (*state_it)->backwardUpdate(true);
			if(change > maxChange)
				maxChange = change;
		}

		// FORWARD
		for(deque<StateNode*>::iterator state_it = states.begin(); state_it != states.end(); state_it++) {
			double change = (*state_it)->forwardUpdate();
			if(change > maxChange)
				maxChange = change;
		}
	}while(maxChange > gEpsilon);
}

StateNode* ALAOStar::getBestTip(){
	set<StateNode*> closed;
	queue<StateNode*> open;
	open.push(start);

	StateNode* state = NULL;
	StateNode* best = NULL;

	while(!open.empty()){
		state = open.front();
		open.pop();
		closed.insert(state);

		if(state->Expanded > 0){
			assert(state->BestAction != NULL);
			for(StateDistribution* dist = state->BestAction->NextState; dist != NULL; dist = dist->Next){
				StateNode* next = dist->State;
				if(closed.count(next) <= 0)
					open.push(next);
			}
		}
		else{
//			if(best == NULL || best->f > state->f || (best->f == state->f && rand() % 2 == 0))
			if(best == NULL || best->prReached < state->prReached || (best->prReached == state->prReached && rand() % 2 == 0))
				best = state;
		}
	}

	return best;
}
