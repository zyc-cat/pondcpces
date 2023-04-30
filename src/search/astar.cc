#include "astar.h"

#include "globals.h"						// verbosity
#include "graph.h"              // StateNode
#include "../ppddl/actions.h"
#include <queue>
#include <set>
#include <cassert>
#include <lao_wrapper.h>// momo for progress
#include <lug.h> // momo for getHeuristic
#include <sstream>
using namespace std;

extern double gWeight;

AStar::AStar()
: StepSearch(ASTAR){
}
/**
 * 算法初始化
 */
void AStar::setup(StateNode* start){
	StepSearch::setup(start);// 设置初始状态
	closed.clear();// 清空两个表
	open.clear();
	// open.key_comp().init(StateComparator::HEUR);// 默认使用HEUR
	open.key_comp().init(StateComparator::F_VAL);
	next = start;
	next->PrevActions = NULL;
	next->BestPrevAction = NULL; //最佳action值为空
	next->NextActions = NULL;
	open.insert(next); // 插入节点next
}
/**
 * 返回true，说明需要继续搜索
 * 返回false，说明查找失败或到达目标
 */
bool AStar::step(){
	assert(next != NULL);
	if(open.empty() || next->isGoal()){
		cout << "\t[sNo = " << next->StateNo << "]\t[g = " << next->g << "]\t[h = " << next->h << "]\t[t = " << next->goalSatisfaction << "]\t[pr =" << next->prReached  << "]"<< endl;
		if(open.empty())
			cout << "Dead end!\n";
		if(next->isGoal()){
			// printf("goal BDD is :");
			// printBDD(next->dd);
			cout << "Found branch!\n";
			// printBestPlan();//输出最佳方案
			planlist(candidateplan);	// zyc 存储规划序列到candidateplan中
			std::cout << "successfully found candidateplan" << std::endl;
		}

		return false;// 返回false停止搜索
	}

	open.erase(open.begin());
	closed.insert(next);
	// 添加所有满足当前状态动作到ActionNodeList中，核心，实现了状态结点
	// 这个接口有bug，展示重写，用简单版本
	// next->addAllActions();// 仅根据名字判断，不同参数会没有添加
	/**
	 * momo007 2022.06.10
	 */
	if(next->NextActions==NULL)
	{
		next->NextActions = new ActionNodeList();
	}
	for(map<const Action*, DdNode*>::iterator a = action_preconds.begin(); a != action_preconds.end(); a++){
		if((*a).first->name().compare("noop_action") == 0)
			continue;
		DdNode *preBdd = action_preconds.find((*a).first)->second;
		if(bdd_isnot_one(manager, bdd_imply(manager, next->dd,preBdd)))
		{
			continue;
		}
		ActionNode *actNode = new ActionNode();
		actNode->act = (*a).first;
		actNode->PrevState = next;// 状态结点连接动作结点
		actNode->ActionNo = (*a).first->id();
		next->NextActions->push_back(actNode);
	}
	// 拓展结点个数+1
	expandedNodes++;
	// std::cout << "######:" << expandedNodes << std::endl;
	// 考虑每个动作，计算后继状态，同时链接起来
	for (ActionNodeList::iterator act_it = next->NextActions->begin(); act_it != next->NextActions->end(); act_it++)
	{
		ActionNode *action = *act_it;
		// 计算得到后继状态结点
		debugCnt++;
		// std::cout << "######:" << debugCnt << std::endl;
		DdNode *successor;
		switch (progMode)
		{
		case FORGETTING:
		{
			DdNode *preBdd = action_preconds.find(action->act)->second;
			pair<const Action *const, DdNode *> act_pair(action->act, preBdd); // 动作及其前提条件pair
			successor = progress(next->dd, &act_pair);						   // method1 fogetting progress
			break;
		}
		case PARTITION_MERGE:
			successor = progress(next->dd, action->act); // method 2 partition progress
			break;
		case DEFINABILITY:
			successor = definability_progress(next->dd, action->act); // method3 definability progress
			break;
		}
		// if (act_ndefp.find(action->act) != act_ndefp.end())
		// {
		// 	if(act_ndefp[action->act].size()>1)
		// 	{
		// 		if (successor == successor1)
		// 			good[2]++;
		// 		else
		// 		{
		// 			good[3]++, printBDD(next->dd), printBDD(successor), printBDD(successor1), assert(0);
		// 		}
		// 	}
		// 	else
		// 	{
		// 		action->act->print(std::cout, my_problem->terms());
		// 		cout <<  "&&"  << endl << flush;
		// 		assert(successor == successor1);
		// 		good[1]++;
		// 	}
		// }
		// else
		// {
		// 	assert(successor1 == successor);
		// 	good[0]++;
		// }
		// 后继状态为空
		// momo007 修复该bug
		if(bdd_is_zero(manager,successor))
		{
			std::cout << "meet zero successor states\n";
			continue;
		}
		StateNode *child = NULL;
		// 查找该状态BDD是否遇到过
		bool isNew = (StateNode::generated.count(successor) == 0);
		if(isNew)// 新结点，创建StateNode
		{
			child = new StateNode();
			child->StateNo = state_count++;
			child->dd = successor;

			child->horizon = next->horizon + 1;
			child->kld = 1.0;
			child->goalSatisfaction = child->isGoal();// 更新该结点是否满足
			child->ExtendedGoalSatisfaction = child->goalSatisfaction;
			child->prReached = next->prReached * 1.0;         // HACK
			child->g = next->g + 1;// action的cost默认为1
			child->BestAction = NULL;// 最佳的下一个动作
			child->horizon = next->horizon + 1;

			child->PrevActions = new ActionNodeList();
			child->NextActions = new ActionNodeList();		// Here (not in constructor) because it messes up StateNode::expand
			// momo007 here need to extra code to link the stateNode and ActionNode
			action->NextState = new StateDistribution();
			action->NextState->State = child;
			action->NextState->Next = NULL;
			child->PrevActions->push_back(action);
			/** 至此，两个状态结点和动作结点完美连接，还差parent即next的bestAction
			 * 和child的bestPrevaction
			*/
			list<StateNode *> m_states;
			m_states.push_back(child);
			getHeuristic(&m_states, next, next->horizon + 1);// 计算child的启发值
			//state->h = 100;
			m_states.clear();

			StateNode::generated[successor] = child;
		}
		else
		{
			child = StateNode::generated[successor];
			Cudd_RecursiveDeref(manager, successor);
		}

		assert(child != NULL);
		double new_g = next->g + action->Cost;// A* Heuristic
		set<StateNode *, StateComparator>::iterator child_it = open.find(child);
		bool inFringe = (child_it != open.end());// 之前遇到过该状态
		bool isClosed = (closed.count(child) > 0);// 已经搜索过了
		bool cheaper = (new_g < child->g);// 是否比之前到达该child更便宜
		// 仅考虑没有被访问的情况
		// 在open中还没用拓展。1.不在open 2在open更近但更近，更新preAction
		if(!isClosed && (!inFringe || (inFringe && cheaper))){
			child->BestPrevAction = action;// 这里每个state的BestPreAction都设置了

			if(inFringe)// 删除open的stateNode
				open.erase(child_it);
			// 配置新的stateNode
			child->g = new_g;
			child->f = child->g + gWeight * child->h;

			child->prReached = next->prReached * 1.0;//dist->Prob;		// HACK
			child->horizon = next->horizon + 1;
			// 在无启发式的情况下，StateComparator中会使用该参数，从而影响搜索的方向
			// child->randPriority = rand();
			child->randPriority = 1;
			// 重新添加
			if(inFringe)
				open.insert(child_it, child);
			else
				open.insert(child);
		}
	}
	// 从open中取出最佳节点
	next = *open.begin();
	// setBestAct(next);// 在前面的逻辑中更新了，不需要调用该接口
	return true;
}

