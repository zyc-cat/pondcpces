#include "anytime_search.h"

#include "globals.h"            // verbosity
#include "graph.h"              // StateNode, ActionNode
#include <time.h>               // clock
#include <deque>
#include <set>
#include <queue>
#include <iostream>
#include <iomanip>              // setiosflags, setprecision
#include <fstream>
#include <sstream>

using namespace std;

AnytimeSearch::AnytimeSearch(SearchType inheriting_type)
: Search(inheriting_type), planSuccessCSV("plan_success.csv"), planSuccessTXT("plan_success.txt"){
  startTime = clock();
	planSuccessCSV << "Time,P(PlanSuccess),MeanFirstPassage,E[R]" << endl << setiosflags(ios::fixed) << setprecision(3);
	planSuccessTXT << "#Time\tP(PlanSuccess)\tMeanFirstPassage\tE[R]" << endl << setiosflags(ios::fixed) << setprecision(3);
}

AnytimeSearch::~AnytimeSearch(){
	planSuccessCSV.close();
	planSuccessTXT.close();
}

void AnytimeSearch::search(){
	set<StateNode*> open;
	set<StateNode*> closed;
	StateNode* state;
	StateNode* best;

	bool success;

	do{
		// Pick state
		best = NULL;
		open.clear();
		closed.clear();
		open.insert(Start);

		while(!open.empty()){
			state = *open.begin();
			open.erase(open.begin());
			closed.insert(state);

			if(state->Solved <= 0){
				if(best == NULL || best->prReached < state->prReached)
					best = state;
			}
			else if(state->BestAction != NULL){
				for(StateDistribution* dist = state->BestAction->NextState; dist != NULL; dist = dist->Next){
					StateNode* next = dist->State;
					if(closed.count(next) <= 0)
						open.insert(next);
				}
			}
		}

		if(best == NULL)
			break;

		success = solve(best);

		if(success == false){
			if(verbosity >= 1)
				cout << "Dead End!\n";

			// pi(b) <- terminate
			best->BestAction = best->TerminalAction;
			best->Solved = 1;

			assert(best->TerminalAction != NULL);

			assert(best != NULL
				&& best->BestAction != NULL
				&& best->BestAction->NextState != NULL
				&& best->BestAction->NextState->Next == NULL
				&& best->BestAction->NextState->State != NULL);

			best = best->BestAction->NextState->State;
			best->BestAction = NULL;
			best->Solved = 1;
		}
		else{
			if(verbosity >= 1)
				cout << "Found Branch!\n";
		}

		valueIteration();
		writePS();
	} while(Start->ExtendedGoalSatisfaction < plan_success_threshold);

//	Start->ExtendedGoalSatisfaction = 1.0;	// Hack
	Start->terminateLeaves();

	cout << "Search Time: " << (float)((clock() - startTime) / (float)CLOCKS_PER_SEC) << endl;
	cout << "Expanded States = " << StateNode::expandedStates << endl;
}

void AnytimeSearch::valueIteration(){
	queue<StateNode*> open;
	set<StateNode*> closed;
	StateNode* state = NULL;

	deque<StateNode*> states;

	open.push(Start);
	while(!open.empty()){
		state = open.front(); open.pop();
		closed.insert(state);

		states.push_back(state);

		if(state->Solved > 0 && state->BestAction != NULL){
			for(StateDistribution* dist = state->BestAction->NextState; dist != NULL; dist = dist->Next){
				StateNode* next = dist->State;
				if(closed.count(next) <= 0)
					open.push(next);
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
			double change = (*state_it)->backwardUpdate();
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

void AnytimeSearch::writePS(){
	// Output plan success to file
	planSuccessCSV << (float)((clock() - startTime) / (float)CLOCKS_PER_SEC) << ',' << Start->ExtendedGoalSatisfaction << ',' << Start->meanFirstPassage << ',' << Start->expDiscRew << endl;
	planSuccessTXT << (float)((clock() - startTime) / (float)CLOCKS_PER_SEC) << '\t' << Start->ExtendedGoalSatisfaction << '\t' << Start->meanFirstPassage << '\t' << Start->expDiscRew << endl;

	// Output plan success to screen
	if(verbosity >= 1)
		cout << "Time: " << (float)((clock() - startTime) / (float)CLOCKS_PER_SEC) << "\tSuccess: " << Start->ExtendedGoalSatisfaction << endl;
}
