#ifndef LAO_WRAPPER_H
#define LAO_WRAPPER_H

#include "graph.h"
#include <iostream>
#include <list>
#include <set>
#include <map>
#include <utility>
#include "dbn.h"
// 该文件应该封装了lao实现了 AO*算法
// 搜索类
class Search;

#ifdef PPDDL_PARSER
int not_loop(StateNode* dest, BitVector* visited, StateNode* srcNo);
void increment_heuristic(StateNode*);// use in expand node
/**
 * momo007 2022.05.25 not need in conformant
 */
// DdNode * apply_events(DdNode*);
DdNode* normalize(DdNode*);// not used

double computeGoalSatisfaction(DdNode*);// 判断是否蕴含goal，成立返回1.0，否则0

struct StateFormula;
// 该动作更新接口，调用计算action的转换BDD，随后调用下面的progress(DdNode*image, DdNode* parent)接口
 DdNode* progress(std::pair<const Action* const, DdNode*>*, 
		 DdNode* parent);
 DdNode* progress(DdNode* parent, const Action* a);
//int split(std::map<const StateFormula*, DdNode*>,//std::pair<const Action* const, DdNode*>*, 
int split(std::list<DdNode*>*,
	  DdNode* parent, std::list<DdNode*>*, std::list<DdNode*>*);
int split(std::list<std::map<const pEffect*, DdNode* >*>* a,
	  DdNode* parent, std::list<DdNode*>* result,
	  std::list<std::set<const pEffect*>*>* reasons,
	  std::list<double>* probabilities,
	  int numSamples = 1
	  );
// double computeReward(std::pair<const Action* const, DdNode*>* action, DdNode* successor, DdNode* parent);
#else
DdNode* progress(alt_action*, 
		 DdNode* parent);
int split(alt_action* a , DdNode* parent, std::list<DdNode*>*, std::list<DdNode*>*);

int validLifted(DdNode* state, StateNode* prevNode, alt_action *a);
DdNode* regress(alt_action*, DdNode*);
#endif

DdNode* progress(DdNode*image, DdNode* parent);// 将动作理论和状态的BDD进行合取，遗忘，替换
DdNode* progress(dbn*image, DdNode* parent);// not used, main中solve_problem->collectInit->progress
void update_leaf_h_values();// not used
void backup_graph();// not used

struct action_list_node;
typedef action_list_node * action_list;
class goal_list;


struct StateDistribution *CreateStateDistribution( void );// 创建一个默认状态分布(概率为0等)
void printAction(struct ActionNode* a);// 后续可以用到
void CreateHeuristic();// not used
void DisplayStateList(StateList *list);// not used
StateList *CreateStateList();  // not used

// lug.cc中使用和 check_loop_conditions()中使用
BitVector *new_bit_vector(int length);// 创建一个指针长度为 length*int位
void free_bit_vector(BitVector*);
int get_bit(BitVector*, int, int);
void set_bit(BitVector* b, int index);

// 后续输出状态信息可能用到
DdNode** extractDNFfromBDD(DdNode* node);
DdNode** extractTermsFromMinterm(DdNode* node);

// call in StateNode::expand() -> StateNode::processSuccessors() -> check_loop_conditions()
int check_loop_conditions(StateNode* src, StateNode* dest, DdNode* state, int currHorizon);

DdNode * stateForward(DdNode*m,DdNode *f);// not used
DdNode * stateBackward(DdNode*m,DdNode *f);// not used

extern action_list available_acts;
// following does work well
void outputPlan();
void outputPlanR(StateNode* s, int indent, int, int&,int&, int&, BitVector*, double, std::list<double>*, double, std::list<double>*);

#endif