// 忽略不使用
double AStar::getQuality(){
	std::cout << "AStar::getQuality()\n";
	assert(0);
	assert(next != NULL);
	return next->goalSatisfaction;	// Assuming conformant search
}

void AStar::commit(){
	StateNode* state = next;
	do{
		state->Solved = 1;
	} while(state->BestPrevAction != NULL && (state = state->BestPrevAction->PrevState) != NULL && state->Solved <= 0);
}
/**
 * 从start结点开始，清空每个StateNode的bestAction和solve
 */
void AStar::cleanup(){
	StepSearch::cleanup();
	closed.clear();
	open.clear();
	next = NULL;
}

/**
 * momo007 011 2022.04.05 21.35
 * step中调用该函数，从state结点开始，不断设置最佳动作
 */
void AStar::setBestAct(StateNode* state){
	assert(state != NULL);
/*
  if(state->Solved <= 0)
		state->BestAction = NULL;
*/
	ActionNode* prevAction = NULL;
	StateNode* prevState = NULL;
	/**
	 * 从最后一个状态出发，设置每个状态的最佳动作
	 */
	while((prevAction = state->BestPrevAction) != NULL && (prevState = prevAction->PrevState) != NULL/* && prevState->Solved <= 0*/){
		prevState->BestAction = prevAction;
		state = prevState;
	}
}
// 当到达goal时调用，每个结点都设置了bestPrevAction
void AStar::printBestPlan()
{
	ActionNode *actNode;
	StateNode *stateNode = next;
	std::stringstream sstr;
	// StateNode* start;初始状态结点
	std::stack<ActionNode *> plan;
	int i = 0;
	std::cout << "start to print plan\n"
			  << std::flush;
	while (next->dd != start->dd)
	{
		if(next->BestPrevAction == NULL)
		{
			std::cout << "Plan abstract error\n"
					  << std::flush;
			abort();
		}
		i++;
		
		// next->BestPrevAction->act->print(sstr, my_problem->terms());
		plan.push(next->BestPrevAction);
		next = next->BestPrevAction->PrevState;
	}
	while(!plan.empty())
	{
		actNode = plan.top();
		plan.pop();
		actNode->act->print(std::cout, my_problem->terms());
		std::cout << "\n";
	}
	std::cout << "Plan Length is: " << i << std::endl;
}

/**
 * zyc12.27
*/
void AStar::planlist(std::vector<const Action*> &candplan){
	ActionNode *actNode;
	StateNode *stateNode = next;
	std::stack<ActionNode *> plan;
	int i = 0;
	std::cout << "start to print plan\n"
			  << std::flush;
	while (next->dd != start->dd)
	{
		if(next->BestPrevAction == NULL)
		{
			std::cout << "Plan abstract error\n"
					  << std::flush;
			abort();
		}
		i++;
		plan.push(next->BestPrevAction);
		next = next->BestPrevAction->PrevState;
	}
	// 将栈里的动作ActioNode转化到vector中的Action
	while(!plan.empty())
	{
		actNode = plan.top();
		plan.pop();
		actNode->act->print(std::cout, my_problem->terms());
		std::cout << "\n";
		candplan.push_back(actNode->act);
	}
	std::cout << "Plan Length is: " << i << std::endl;
}