#ifndef NSPOLICY_H
#define NSPOLICY_H

/**
*    NON-STATIONARY POLICY
**/

#include "globals.h"            // plan_success_threshold, search_goal_threshold, eqstr, hash<DdNode*>
#include <vector>
#include <list>
#include <ext/hash_map>

class PolicyState;
class PolicyAction;

class StateNode;
class ActionNode;

class PolicyNode{
public:
  PolicyNode(int samples);
  virtual ~PolicyNode() {};

  virtual void expand() = 0;

  int getSamples() { return samples; }
  double getF() { return f; }

  bool isDeadEnd() { return (f >= maxDepth); }

  static int maxDepth;

protected:
  unsigned int samples;
  double f;
};

class PolicyAction : public PolicyNode{
public:
  friend class PolicyState;

  PolicyAction(ActionNode* action, int samples, PolicyState* parent);
  ~PolicyAction();

  PolicyState* newChild(StateNode* state, int childSamples, double prob);

  void expand();
  void backward();
  void setBestAct(PolicyState* child);
  void freeze();

  ActionNode* getAction() { return action; }
  PolicyState* getParent() { return parent; }
  std::vector<PolicyState*> &getChildren() { return children; }

protected:
  ActionNode* action;
  PolicyState* parent;
  std::vector<PolicyState*> children;
};

class PolicyState : public PolicyNode{
public:
  friend class PolicyAction;

  PolicyState(StateNode* state, int samples, double prReached = 1.0, double g = 0.0, int horizon = 0, PolicyAction* parent = NULL);
  ~PolicyState();

  PolicyAction* newChild(ActionNode* action, int childSamples);

  void expand();
  void backward(PolicyAction* changed);
  void setBestAct(PolicyAction* child = NULL);
  void freeze();

  StateNode* getState() { return state; }
  PolicyAction* getParent() { return parent; }
  std::vector<PolicyAction*> &getChildren() { return children; }

  PolicyAction* getBestPolicyAction() { return bestAction; }
  ActionNode* getBestAction() { return ((bestAction == NULL) ? NULL : bestAction->getAction()); }

  double getG() { return g; }
  double getPrReached() { return prReached; }
  int getHorizon() { return horizon; }

  bool isFrozen() { return frozen; }
  bool isGoal() { return ((state->goalSatisfaction >= search_goal_threshold && state->Terminal > 0) || state->Solved > 0); }

protected:
  StateNode* state;
  PolicyAction* parent;
  std::vector<PolicyAction*> children;

  PolicyAction* bestAction;

  double g;
  double prReached;
  int horizon;
  bool frozen;
};

#endif  // NSPOLICY_H
