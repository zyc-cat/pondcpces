#include "moastar.h"

#include "globals.h"
//#include "lao_wrapper.h"        // expandState
#include "graph.h"              // StateNode::expand
#include "statistical.h"
#include "lug.h"
#include <algorithm>
#include "mobackup.h"
#include "moastar_heuristics.h"

#include <iostream>
#include <fstream>

using namespace __gnu_cxx;
using namespace std;

extern double gWeight;
extern clock_t gStartTime;

extern bool mvfpCmp(MOValueFunctionPoint* s1, MOValueFunctionPoint* s2);

MOAStar::MOAStar()
: Search(MOASTAR){
}

void MOAStar::addNonDominatedPointsMOAstar(MOValueFunction* newPts, MOValueFunction* existingPts, struct StateNode* newPtsState){
  MOValueFunction dominatedPts;

  // find minimum action, each new point may possibly dominate several exiting points
  for(MOValueFunctionHash::iterator p1 = newPts->points.begin(); p1 != newPts->points.end();  p1++){
    // insert each point into users of bestpoints
    for(list<MOValueFunctionPoint*>::iterator  i = (*p1)->bestPoints.begin(); i != (*p1)->bestPoints.end(); i++)
      (*i)->users.insert(*p1);

    bool inferior = checkDominance(*p1, existingPts, newPtsState,  &dominatedPts);

    if(!inferior){
      existingPts->points.insert(*p1);

      MOValueFunctionHash::iterator e;
      for(MOValueFunctionHash::iterator p = dominatedPts.points.begin(); p != dominatedPts.points.end();  p++)
        (*p)->dominated = true;

      dominatedPts.points.clear();
    }
    else
      dominatedPts.points.clear();
  }
}

bool MOAStar::updateStateValue(ActionNode* actionNode, StateNode* successor, StateNode* n, MOValueFunction* NDpoints){
  MOValueFunction newPoints;

  for(MOValueFunctionHash::iterator i = n->moValueFunction->points.begin(); i != n->moValueFunction->points.end(); i++){
    if((*i)->solution)
      continue;

    /////////// Add heuristic Point //////////////
    list<MOValueFunctionPoint*> pts;
    computeMAOStarHeuristics(successor, actionNode, *i, &pts, NDpoints);
    newPoints.points.insert(pts.begin(), pts.end());

    if(successor->goalSatisfaction > n->goalSatisfaction){
      // if parent has better or samesatisfaction, then this child won't dominate
      ///////////Add Terminating Point//////////////////
      MOValueFunctionPoint *pt = new MOValueFunctionPoint(
          (actionNode->Cost + (*i)->g),
          0,
          actionNode->Cost + (*i)->g,
          1,
          successor->goalSatisfaction,
          1,
          1,
          actionNode,
          1,
          successor);

      pt->bestPoints.push_back(*i);
      newPoints.points.insert(pt);
    }
  }

  addNonDominatedPointsMOAstar(&newPoints, successor->moValueFunction, successor);
  removeDominatedPoints(successor, successor->moValueFunction);

  for(MOValueFunctionHash::iterator i = newPoints.points.begin(); i != newPoints.points.end(); i++){
    MOValueFunctionHash::iterator e = successor->moValueFunction->points.find(*i);
    if(e != successor->moValueFunction->points.end())
      return true;
  }

  return false;
}

void MOAStar::findNDKungRecurse(list<MOValueFunctionPoint*>* population){
  int size = population->size();
  if(size == 1)
    return;

  int middle = size / 2;
  list<MOValueFunctionPoint*>::iterator middlePtr = population->begin();
  advance(middlePtr, middle);

  list<MOValueFunctionPoint*>::iterator middleNextPtr = middlePtr;
  middleNextPtr++;

  list<MOValueFunctionPoint*> T(population->begin(), middlePtr);
  findNDKungRecurse(&T);

  MOValueFunction TPts;
  TPts.points.insert(T.begin(), T.end());

  list<MOValueFunctionPoint*> B (middlePtr, population->end());
  findNDKungRecurse(&B);

  for(list<MOValueFunctionPoint*>::iterator j = B.begin(); j != B.end(); j++){
    MOValueFunction dominatedPts;   // not used yet
    bool inferior = checkDominance(*j, &TPts, (*j)->stateNode,  &dominatedPts);
    if(!inferior){
      TPts.points.insert(*j);
      for(MOValueFunctionHash::iterator p = dominatedPts.points.begin(); p != dominatedPts.points.end();  p++)
        TPts.points.erase(*p);
    }
    dominatedPts.points.clear();
  }

  population->clear();
  population->insert(population->begin(), TPts.points.begin(), TPts.points.end());
}

