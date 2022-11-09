#ifndef GRAPH_H
#define GRAPH_H

#define PPDDL_PARSER 1                // Copied from globals.h ... FIX ME!

#include <ext/hash_map>
#include <map>
#include <list>
#include <set>
#include <vector>

#include "./movalfn.h"
#include "effects.h"

/* forward declaration of structures */
class RelaxedPlanLite;
class RelaxedPlan;
class StateNode;
class ActionNode;
/* Conformant planning不使用*/
class PolicyState;
struct StateDistribution;
struct DdNode;
typedef unsigned int BitVector;

struct eqstr
{
  bool operator()(const DdNode* p, const DdNode* q) const
  {
    return  p == q;
  }
};

#ifndef PPDDL_PARSER
struct alt_action;
#endif

typedef std::list<StateNode*> StateList;
typedef std::list<ActionNode*> ActionNodeList;

struct goal;

const short UNEXPANDED = 20;
const short EXPANDED = 21;
const short GOAL_REACHABLE = 22;
const short ISOLATED = 23;
const short OUT_ISOLATED = 24;
const short DEAD = 25;
/* declaration of structures */
class StateNode {
public:
  StateNode();
  ~StateNode();

	// For sampling
	void addAllActions();
	ActionNode* getAction(const std::string name, bool addIfNeeded = true);

  void expand();
/**
 * momo007 2022.05.25 not use comment 
 */
/* void getAncestery(std::vector<StateNode*>& ancestery);
  static void getAncestery(std::vector<StateNode*>& states, std::vector<StateNode*>& ancestery);
*/

  double forwardUpdate();// 状态节点前向更新
  double backwardUpdate(bool setBestAct = false);//后向更新
/**
 * valueUpdate调用 forwardUpdate和 backwardUpdate进行更新，涉及涉及概率，忽略
 * momo007 2022.05.25 not use comment
 */
  // double valueUpdate(bool setBestAct = false);

  void valueIteration();
  static void valueIteration(std::vector<StateNode*>& states);

  static __gnu_cxx::hash_map<DdNode*, StateNode*, __gnu_cxx::hash<DdNode*>, eqstr> generated;

  void terminateLeaves();
  static void terminateLeaves(StateNode* start);

  static int                expandedStates;

  DdNode*                   dd;
  DdNode*                   backup_dd;
  DdNode*                   rp_states;
  int                       StateNo;
  char*                     Description;
  double                    f;
  double                    fWeight;
  double                    g;
  double                    h;
  double                    expH;
  double                    expDiscRew;
  double                    ExtendedGoalSatisfaction;
  double                    meanFirstPassage;   // upper bound on steps to goal
  double                    goalSatisfaction;
  double                    prReached;  // probability reached 到达当前节点的概率
  double                    bestActF;
  int                       num_supporters;
  int                       Terminal;
  int                       Expanded;
  int                       Update;
  int                       Backups;
  int                       Solved;             // 0 - not solved, 1 - fully solved, 2 - partially solved
  BitVector*                ha;
  int                       horizon;
  RelaxedPlanLite*          rpIncrement;
  int                       hIncrement;
  RelaxedPlan*              currentRP;
  DdNode*                   usedSamples;        // samples used from SAG by increments
  int                       numHvalues;
  double*                   hValues;
  int                       goalSamples;
  double                    kld;
  MOValueFunction*          moValueFunction;
  int												randPriority;

  ActionNode*               TerminalAction;
  ActionNode*               BestAction;
  ActionNodeList*           NextActions;
  ActionNodeList*           PrevActions;
  ActionNode*               BestPrevAction;
  short                     m_status;// momo007 stateNode status
  int                       m_active_in_transitions = 1;//momo007
  int                       m_depth;//momo007 

  int processSuccessors(std::list<StateNode*>* states, std::list<StateNode*>* fh_states);
  bool isGoal();

protected:
	// For sampling
	ActionNode* addAction(const std::string name, bool reuseIfExists = true, bool displayState = true);

  void applyDiscountedGoalProbability();
};

class ActionNode{
public:
  ActionNode();
  ~ActionNode();

	// For sampling
	StateNode* getState(const std::set<std::string>& trueObs, bool addIfNeeded = true);
	StateNode* newSample();

  static int                actionCount;

#ifdef PPDDL_PARSER
  const struct Action*      act;
#else
  struct alt_action*        act;
#endif 
  int                       ActionNo;
  double                    Cost;               // Immediate cost
  double                    expDiscRew;
  int                       Solved;             // 0 - not solved, 1 - fully solved, 2 - partially solved
  int                       penalty;
  struct StateDistribution* NextState;
  struct StateNode*         PrevState;
  struct StateDistribution* Determinization;    // The one state that this action is determinized for (ignore StateDistribution::Next)

protected:
	// For sampling
	StateNode* addState(const std::set<std::string>& trueObs, bool reuseIfExists = true);
	StateNode* newSample(std::list<std::map<const pEffect*, DdNode*>*>* observations);
};
/**
 * 状态分布，包括状态、reason、creason、概率、reward、下一个状态分布
 */
class StateDistribution{
public:
  struct StateNode*         State;// 状态结点
  DdNode*                   reason;// effect BDD?
  std::set<const pEffect*>*         creason;// effect公式
  double                    Prob;// 概率
  double                    Reward;// 奖赏
  struct StateDistribution* Next;// 下一个分布
};

class StateComparator{
public:
enum CompareMode{
  F_VAL,                  // F value
  PR_REACH,               // Probability of being reached
  EXP_EGS,                // Expected extended goal satisfaction
  HEUR                    // Heuristic value，默认模式
};
  // 用于Open List的结点比较排序
  StateComparator();
  void init(CompareMode mode);

  bool operator() (StateNode *lhs, StateNode *rhs) const;
  bool operator() (PolicyState *lhs_ps, PolicyState *rhs_ps) const;

  CompareMode mode;
};

/* forward declaration of StateList functions */
extern void DisplayStateList(StateList *list);

/* external declaration of global data structures */
extern struct StateNode*    Start;
extern struct StateNode*    Goal;

#endif
