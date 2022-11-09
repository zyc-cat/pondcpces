#include "NSPolicy.h"

#include "graph.h"          // StateNode, ActionNode
#include <limits.h>         // INT_MAX
#include <vector>
#include <list>

using namespace __gnu_cxx;
using namespace std;

/**
*    STATIC VARIABLES
**/
int PolicyNode::maxDepth = INT_MAX;


/**
*    CONSTRUCTOR
**/
PolicyNode::PolicyNode(int samples){
  this->samples = samples;
  f = maxDepth;
}

PolicyState::PolicyState(StateNode* state, int samples, double prReached, double g, int horizon, PolicyAction* parent)
: PolicyNode(samples){
  this->state = state;
  this->parent = parent;

  bestAction = NULL;

  if(isGoal())
    f = 0.0;
  else
    f = state->h;

  this->g = g;
  this->prReached = prReached;
  this->horizon = horizon;
  frozen = false;
}

PolicyAction::PolicyAction(ActionNode* action, int samples, PolicyState* parent)
: PolicyNode(samples){
  this->action = action;
  this->parent = parent;
}


/**
*    DESTRUCTOR
**/
PolicyState::~PolicyState(){
  for(vector<PolicyAction*>::iterator child_it = children.begin(); child_it != children.end(); child_it++)
    delete *child_it;
}

PolicyAction::~PolicyAction(){
  for(vector<PolicyState*>::iterator child_it = children.begin(); child_it != children.end(); child_it++)
    delete *child_it;
}


/**
*    NEW CHILD
**/
PolicyAction* PolicyState::newChild(ActionNode* action, int childSamples){
  PolicyAction* child = new PolicyAction(action, childSamples, this);
  children.push_back(child);
  child->expand();
  return child;
}

PolicyState* PolicyAction::newChild(StateNode* state, int childSamples, double prob){
  PolicyState* child = new PolicyState(state, childSamples, parent->getPrReached() * prob, parent->getG() + action->Cost, parent->getHorizon() + 1, this);
  children.push_back(child);
  return child;
}


/**
*    EXPAND
**/
void PolicyState::expand(){
  if(frozen)
    return;

  double oldF = f;

  bool deadEnd = false;

  if(horizon >= maxDepth)               // Exceeded maximum depth threshold?
    deadEnd = true;
  else if(state->Terminal <= 0){        // Within depth threshold, and non-terminal?
    if(state->Expanded <= 0){
      if(verbosity >= 2)
        cout << StateNode::expandedStates << ":\t" << state->StateNo << "\t" << this->g << "\t" << state->h << "\t"
             << state->goalSatisfaction << "\t" << this->prReached << endl;

      state->expand();
      state->Expanded = 1;
      state->BestAction = NULL;
    }

    if(state->NextActions != NULL && state->NextActions->size() > 0){
      for(ActionNodeList::iterator act_it = state->NextActions->begin(); act_it != state->NextActions->end(); act_it++){
        PolicyAction* child = newChild(*act_it, samples);
        if(bestAction == NULL || f > child->getF()){
          bestAction = child;
          f = bestAction->getF();
        }
      }
    }
    else                                // Non-terminal, with no children?
      deadEnd = true;
  }
  else if(!isGoal())                    // Terminal, but not goal?
    deadEnd = true;

  if(deadEnd){
    f = maxDepth;
    bestAction = NULL;
  }

  if(parent != NULL && f != oldF)
    parent->backward();
}

void PolicyAction::expand(){
  unsigned int distIndex, distSize = 0;
  for(StateDistribution* dist = action->NextState; dist != NULL; dist = dist->Next)
    distSize++;

  if(distSize == 0 || samples == 0){    // Pathological cases
    f = maxDepth;
    return;
  }

  unsigned int distSamples[distSize];
  memset(distSamples, 0, distSize * sizeof(unsigned int));

  for(unsigned int i = 0; i < samples; i++){
    float p = (float)rand() / (float)RAND_MAX;
    float cum_p = 0.0f;

    distIndex = 0;
    for(StateDistribution* dist = action->NextState; dist != NULL; dist = dist->Next){
      cum_p += (float)dist->Prob;
      if(cum_p >= p){
        distSamples[distIndex]++;
        break;
      }
      distIndex++;
    }
  }

  f = 0.0;
  distIndex = 0;
  for(StateDistribution* dist = action->NextState; dist != NULL; dist = dist->Next){
    if(distSamples[distIndex] > 0){
      StateNode* state = dist->State;
      PolicyState* child = newChild(state, distSamples[distIndex], dist->Prob);
      f += child->getF() * (double)distSamples[distIndex];
    }
    distIndex++;
  }
  f /= (double)samples;
  f += action->Cost;
}


/**
*    BACKWARD INDUCTION
**/
void PolicyState::backward(PolicyAction* changed){
  if(frozen)
    return;

  double oldF = f;

  if(bestAction != changed && f > changed->getF())
    bestAction = changed;
  else if(bestAction == changed && f < changed->getF()){
    for(vector<PolicyAction*>::iterator child_it = children.begin(); child_it != children.end(); child_it++){
      PolicyAction* child = *child_it;
      if(bestAction->getF() > child->getF())
        bestAction = child;
    }
  }

  f = bestAction->getF();

  if(parent != NULL && f != oldF)
    parent->backward();
}

void PolicyAction::backward(){
  double oldF = f;

  f = 0.0;
  for(vector<PolicyState*>::iterator child_it = children.begin(); child_it != children.end(); child_it++){
    PolicyState* child = *child_it;
    f += child->getF() * (double)child->getSamples();
  }
  f /= (double)samples;
  f += action->Cost;

  if(parent != NULL && f != oldF)
    parent->backward(this);
}


/**
*    SET BEST ACTION
**/
void PolicyState::setBestAct(PolicyAction* child){
  bestAction = child;
  if(parent != NULL)
    parent->setBestAct(this);
}

void PolicyAction::setBestAct(PolicyState* child){
  if(parent != NULL)
    parent->setBestAct(this);
}


/**
*    FREEZE
**/
void PolicyState::freeze(){
  if(!frozen){
    frozen = true;
    if(parent != NULL)
      parent->freeze();
  }
}

void PolicyAction::freeze(){
  if(parent != NULL)
    parent->freeze();
}