bool MOAStar::findNDKung(set<StateNode*>* NDstates, MOValueFunction* NDpoints, set<StateNode*>* Open, MOValueFunction* NDsolutions){
  // remove from NDpoints the points that are not in Open anymore
  NDpoints->points.clear();

  list<MOValueFunctionPoint*> population;
  population.sort(mvfpCmp);

  // add NDsolutions to NDpoints
  NDpoints->points.insert(NDsolutions->points.begin(), NDsolutions->points.end());

  // add Open list points to NDpoints
  for(set<StateNode*>::iterator i = Open->begin(); i != Open->end(); i++)
    NDpoints->points.insert((*i)->moValueFunction->points.begin(), (*i)->moValueFunction->points.end());

  list<MOValueFunctionPoint*> pts (NDpoints->points.begin(), NDpoints->points.end());
  findNDKungRecurse(&pts);

  NDpoints->points.clear();
  NDpoints->points.insert(pts.begin(), pts.end());

  NDpoints->print(cout);

  // Add states to NDstates for points in NDpoints
  NDstates->clear();
  for(MOValueFunctionHash::iterator i = NDpoints->points.begin(); i != NDpoints->points.end(); i++)
    if(NDsolutions->points.find(*i) == NDsolutions->points.end())
      NDstates->insert((*i)->stateNode);

  return NDstates->size() > 0;
}

bool MOAStar::findND(set<StateNode*>* NDstates, MOValueFunction* NDpoints, set<StateNode*>* Open, MOValueFunction* NDsolutions){
  // remove from NDpoints the points that are not in Open anymore
  NDpoints->points.clear();

  // add NDsolutions to NDpoints
  NDpoints->points.insert(NDsolutions->points.begin(), NDsolutions->points.end());

  // add Open list points to NDpoints
  for(set<StateNode*>::iterator i = Open->begin(); i != Open->end(); i++)
    addNonDominatedPointsMOAstar((*i)->moValueFunction,   NDpoints,  *i);

  // prune dominated points from NDpoints
  MOValueFunction toRemove;
  for(MOValueFunctionHash::iterator p = NDpoints->points.begin(); p != NDpoints->points.end();  p++)
    if((*p)->dominated)
      toRemove.points.insert(*p);

  for(MOValueFunctionHash::iterator p = toRemove.points.begin(); p != toRemove.points.end();  p++)
    NDpoints->points.erase(*p);

  NDpoints->print(cout);

  // Add states to NDstates for points in NDpoints
  NDstates->clear();
  for(MOValueFunctionHash::iterator i = NDpoints->points.begin(); i != NDpoints->points.end(); i++)
    if(NDsolutions->points.find(*i) == NDsolutions->points.end())
      NDstates->insert((*i)->stateNode);

  return NDstates->size() > 0;
}

void MOAStar::selectFirstNodeToExpand(set<StateNode*>* NDstates, list<StateNode*>* nodesToExpand){
  nodesToExpand->push_back(*(NDstates->begin()));
}

void MOAStar::selectRandomNodeToExpand(set<StateNode*> *NDstates, list<StateNode*> *nodesToExpand){
  int rand = (int)(((double)randomGen->myrand() / ((double)(RAND_MAX) + 1.0)) * NDstates->size());
  int count = 0;

  for(set<StateNode*>::iterator i = NDstates->begin(); i != NDstates->end(); i++, count++){
    if(count == rand){
      nodesToExpand->push_back(*i);
      return;
    }
  }
}

void MOAStar::selectMostNDNodeToExpand(set<StateNode*>* NDstates, MOValueFunction* NDpoints, list<StateNode*>* nodesToExpand){
  int maxPoints = 0;
  StateNode* maxNode;

  for(set<StateNode*>::iterator i = NDstates->begin(); i != NDstates->end(); i++){
    int numPoints = 0;

    for(MOValueFunctionHash::iterator p = (*i)->moValueFunction->points.begin(); p != (*i)->moValueFunction->points.end();  p++){
      MOValueFunctionHash::iterator e = NDpoints->points.find(*p);
      if(e != NDpoints->points.end())
        numPoints++;
    }

    if(numPoints > maxPoints){
      maxPoints = numPoints;
      maxNode = *i;
    }
  }

  nodesToExpand->push_back(maxNode);
}

