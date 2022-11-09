#include "sampled_search.h"

#include "NSPolicy.h"
#include "globals.h"            // verbosity
#include "graph.h"              // StateNode, StateQueue
#include <time.h>               // clock
#include <deque>
#include <set>
#include <iostream>
#include <iomanip>              // setiosflags, setprecision
#include <fstream>

using namespace std;

SampledSearch::SampledSearch(SearchType inheriting_type)
: AnytimeSearch(inheriting_type){
}

bool SampledSearch::solve(StateNode* start){
	return false;
/*
  StateNode* front = start;
  StateNode* next;

  while(front != NULL && front->Terminal <= 0 && front->Solved <= 0){
    front->BestAction = findBestAction(front);

    if(front->BestAction != NULL){
      next = NULL;

			double p = (double)rand() / (double)RAND_MAX;
			double cum_p = 0.0;

			int count = 0;
      for(StateDistribution* dist = front->BestAction->NextState; dist != NULL; dist = dist->Next)
				count++;

      for(StateDistribution* dist = front->BestAction->NextState; dist != NULL; dist = dist->Next){
//				cum_p += 1.0/(double)count;
				cum_p += dist->Prob;
//        if(next == NULL || next->h > dist->State->h)
//        if(next == NULL || (next->prReached * (pow(0.5, next->h))) < (dist->State->prReached * (pow(0.5, dist->State->h))))
//        if(next == NULL || next->prReached < dist->State->prReached)
				if(cum_p > p){
          next = dist->State;
          break;
        }
      }

//			front->Solved = 1;
      partialSoln.push_back(front);
      for(StateDistribution* dist = front->BestAction->NextState; dist != NULL; dist = dist->Next)
        if(dist->State != next)
          open.push(dist->State);

			valueIteration(partialSoln, open.getContainer());
			open.sort();
			writePS();

      front = next;
    }
    else
      front = NULL;
  }

  if(verbosity >= 1 && front != NULL)
    cout << "\t" << front->StateNo << "\t" << front->g << "\t" << front->h << "\t"
         << front->goalSatisfaction << "\t" << front->prReached << endl;

  return (front != NULL);

*/}

ActionNode* SampledSearch::findBestAction(StateNode* state){
  if(state->Expanded <= 0){
    if(verbosity >= 2)
      cout << StateNode::expandedStates << ":\t" << state->StateNo << "\t" << state->g << "\t" << state->h << "\t"
           << state->goalSatisfaction << "\t" << state->prReached << endl;

    state->expand();
    state->Expanded = 1;
    state->BestAction = NULL;
  }
  else if(state->Solved > 0)
		return state->BestAction;

	ActionNode* bestAction = NULL;
	double bestExpH, expH;

	for(ActionNodeList::iterator act_it = state->NextActions->begin(); act_it != state->NextActions->end(); act_it++){
		ActionNode* action = *act_it;
		expH = 0.0;

//		if(action == state->TerminalAction && state->goalSatisfaction < search_goal_threshold)
//			continue;

//		cout << "action: " << action->ActionNo << endl;
		for(StateDistribution* dist = action->NextState; dist != NULL; dist = dist->Next){
			dist->State->prReached = state->prReached * dist->Prob;
			if(dist->State->h <= 0.0 && dist->State->goalSatisfaction < search_goal_threshold)
				dist->State->h = IPP_MAX_PLAN;

//			cout << "prob: " << dist->Prob << "\th: " << dist->State->h << endl;
			expH += dist->Prob * dist->State->h;
		}

		if(bestAction == NULL || bestExpH > expH || (bestExpH == expH && rand() % 2 == 0)){
			bestAction = action;
			bestExpH = expH;
		}
	}
/*
	if(bestAction == NULL)
		cout << "** NO BEST ACTION\n";
	else
		cout << "** best action = " << bestAction->ActionNo << endl;
*/
	return bestAction;
}
