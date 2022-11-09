#include "online_search.h"

#include "../globals.h"				// server_socket, verbosity
#include "step_search.h"
#include "graph.h"						// StateNode, ActionNode

#include <limits>
#include <sstream>
#include <string>
#include <cassert>
#include <ext/hash_map>

using namespace std;
using namespace __gnu_cxx;

extern void printBDD(DdNode*);

const string SOFT_RESET_CMD =			"%%))SOFT-RESET((%%";

int g_hLimit = -1;

OnlineSearch::OnlineSearch(StepSearch* step_search, int hLimit)
: Search(ONLINE){
	this->step_search = step_search;
	if(hLimit < 0)
		this->hLimit = max_horizon;
	else
		this->hLimit = hLimit;
	g_hLimit = hLimit;
}

OnlineSearch::~OnlineSearch(){
	delete step_search;
}

string OnlineSearch::readString(int fd){
	char nextChar;
	string inMsg = "";
	while(read(fd, &nextChar, 1) == 1 && nextChar != '\0')
		inMsg += nextChar;
	return inMsg;
}

double OnlineSearch::readNum(int fd){
	double num;
	istringstream iss(readString(fd));
	if(!(iss >> num))
		num = 0.0;
	return num;
}

void OnlineSearch::sendAction(int fd, ActionNode* action){
	assert(action == NULL || action->PrevState != NULL);
	ostringstream os;
	if(action != NULL){
		if(action != action->PrevState->TerminalAction)
			os << action->act->name();
		else
			os << "noop";
	}
	os << '\0';
	write(fd, os.str().c_str(), os.str().length());
}

