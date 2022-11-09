#include "hop.h"

#include "globals.h"						// verbosity
#include "hop_advance.h"
#include "graph.h"              // StateNode
#include "../ppddl/actions.h"
#include <limits>
#include <queue>
#include <set>
#include <cassert>

using namespace std;

extern void printHOP(DdNode* dd);

HOP::HOP(int numSamples)
: StepSearch(HOP_SEARCH){
	this->numSamples = numSamples;
}

void HOP::setup(StateNode* start){
	StepSearch::setup(start);

	assert(start->Solved <= 0);
	assert(allSamples.empty());
	///*
//getchar();
	start->addAllActions();
	for(ActionNodeList::iterator act_it = start->NextActions->begin(); act_it != start->NextActions->end(); act_it++){
		ActionNode* action = *act_it;
		allSamples[action] = new vector<HOPAdvance*>();
		for(int i = 0; i < numSamples; i++)
			allSamples[action]->push_back(new HOPAdvance(action));
	}
	start->BestAction = NULL;
	//*/
	bestQuality = -(numeric_limits<double>::max());
	bestRand = rand();
	done = false;
}

bool dirUp = true;
char prevCmd[256];
bool HOP::step(){
	if(done)
		return false;
/*
	start->addAllActions();
	for(ActionNodeList::iterator act_it = start->NextActions->begin(); act_it != start->NextActions->end(); act_it++){
		ActionNode* action = *act_it;
		double avg = 0.0;
		for(int i = 0; i < 15; i++){
		StateNode* child = action->newSample();
		avg += child->h;
		}
		avg /= 15.0;
		cout << "ACTION: " << action->act->name() << "\t" << "(h = " << avg <<  ")" << endl;
	}
*/
/*
	printHOP(start->dd);
	cout << "h = " << start->h << endl;

	char line[256];
	cin.getline(line, 256);
	if(tolower(line[0]) == '\0')
		strcpy(line, prevCmd);
	else if(tolower(line[0]) == 'c')
		strcpy(line, "close_door__e0");
	else if(tolower(line[0]) == 'm')
		strcpy(line, "move_current_dir__e0");
	else if(tolower(line[0]) == 'n')
		strcpy(line, "noop");
	else if(tolower(line[0]) == 'd'){
		strcpy(line, "open_door_going_down__e0");
		dirUp = false;
	}
	else if(tolower(line[0]) == 'u'){
		strcpy(line, "open_door_going_up__e0");
		dirUp = true;
	}
	else if(tolower(line[0]) == 'o'){
		if(dirUp)
			strcpy(line, "open_door_going_up__e0");
		else
			strcpy(line, "open_door_going_down__e0");
	}
	strcpy(prevCmd, line);
	
	string manual_action = line;
	start->BestAction = start->getAction(manual_action);
	assert(start->BestAction != NULL);

	done = true;

	return true;

*/

	done = true;

	for(map<ActionNode*, vector<HOPAdvance*>*>::iterator s_it = allSamples.begin(); s_it != allSamples.end(); s_it++){
		ActionNode* action = (*s_it).first;
		vector<HOPAdvance*>* samples = (*s_it).second;

		double curQuality = 0.0;
		int curRand = rand();
		for(int i = 0; i < numSamples; i++){
			HOPAdvance* sample = (*samples)[i];
			if(sample->step()){
				curRand = sample->getRand();
				done = false;
			}
			curQuality += sample->getQuality();
		}
		curQuality /= (double)numSamples;

//		cout << "Act: " << action->act->name() << " " << curQuality << endl;

		if(start->BestAction == NULL || bestQuality < curQuality || (bestQuality == curQuality && bestRand < curRand)){
//		  cout << "BEST" <<endl;
			bestQuality = curQuality;
			bestRand = curRand;
			start->BestAction = action;
		}
	}

	return true;
}

void HOP::cleanup(){
	for(map<ActionNode*, vector<HOPAdvance*>*>::iterator s_it = allSamples.begin(); s_it != allSamples.end(); s_it++){
		vector<HOPAdvance*>* samples = (*s_it).second;
		for(int i = 0; i < numSamples; i++)
			delete (*samples)[i];
		delete samples;
	}
	allSamples.clear();
}
