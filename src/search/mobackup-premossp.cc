/* backup.c */


#include <time.h>

#include "solve.h"
#include "graph.h"
#include "globals.h"
#include "mobackup.h"
#include "lao_wrapper.h"
#include "lug/ipp.h"
#include "lug/graph_wrapper.h"
#include <iostream>
#include <float.h>
//using namespace std;
extern int CHILDCOMBO;



bool PASSKMAX = //
false;//true;
int passKPoints = 1;



extern clock_t myBackupTime;

double MOBackupAllPts(struct StateNode *state, int currHorizon, bool *removedOriginalPoint);
double MOBackupPassKMax(struct StateNode *state, int currHorizon, bool *removedOriginalPoint);

double MOBackup(struct StateNode *state, int currHorizon, bool *removedOriginalPoint){
  if(PASSKMAX){
    MOBackupPassKMax(state, currHorizon, removedOriginalPoint);
  }
  else{
    MOBackupAllPts(state, currHorizon, removedOriginalPoint);
  }
}

double MOBackupPassKMax(struct StateNode *state, int currHorizon, bool *removedOriginalPoint){
  //for each action find the highest k probability points
  //the best k points for an action are in the cross product of the
  // best k^{|obs|} points.  These are most easily found by iterating
  // through the highest probability points for each observation
  //take the best k points across all actions

  double                    fCost, gCost, hCost, maxfCost,
                            bestfCost, bestgCost, besthCost, oldfCost,
                            fWeightCost, bestfWeightCost, oldfWeightCost,
                            oldMeanFirstPassage, goalSat, bestGoalSat;
  ActionNodeList::iterator      act_it;
  struct ActionNode        *actionNode, *bestAction, *firstAction;
  struct StateDistribution *nextState;
  MOValueFunctionPoint *maxPrActNextStPt = NULL;
  MOValueFunctionPoint *maxPrActPt = NULL;
  MOValueFunctionPoint *maxPrPt = NULL;
  MOValueFunction *stateMOValueFunction = new MOValueFunction(); 

  for(int i = 0; i < 2; i++){
    if(i == 1)
      maxPrPt = new MOValueFunctionPoint(0.0, 0.0, 
					 0.0, 
					 state->goalSatisfaction,
					 state->goalSatisfaction,
					 0, 0.0,
					 NULL, 
					 true, 
					 state);
    else
      maxPrPt = NULL;

    for(act_it = state->NextActions->begin(); act_it != state->NextActions->end(); act_it++){
      actionNode = *act_it;

      maxPrActPt = new MOValueFunctionPoint(0, 0, 0, 0, 0.0, 1, 0.0, NULL, true, state);

      for(nextState = actionNode->NextState; (nextState && nextState->State); nextState = nextState->Next){
    	maxPrActNextStPt = nextState->State->moValueFunction->getMaxPrSolPt((i==1));

    	if(!maxPrActNextStPt)
	      break;

	    gCost = nextState->Prob  * (actionNode->Cost + 
				        (gDiscount * maxPrActNextStPt->g));     	
	    hCost = gDiscount * nextState->Prob * maxPrActNextStPt->h;  	  
	    goalSat =  nextState->Prob * maxPrActNextStPt->goalSatisfaction;
	    fWeightCost = gCost + (gWeight * hCost);

	    maxPrActPt->g += gCost;
	    maxPrActPt->h += hCost;
	    maxPrActPt->f += fWeightCost;
	    maxPrActPt->gridValue = 1.0;
	    maxPrActPt->goalSatisfaction += goalSat;
	    maxPrActPt->contingencies += maxPrActNextStPt->contingencies;
	    maxPrActPt->bestAction = actionNode;
	    maxPrActPt->solution = (maxPrActPt->solution && maxPrActNextStPt->solution);
	    maxPrActPt->bestPoints.push_back(new MOValueFunctionPoint(maxPrActNextStPt));
      }

      if(maxPrActPt->goalSatisfaction > 1.0)
	    maxPrActPt->goalSatisfaction = 1.0;

      if(!maxPrPt || maxPrPt->goalSatisfaction < maxPrActPt->goalSatisfaction
	  || (maxPrPt->goalSatisfaction == maxPrActPt->goalSatisfaction
	  && maxPrPt->f > maxPrActPt->f)){
    	if(maxPrPt)
	      delete maxPrPt;
    	maxPrPt = maxPrActPt;
      }
    }

    if(maxPrPt == NULL){
       maxPrPt = new MOValueFunctionPoint(DBL_MAX, DBL_MAX, 
					  DBL_MAX, 
					  1.0, 1.0,
					  1, 0.0,
					 NULL, 
					 false, 
					 state);
    }

    stateMOValueFunction->points.insert(maxPrPt);
    maxPrPt = NULL;
  }

   delete state->moValueFunction;
   state->moValueFunction = stateMOValueFunction;
}