void OnlineSearch::search(){
	assert(server_socket > -1);
	int fd = server_socket;

	StateNode* start = Start;
	int curHorizon = 0;
	/*if(curHorizon < max_horizon - hLimit)
		start->horizon = max_horizon - hLimit;
	else*/
		start->horizon = curHorizon;
	start->g = start->horizon;

	// Go to initial state
	bool isAction = true;
	stringstream commaSS(readString(fd));
	string commaItem;
	while(getline(commaSS, commaItem, ',')){
		if(isAction){
			string& action = commaItem;
			assert(start != NULL);
			start->BestAction = start->getAction(action);
			assert(start->BestAction != NULL);
			start->Solved =	1;
		}
		else{
			stringstream spaceSS(commaItem);
			string obs;
			set<string> trueObs;
			while(getline(spaceSS, obs, ' '))
				if(!(obs.empty()))
					trueObs.insert(obs);
			assert(start != NULL && start->BestAction != NULL);
			start = start->BestAction->getState(trueObs);
			printBDD(start->dd);
			curHorizon++;
			/*if(curHorizon < max_horizon - hLimit)
				start->horizon = max_horizon - hLimit;
			else*/
				start->horizon = curHorizon;
			start->g = start->horizon;
			assert(start != NULL);
		}
		isAction = !isAction;
	}

	cout << "... and, go!\n";

	ActionNode* bestAction = NULL;
	double curQuality, bestQuality;
	double time_remaining;
	string obsGrp;

	// While we're not done (time remains, something to do, etc)
	do{
		assert(start != NULL);
		if(start->Solved > 0 && (bestAction = start->BestAction) != NULL){
			// Send previously solved best action
			cout << "\t" << start->StateNo << "\t" << start->g << "\t" << start->h << "\t"
					 << start->goalSatisfaction << "\t" << start->prReached << endl;

			time_remaining = readNum(fd);
			sendAction(fd, bestAction);

			if(time_remaining > 0.0){					// Still have time?
				readNum(fd);										//   Ignore time
				sendAction(fd, NULL);						//   Say that we're done
			}
		}
		else{
			bestAction = NULL;
			bestQuality = -(numeric_limits<double>::max());
			time_remaining = readNum(fd);

			if(time_remaining > 0.0){
				int bestRand = rand();
				step_search->setup(start);
				// While we're not done (time remains, something to do, etc)
				while(time_remaining > 0.0){
					if(!step_search->step())				// Progress search
						break;

					// Find best action
					curQuality = step_search->getQuality();
					int curRand = rand();
					if(start->BestAction != NULL && (bestAction == NULL || bestQuality < curQuality || (bestQuality == curQuality && bestRand < curRand))){
						bestAction = start->BestAction;
						bestQuality = curQuality;
						bestRand = curRand;
					}
					sendAction(fd, bestAction);			// Return best action
					time_remaining = readNum(fd);
				}
				step_search->cleanup();
			}

			sendAction(fd, NULL);								// No more; all done!

			// Mark state as solved
			assert(bestAction != NULL);
			start->BestAction = bestAction;
			start->Solved = 1;
		}

		// Get observations
		obsGrp = readString(fd);
		if(obsGrp.compare(SOFT_RESET_CMD) == 0){
			start = Start;
			curHorizon = 0;
			/*if(curHorizon < max_horizon - hLimit)
				start->horizon = max_horizon - hLimit;
			else*/
				start->horizon = curHorizon;
			start->g = start->horizon;
/*
			// Delete all nodes, except Start and Goal
			set<DdNode*> cleanup;
			set<StateNode*> open;
			set<StateNode*> closed;
			open.insert(Start);
			while(!open.empty()){
				StateNode* state = *open.begin();
				open.erase(open.begin());
				closed.insert(state);
				
				for(ActionNodeList::iterator act_it = state->NextActions->begin(); act_it != state->NextActions->end(); act_it++){
					ActionNode* action = *act_it;
					for(StateDistribution* dist = action->NextState; dist != NULL; dist = dist->Next){
						StateNode* child = dist->State;
						if(closed.count(child) <= 0){
							open.insert(child);
						}
					}
					delete action;
				}

				state->Expanded = 0;
				if(state->NextActions != NULL)
					state->NextActions->clear();
				if(state->PrevActions != NULL)
					state->PrevActions->clear();

				cleanup.insert(state->dd);
				cleanup.insert(state->backup_dd);
				cleanup.insert(state->rp_states);
				cleanup.insert(state->usedSamples);

				if(state != Start && state != Goal)
					delete state;
			}

			for(hash_map<DdNode*, StateNode*, __gnu_cxx::hash<DdNode*>, eqstr>::iterator gen_it = StateNode::generated.begin(); gen_it != StateNode::generated.end(); gen_it++)
				cleanup.insert((*gen_it).first);						// Redundant with cleanup.insert(state->dd) above?

			for(set<DdNode*>::iterator clean_it = cleanup.begin(); clean_it != cleanup.end(); clean_it++){
				DdNode* dd = *clean_it;
				if(dd != NULL
				&& dd != Start->dd && (Goal == NULL || dd != Goal->dd)
				&& dd != Start->backup_dd && (Goal == NULL || dd != Goal->backup_dd)
				&& dd != Start->rp_states && (Goal == NULL || dd != Goal->rp_states)
				&& dd != Start->usedSamples && (Goal == NULL || dd != Goal->usedSamples))
					dd->ref = 1;			// Don't tell CUDD I'm doing this, tsk tsk :)  But I gotta make sure they disappear!
			}

			for(set<DdNode*>::iterator clean_it = cleanup.begin(); clean_it != cleanup.end(); clean_it++){
				DdNode* dd = *clean_it;
				if(dd != NULL
				&& dd != Start->dd && (Goal == NULL || dd != Goal->dd)
				&& dd != Start->backup_dd && (Goal == NULL || dd != Goal->backup_dd)
				&& dd != Start->rp_states && (Goal == NULL || dd != Goal->rp_states)
				&& dd != Start->usedSamples && (Goal == NULL || dd != Goal->usedSamples))
					Cudd_RecursiveDeref(manager, dd);
			}

			cuddGarbageCollect(manager, TRUE);

			StateNode::generated.clear();
			StateNode::generated[Start->dd] = Start;
			if(Goal != NULL && Goal->dd != NULL)
				StateNode::generated[Goal->dd] = Goal;

			StateNode::expandedStates = 0;
			ActionNode::actionCount = 0;
			state_count = 2;
*/
		}
		else if(!obsGrp.empty()){
			stringstream spaceSS(obsGrp);
			string obs;
			set<string> trueObs;
			//cout << "POND observations:\n";
			while(getline(spaceSS, obs, ' '))
				if(!(obs.empty())){
					//cout << obs << endl;
					trueObs.insert(obs);
				}
			assert(start != NULL && start->BestAction != NULL);
			start = start->BestAction->getState(trueObs);
			curHorizon++;
			/*if(curHorizon < max_horizon - hLimit)
				start->horizon = max_horizon - hLimit;
			else*/
				start->horizon = curHorizon;
			start->g = start->horizon;
			assert(start != NULL);
		}
	} while(!obsGrp.empty());
}
