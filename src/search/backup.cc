/* backup.c */


#include <time.h>

#include "solve.h"
#include "graph.h"
#include "globals.h"
#include "backup.h"
#include "lao_wrapper.h"
#include "lug/ipp.h"
#include "lug/graph_wrapper.h"
#include <iostream>
#include <float.h>

extern int CHILDCOMBO;

clock_t myBackupTime = 0;

/* Returns change of state value */
double Backup(struct StateNode *state, int currHorizon)
{
  double                    fCost, gCost, hCost, maxfCost,
                            bestfCost, bestgCost, besthCost, oldfCost,
                            fWeightCost, bestfWeightCost, oldfWeightCost,
                            oldMeanFirstPassage, goalSat, bestGoalSat;
  ActionNodeList::iterator  act_it;
  struct ActionNode*        actionNode, *bestAction, *firstAction;
  struct StateDistribution* nextState;
  BitVector                 solved_visited;
  DdNode*                   tmpDD;
  double                    max_pr;
  double                    reward = 0.0;
  bool                      actSolved = false, gotSolved = false;

  if(state->Terminal == 1){
    state->h = 0.0;
    return 0.0;
  }

  if(state->Solved)
    return 0.0;

  bestAction = NULL;

  /* used for pathmax */
  oldfWeightCost = state->fWeight;
  oldfCost = state->f;

  /* Initialize to worst possible cost to guarantee improvement */
  bestfWeightCost = DBL_MAX;
  bestfCost = DBL_MAX;
  bestGoalSat = 0.0;

  if(state->BestAction == NULL || state->NextActions == NULL || state->NextActions->empty()){
      state->f = DBL_MAX;
      state->g = DBL_MAX;
      state->h = DBL_MAX;
      return DBL_MAX;
  }

  // if optimizing probability, find probability of goal satisfaction
  // obtained in this state, given max parent satisfaction
  if(!OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY){        // probability
    max_pr = 0.0;
    for(act_it = state->PrevActions->begin(); act_it != state->PrevActions->end(); act_it++){
      ActionNode* action = *act_it;
      if(action->PrevState->goalSatisfaction > max_pr)
    	max_pr = action->PrevState->goalSatisfaction;      
    }
  }

  /* Find action that minimizes expected cost */
  for(act_it = state->NextActions->begin(); act_it != state->NextActions->end(); act_it++){
    if(gotSolved)
      break;

    actionNode = *act_it;

    if(OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY){       // cost
      gCost = actionNode->Cost;
      hCost = 0.0;
      maxfCost = -1.0;
    }
    else if(!OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY){ // probability
      gCost = -1*(state->goalSatisfaction-max_pr);
      hCost = 0.0;
      maxfCost = -2.0;
    }
    else{                                               // no reward
      gCost = actionNode->Cost;
      hCost = 0.0;
      maxfCost = -1.0;
   
    }

    goalSat = 0.0;

    actSolved = true;
    for(nextState = actionNode->NextState; (nextState && nextState->State); nextState = nextState->Next) {

      if(CHILDCOMBO == 1 && (nextState->State->h + nextState->State->g > maxfCost)){
	    gCost =  nextState->State->g;     
	    hCost = nextState->State->h;  
	    maxfCost = gCost + hCost;
      }
      if(CHILDCOMBO == 2){

	if(OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY){       // cost
	  gCost += gDiscount * 
	    (nextState->Prob * nextState->State->g);     	
	  hCost += gDiscount * 
	    (nextState->Prob * nextState->State->h);  
	}
	else if(!OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY){ // probability
	  gCost += nextState->Prob * nextState->State->g;     	
	  hCost += nextState->Prob * nextState->State->h;  	  
	}
	else{                                               // no reward
	  gCost += gDiscount * 
	    (nextState->Prob * nextState->State->g);     	
	  hCost += gDiscount * 
	    (nextState->Prob * nextState->State->h);  
	  
	}
	goalSat +=  nextState->Prob * nextState->State->ExtendedGoalSatisfaction;
      }
      if(!nextState->State->Solved)
	actSolved = false;
    }
    fCost = gCost + hCost;
    fWeightCost = gCost + (gWeight * hCost);

    if ((fWeightCost < bestfWeightCost)
       || !bestAction // need to pick a best action no matter what       
       || (actSolved)){      

      bestfWeightCost = fWeightCost;
      bestfCost = fCost;
      bestgCost = gCost;
      besthCost = hCost;      
      bestAction = actionNode;
      bestGoalSat = goalSat;
      
      
      if(actSolved){
	    gotSolved = true;
      }
    }
  }

  state->fWeight = bestfWeightCost;
  state->f = bestfCost;
  state->g = bestgCost;
  state->h = besthCost;
  state->BestAction = bestAction;
  state->ExtendedGoalSatisfaction = bestGoalSat;
  state->bestActF = bestAction->NextState->State->f;
  if(gotSolved)
    state->Solved = 1;

  /* Update estimate of mean first passage time */
  oldMeanFirstPassage = state->meanFirstPassage;
  if(state->Terminal != 1 && state->BestAction)     /* update every state except goal */
    for(nextState = state->BestAction->NextState; nextState != NULL; nextState = nextState->Next)
       state->meanFirstPassage += nextState->Prob * nextState->State->meanFirstPassage;

  /* Change will always be positive with pathmax, so no need to 
     return absolute value of change */

  return(bestfCost - oldfCost);
}
