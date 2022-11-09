#include "step_search.h"

#include "graph.h"						// StateNode, ActionNode

#include <set>
#include <queue>
#include <cassert>

using namespace std;

StepSearch::StepSearch(SearchType inheriting_type)
: Search(inheriting_type){
	start = NULL;
}

StepSearch::~StepSearch(){
}

void StepSearch::search(){
	setup(Start);// 调用astar的初始化
	while(step());//启用step进行搜索
	cleanup();
}

void StepSearch::setup(StateNode* start){
	assert(start != NULL);
	this->start = start;
	bestFirstAction = NULL;
}

void StepSearch::store(){
	assert(start != NULL);
	bestFirstAction = start->BestAction;
}

void StepSearch::recall(){
	assert(start != NULL);
	start->BestAction = bestFirstAction;
}

void StepSearch::commit(){
	assert(start != NULL);
	start->Solved = 1;
}

void StepSearch::cleanup(){
	assert(start != NULL);

	queue<StateNode*> open;
	open.push(start);

	set<StateNode*> closed;
	closed.insert(start);

	StateNode* state;
	StateNode* child;
	while(!open.empty()){
		state = open.front(); open.pop();
		// 每个结点的best和solve制空。
		if(state->Solved <= 0 && state->BestAction != NULL){
			state->BestAction = NULL;
			state->Solved = 0;
		}

		if(state->Expanded > 0){
			for(ActionNodeList::iterator act_it = state->NextActions->begin(); act_it != state->NextActions->end(); act_it++){
				for(StateDistribution* dist = (*act_it)->NextState; dist != NULL; dist = dist->Next){
					child = dist->State;
					if(closed.count(child) <= 0){
						open.push(child);
						closed.insert(child);
					}
				}
			}
		}
	}

	start = NULL;
	bestFirstAction = NULL;
}