void MOAStar::selectLargestHvalue(set<StateNode*>* NDstates, MOValueFunction* NDpoints, list<StateNode*>* nodesToExpand){
  int rand = (int)(((double)randomGen->myrand() / ((double)(RAND_MAX) + 1.0)) * NDstates->size());

  if(rand > .9)
    selectRandomNodeToExpand(NDstates, nodesToExpand);
  else{
    double maxH = -1;
    StateNode *maxNode = NULL;

    for(MOValueFunctionHash::iterator p = NDpoints->points.begin(); p != NDpoints->points.end();  p++){
      if((*p)->h > maxH){
        maxH = (*p)->h;
        maxNode = (*p)->stateNode;
      }
    }

    nodesToExpand->push_back(maxNode);
  }
}

void MOAStar::selectNodesToExpand(set<StateNode*>* NDstates, MOValueFunction* NDpoints, list<StateNode*>* nodesToExpand){
  int h = 1;

  switch(h){
    case 0:
      selectFirstNodeToExpand(NDstates, nodesToExpand);
      break;
    case 1:
      selectRandomNodeToExpand(NDstates, nodesToExpand);
      break;
    case 2:
      selectMostNDNodeToExpand(NDstates, NDpoints, nodesToExpand);
      break;
    case 3:
      nodesToExpand->insert(nodesToExpand->begin(), NDstates->begin(), NDstates->end());
      break;
    case 4:
      selectLargestHvalue(NDstates, NDpoints, nodesToExpand);
      break;
  }
}

void MOAStar::search(){
  set<StateNode*> Open;
  set<StateNode*> NDStates;
  MOValueFunction NDpoints;
  MOValueFunction NDsolutions;
  StateHash Closed;
  static int ExpandedStates;
  int Iteration = 0;
  list<StateNode*> nodesToExpand;

  ofstream performanceFile;
  performanceFile.open ("perf.out");

  /////////Step 0/////////
  Start->ExtendedGoalSatisfaction = 1.0;    // hack
  Open.insert(Start);

  /////////Step 1/////////
  while(findNDKung(&NDStates, &NDpoints, &Open, &NDsolutions)){
    double time = ((float)(clock() - gStartTime) / CLOCKS_PER_SEC);

    // print hypervolume
    performanceFile << endl
                    << time << " "
                    << computeHyperVolume(&NDsolutions, false)
                    << " "
                    << computeHyperVolume(&NDpoints, true)
                    << endl;

    Iteration++;

    nodesToExpand.clear();
    selectNodesToExpand(&NDStates, &NDpoints, &nodesToExpand);

    // Step 2.2
    for(list<StateNode*>::iterator i = nodesToExpand.begin(); i != nodesToExpand.end(); i++){
      StateNode* n =*i;

      NDStates.erase(n);
      Open.erase(n);
      Closed.add(n);

      // Step 4
      MOValueFunction nSolPts;
      n->moValueFunction->getSolutionPts(&nSolPts);
      if(nSolPts.points.size() > 0){
        addNonDominatedPointsMOAstar(&nSolPts,   &NDsolutions,  n);
        MOValueFunction toRemove;

        for(MOValueFunctionHash::iterator p = NDsolutions.points.begin(); p != NDsolutions.points.end();  p++)
          if((*p)->dominated)
            toRemove.points.insert(*p);

        for(MOValueFunctionHash::iterator p = toRemove.points.begin(); p != toRemove.points.end();  p++)
          NDsolutions.points.erase(*p);
      }

      nSolPts.points.clear();

        // Step 5
      if(n->BestAction == NULL || n->Expanded == -1){
        n->expand();

        ExpandedStates++;

        for(ActionNodeList::iterator act_it = n->NextActions->begin(); act_it != n->NextActions->end(); act_it++){
          ActionNode* action = *act_it;
          for(StateDistribution* successor = action->NextState; successor != NULL; successor = successor->Next){
            if(successor->State->Expanded < Iteration){   // Step 5.3.1
              Open.insert(successor->State);
              updateStateValue(action, successor->State, n, &NDpoints);
            }
            else{                                         // Step 5.3.2
              cout << "ReGenerated State: " << successor->State->StateNo << endl;
              pair<StateHash::iterator, StateHash::iterator> q = Closed.equal_range(successor->State->dd);
              if(q.first != Closed.end() && q.second != Closed.end()){
                bool newValue = updateStateValue(action, successor->State, n, &NDpoints);
                if(newValue){
                  Closed.remove(successor->State);
                  Open.insert(successor->State);
                }
              }
            }

            successor->State->Expanded = Iteration;
          }
        }
      }
    }
  }

  // Step 2.1
  // TODO: computeSoltions();

  performanceFile.close();

  cout << "Expanded States = " << ExpandedStates << endl;
}