double MOBackupAllPts(struct StateNode *state, int currHorizon, bool *removedOriginalPoint)
/* Returns change of state value */
{
  double                    fCost, gCost, hCost, maxfCost,
                            bestfCost, bestgCost, besthCost, oldfCost,
                            fWeightCost, bestfWeightCost, oldfWeightCost,
                            oldMeanFirstPassage, goalSat, bestGoalSat;
  int contingencies;
  ActionNodeList::iterator      act_it;
  struct ActionNode        *actionNode, *bestAction, *firstAction;
  struct StateDistribution *nextState;
  BitVector solved_visited;
  DdNode* tmpDD;
 double max_pr;
 double reward = 0.0;
 double meanFirstPassage = 0.0;
 bool actSolved = false, gotSolved = false;
 double residual = 0.0;

  if(state->Terminal == 1)
    return 0.0;

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

  MOValueFunction *stateMOValueFunction = new MOValueFunction(); 

  for(__gnu_cxx::MOValueFunctionHash::iterator i = state->moValueFunction->points.begin(); i != state->moValueFunction->points.end(); i++){
    if((*i)->solution)
      stateMOValueFunction->points.insert(*i);
  }

  /* Find action that minimizes expected cost */
  for(act_it = state->NextActions->begin(); act_it != state->NextActions->end(); act_it++){
    actionNode = *act_it;

    MOValueFunction actValFn, tmpValFn;
    double actGDefault = actionNode->Cost;

    for(nextState = actionNode->NextState; (nextState && nextState->State); nextState = nextState->Next){

       actGDefault += gDiscount * nextState->State->moValueFunction->gDefault * nextState->Prob;

	 // update each point so far
	 bool addAllPoints = (actValFn.points.size() == 0);
	 for(__gnu_cxx::MOValueFunctionHash::iterator p1 = actValFn.points.begin();
	     p1 != actValFn.points.end() || addAllPoints; p1++){

	   // update with successor points
	   for(__gnu_cxx::MOValueFunctionHash::iterator p = nextState->State->moValueFunction->points.begin();
	       p != nextState->State->moValueFunction->points.end(); p++){

	     if((*p)->dominated)
	       continue;
	     
	     if(!addAllPoints &&
		(!(((*p)->solution && (*p1)->solution)) &&
		 (*p)->gridValue != (*p1)->gridValue)){
	       //don't create new points for heuristic points 
	       //for the cross product of heuristic points, only
	       //heuristic points of the same probability
	       //**this prevents the exponential growth in the heurisic points
		 //	    std::cout << "skip" << std::endl;
		 continue;
	     }

	     gCost = nextState->Prob  * (actionNode->Cost + (gDiscount * (*p)->g));     	
	     hCost = gDiscount * nextState->Prob * (*p)->h;  	  
	     goalSat =  nextState->Prob * (*p)->goalSatisfaction;
	     fWeightCost = gCost + (gWeight * hCost);
	     contingencies = (*p)->contingencies;
	     meanFirstPassage = nextState->Prob*(1.0+(*p)->meanFirstPassage);
	     
	     MOValueFunctionPoint *pt;
	     if(addAllPoints){
	       pt = new MOValueFunctionPoint(gCost,
					     hCost, 
					     fWeightCost, 
					     (*p)->gridValue,
					     goalSat,
					     contingencies,
					     meanFirstPassage,
					     actionNode,
					     (*p)->solution,
					     state);
	       pt->bestPoints.push_back(*p);//new MOValueFunctionPoint(*p));
	       //(*p)->users.insert(pt);
	       tmpValFn.points.insert(pt);
	     }
	     else{
	       pt = new MOValueFunctionPoint(gCost + (*p1)->g,
					     hCost + (*p1)->h, 
					     fWeightCost + (*p1)->f, 
					     (*p1)->gridValue,
					     goalSat + (*p1)->goalSatisfaction,
					     contingencies + (*p1)->contingencies,
					     meanFirstPassage + (*p1)->meanFirstPassage,
					     actionNode,
					     ((*p)->solution && (*p1)->solution),
					     state);
	       pt->bestPoints.push_back(*p);//new MOValueFunctionPoint(*p));
	       //(*p)->users.insert(pt);
	       for(std::list<MOValueFunctionPoint*>::iterator pts = (*p1)->bestPoints.begin();
		   pts != (*p1)->bestPoints.end(); pts++)
		 pt->bestPoints.push_back(*pts);//new MOValueFunctionPoint(*pts));
	       
	       tmpValFn.points.insert(pt);
	     }
	   }
	   //	tmpValFn.print(std::cout);
	   if(addAllPoints)
	     break;
	 }
	
	 //actValFn.print(std::cout);
	 //      std::cout <<  "HO" <<std::endl;
      for(__gnu_cxx::MOValueFunctionHash::iterator p1 = actValFn.points.begin();
	  p1 != actValFn.points.end();  p1++){
 // 	for(std::list<MOValueFunctionPoint*>::iterator  i = (*p1)->bestPoints.begin();
//  	    i != (*p1)->bestPoints.end(); i++)
//  	  delete *i;

	delete *p1;	
      }
      //tmpValFn.print(std::cout);
      actValFn.points.clear();
      actValFn.points.insert(tmpValFn.points.begin(), tmpValFn.points.end());
      tmpValFn.points.clear();	
     }
     if(0 && state == Start){
       
       actValFn.print(std::cout);
       stateMOValueFunction->print(std::cout);       
     }
     // std::cout << "Finished Siblings" << std::endl;

     MOValueFunction dominatedPts;
     
     //find minimum action, each new point may possibly dominate several exiting points
    for(__gnu_cxx::MOValueFunctionHash::iterator p1 = actValFn.points.begin();
	p1 != actValFn.points.end();  p1++){

  //     if((*p1)->goalSatisfaction > 1){
// 	std::cout << "GS = " << (*p1)->goalSatisfaction << std::endl;
// 	  exit(0);
// 	}
      if(!(*p1)->solution &&
	 ((*p1)->goalSatisfaction != (*p1)->gridValue))
      	(*p1)->goalSatisfaction = (*p1)->gridValue;

      //insert each point into users of bestpoints
      for(std::list<MOValueFunctionPoint*>::iterator  i = (*p1)->bestPoints.begin();
	  i != (*p1)->bestPoints.end(); i++){
	  (*i)->users.insert(*p1);
      }


//       std::cout << "pt " <<  *p1 << " " 
// 		<< (*p1)->solution << " " 
// 		<< (*p1)->goalSatisfaction << " " 
// 		<< (*p1)->f << std::endl;
      bool inferior = false;
      for(__gnu_cxx::MOValueFunctionHash::iterator p = stateMOValueFunction->points.begin();
	  p != stateMOValueFunction->points.end();  p++){
	if((*p)->dominated)
	  continue;
// 	std::cout << "p " <<  *p << " " 
// 		  << (*p)->solution << " " 
// 		  << (*p)->goalSatisfaction << " " 
// 		  << (*p)->f << std::endl;
	MOValueFunctionPoint *d;
 	if((*p1)->contingencies > MAXCONTINGENCIES){//check hard constraints
 	  inferior = true;
 	  break;
 	}
 	else  if((d = state->moValueFunction->solutionNoLessThan((*p1)->goalSatisfaction)) &&
		 d->dominated &&
		 d->goalSatisfaction == (*p1)->goalSatisfaction &&
		 d->f == (*p1)->f){
 	  inferior = true;
 	  break;
 	}
 	else 	if(MOValueFunction::dominates(*p, *p1)
// 		(abs((*p1)->goalSatisfaction - (*p)->goalSatisfaction) < gEpsilon &&
// 		 (*p1)->f >= (*p)->f) ||
// 		((*p1)->goalSatisfaction <= (*p)->goalSatisfaction &&
// 		 (*p1)->f == (*p)->f) ||
// 		((*p1)->goalSatisfaction <= (*p)->goalSatisfaction &&
// 		 (*p1)->f >= (*p)->f)
		){
	  // std::cout << "old doms " << *p << " " <<  (*p)->solution << " " << (*p)->goalSatisfaction << std::endl;
	  inferior = true;
	  break;
	}
      else if(MOValueFunction::dominates(*p1, *p)// &&
	   //	   stateMOValueFunction->points.size() > 1
// 	   (abs((*p1)->goalSatisfaction - (*p)->goalSatisfaction) < gEpsilon &&
// 	    (*p1)->f < (*p)->f) ||
// 	   ((*p1)->goalSatisfaction > (*p)->goalSatisfaction &&
// 	    (*p1)->f == (*p)->f) ||
// 	   ((*p1)->goalSatisfaction > (*p)->goalSatisfaction &&
//  	    (*p1)->f < (*p)->f)
	   ){
	//std::cout << "new doms " << *p1 << (*p1)->solution << std::endl;
	     
	  dominatedPts.points.insert(*p);	  
	  //stateMOValueFunction->points.erase(*p);
	  //std::cout << "Dominated: "  << *p << (*p)->solution<< " " << (*p)->goalSatisfaction <<std::endl;

	  //break;
	}
      else if ((*p1)->goalSatisfaction == (*p)->goalSatisfaction &&
	       (*p1)->f == (*p)->f ){
	inferior = true;
	break;
      }
      }
      if(!inferior){
	//std::cout << "added Point " << *p1 << (*p1)->solution << std::endl;
	stateMOValueFunction->points.insert(*p1);
// 	for(std::list<MOValueFunctionPoint*>::iterator  i = (*p1)->bestPoints.begin();
// 	    i != (*p1)->bestPoints.end(); i++){
// 	  (*i)->users.insert(*p1);
// 	}
 


	__gnu_cxx::MOValueFunctionHash::iterator e;
	for(__gnu_cxx::MOValueFunctionHash::iterator p = dominatedPts.points.begin();
	    p != dominatedPts.points.end();  p++){
	  (*p)->dominated = true;
	  //std::cout << *p << " 1dominated because of " << *p1 << std::endl;
	  // std::cout << "set dompoint " << *p << std::endl;
	  //e = stateMOValueFunction->points.find(*p);
	  //stateMOValueFunction->points.erase(e);
	  
// 	  for(std::list<MOValueFunctionPoint*>::iterator  i = (*p)->bestPoints.begin();
// 	      i != (*p)->bestPoints.end(); i++)
// 	    delete *i;
	  
	  //delete *p;
	}

      dominatedPts.points.clear();

      }
      else{
	//std::cout << "Delete Point " << (*p1)->goalSatisfaction << std::endl;
	dominatedPts.points.clear();
	for(std::list<MOValueFunctionPoint*>::iterator  i = (*p1)->bestPoints.begin();
	    i != (*p1)->bestPoints.end(); i++){
	  // std::cout << "deleting parents of " << *i << std::endl;
	  for(std::set<MOValueFunctionPoint*>::iterator a = (*i)->users.begin();
	      a != (*i)->users.end(); a++){
	    if(*a == *p1){
// 	      std::cout << "erase " << *i << " back pointer to " 
// 			<< *p1 << std::endl;
	      (*i)->users.erase(a);
	      break;
	    }	  
	  }
	}
	delete *p1;
	//std::cout << "deleting " << *p1 << std::endl;
    }
      //      std::cout << "|dom| = " << dominatedPts.points.size() << std::endl;
      //      stateMOValueFunction->points.erase(dominatedPts.points.begin(), e);

    }
   
    
    actValFn.points.clear();
    
    if(actGDefault < stateMOValueFunction->gDefault || 
       stateMOValueFunction->actionDefault == NULL){
      stateMOValueFunction->gDefault = actGDefault;
      stateMOValueFunction->actionDefault = actionNode;
    }
    
    if( 0 && state == Start){
      std::cout << "Finished Action" << std::endl;
    stateMOValueFunction->print(std::cout);
    }



  }
 
  //remove all dominated points
  MOValueFunction toRemove;
  bool foundDom;
  do {
    foundDom = false;
    for(__gnu_cxx::MOValueFunctionHash::iterator p = stateMOValueFunction->points.begin();
	p != stateMOValueFunction->points.end();  p++){
      if((*p)->dominated || 
	 ((*p)->bestAction && (*p)->bestPoints.size() == 0)){	
	//std::cout << "dompoint " << *p << std::endl;
	for(std::set<MOValueFunctionPoint*>::iterator  i = (*p)->users.begin();
	    i != (*p)->users.end(); i++){
	  __gnu_cxx::MOValueFunctionHash::iterator e = stateMOValueFunction->points.find(*i);
	  if(e != stateMOValueFunction->points.end()  ){
	    // 	    //foundDom = true;    
 	  }
 	  else{
	    bool otherAnc = false;
	    for(__gnu_cxx::MOValueFunctionHash::iterator b = stateMOValueFunction->points.begin();
		b != stateMOValueFunction->points.end();  b++){
	      if(!(*b)->dominated && ancestorOf(*b, *i)){
		otherAnc = true;
		break;
	      }
	    }
	    //	  if(!ancestorOf(*i, *p)
	    if(!otherAnc){
	      (*i)->dominated = true;
	      //std::cout << *i << " dominated because of " << *p << std::endl;
	    }
	  }
	}
	
	toRemove.points.insert(*p);	 
      }	

      
    }
  }while (foundDom);
  for(__gnu_cxx::MOValueFunctionHash::iterator p = toRemove.points.begin();
      p != toRemove.points.end();  p++){
    stateMOValueFunction->points.erase(*p);

    // std::cout << "deleting pointers of " << *p << std::endl;
    //make sure children back-pointers are set to NULL
    for(std::list<MOValueFunctionPoint*>::iterator  i = (*p)->bestPoints.begin();
 	i != (*p)->bestPoints.end(); i++){
 //      if(!(*i)) //child is gone
//  	continue;
      //std::cout << "deleting parents of " << *i << std::endl;
      for(std::set<MOValueFunctionPoint*>::iterator a = (*i)->users.begin();
	  a != (*i)->users.end(); a++){
	if(*a == *p){
	  //std::cout << "erase " << *i << " back pointer to " << *p << std::endl;
	  (*i)->users.erase(a);
	  break;
	}	  
      }
    }

    //make sure parent pointers are set to NULL
    for(std::set<MOValueFunctionPoint*>::iterator  i = (*p)->users.begin();
	i != (*p)->users.end(); i++){
      //std::cout << "deleting children of " << *i << std::endl;
      //go through parent's child pointers
      // std::cout << "size = " << (*i)->bestPoints.size() << std::endl;

      for(std::list<MOValueFunctionPoint*>::iterator j = (*i)->bestPoints.begin();
	  j != (*i)->bestPoints.end(); j++){
	//std::cout << *j << std::endl;
	if(*j == *p){

	  //std::cout << "erase " << *i << " pointer to " << *j << "size = " << (*i)->bestPoints.size() << std::endl;
	  //  std::cout << "Mark Dominated " << *i << std::endl;
	  (*i)->bestPoints.erase(j);
	  (*i)->dominated = true;
	  break;
	}
      }
    }
  }

 for(__gnu_cxx::MOValueFunctionHash::iterator p = toRemove.points.begin();
      p != toRemove.points.end();  p++){
   // std::cout << "deleting " << *p << std::endl;
   if(state->moValueFunction->points.find(*p) != state->moValueFunction->points.end() &&
      (*p)->solution)
     *removedOriginalPoint = true;
    delete *p;  
 }


 //if there is no applicable actions, add a heuristic point that is infinity
 if(state->NextActions == NULL){
   double hval;
   
//    if(state->goalSatisfaction == 1.0)
//       hval = 0.0;
//     else if(state->goalSatisfaction == 0.0)
//     hval = DBL_MAX;
//     else
//    state->h+=1;
//         hval = state->h;
   hval= DBL_MAX;
   stateMOValueFunction->points.insert(new MOValueFunctionPoint(new MOValueFunctionPoint(0.0,
											 hval, hval,
											 1.0, 1.0,
											 1, 0.0,
											 NULL, false, state)));
 }




 /***********Compute Residual *****************/
 // Find maximum minimum difference in one dimension
 for(__gnu_cxx::MOValueFunctionHash::iterator p = stateMOValueFunction->points.begin();
     p != stateMOValueFunction->points.end();  p++){

   if(!(*p)->solution)
     continue;


   double minResidual = DBL_MAX;
   //find min distant point in old val fn.
   for(__gnu_cxx::MOValueFunctionHash::iterator q = state->moValueFunction->points.begin();
       q != state->moValueFunction->points.end();  q++){

   if(!(*q)->solution)
     continue;

     double prResidual = (*p)->goalSatisfaction - (*q)->goalSatisfaction;
     double fResidual = (*p)->f - (*q)->f;

     double myResidual = (prResidual > fResidual ? prResidual : fResidual);

     if(myResidual < minResidual)
       minResidual = myResidual;
   }

   if(minResidual > residual)
     residual = minResidual;

 }
 /***********Compute Residual END *****************/

 std::cout << "Exit Backup " << state->StateNo << " Residual = " << residual << std::endl;
  delete state->moValueFunction;
  state->moValueFunction = stateMOValueFunction;
    state->moValueFunction->print(std::cout);

  return residual;
}
