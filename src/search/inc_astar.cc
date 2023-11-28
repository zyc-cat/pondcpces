#include "inc_astar.h"

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

Incremental_AStar::Incremental_AStar():AStar(){
}
/**
 * 算法初始化
 */
void Incremental_AStar::setup(StateNode* start){
	closed.clear();// 清空两个表
	open.clear();
	pclosed.clear();
	popen.clear();
	open.key_comp().init(StateComparator::F_VAL);
	popen.key_comp().init(StateComparator::F_VAL);
	next = start;
	next->PrevActions = NULL;
	next->BestPrevAction = NULL; //最佳action值为空
	next->NextActions = NULL;
	open.insert(next); // 插入节点next
}
// 为了使得接口和其他Astar一致，重写判读第一次初始化
void Incremental_AStar::search()
{
	if(iteration == 0)
	{
		setup(Start);
	}
	this->start = Start;
	iteration++;
	reusePrev = 0;
	while (step());
	cleanup();
}
/**
 * 返回true，说明需要继续搜索
 * 返回false，说明查找失败或到达目标
 */
bool Incremental_AStar::step(){
	assert(next != NULL);
	if (open.empty() || next->isGoal())
	{
		cout << "\t[sNo = " << next->StateNo << "]\t[g = " << next->g << "]\t[h = " << next->h << "]\t[t = " << next->goalSatisfaction << "]\t[pr =" << next->prReached  << "]"<< endl;
		if(open.empty())
			cout << "Dead end!\n";
		if(next->isGoal()){
			cout << "Found branch!\n";
			planlist(candidateplan);	// zyc 存储规划序列到candidateplan中
			std::cout << "successfully found candidateplan" << std::endl;
		}
		return false;// 返回false停止搜索
	}
	open.erase(open.begin());
	closed.insert(next);
	if(next->NextActions==NULL)
	{
		next->NextActions = new ActionNodeList();
	}
	for(map<const Action*, DdNode*>::iterator a = action_preconds.begin(); a != action_preconds.end(); a++){
		if((*a).first->name().compare("noop_action") == 0)
			continue;
		DdNode *preBdd = action_preconds.find((*a).first)->second;
		if(bdd_isnot_one(manager, bdd_imply(manager, next->dd,preBdd)))
			continue;
		ActionNode *actNode = new ActionNode();
		actNode->act = (*a).first;
		actNode->PrevState = next;// 状态结点连接动作结点
		actNode->ActionNo = (*a).first->id();
		next->NextActions->push_back(actNode);
	}
	// 拓展结点个数+1
	expandedNodes++;
	// std::cout << "Expanded node:" << expandedNodes << endl;
	if (next->Expanded == 0)
		next->Expanded = iteration;
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
		// 后继状态为空
		// momo007 修复该bug
		if(bdd_is_zero(manager,successor))
		{
			std::cout << "meet zero successor states\n";
			continue;
		}
		if(successor == next->dd)
		{
			continue;
		}
		StateNode *child = NULL;
		// child可能是open或close
		bool isNew = (StateNode::generated.count(successor) == 0);
		if(!isNew)
		{
			child = StateNode::generated[successor];
			Cudd_RecursiveDeref(manager, successor);
			reuseTime[iteration]++;
			totalReuse++;
			if(child->Expanded < iteration)
			{
				reusePrev++;
			}
			if(action->NextState == NULL)
			{
				action->NextState = new StateDistribution();
				action->NextState->State = child;
				action->NextState->Next = NULL;
			}
			updateLists(next, child, action);
		}
		// new node
		if(child == NULL || open.find(child) == open.end() && closed.find(child)== closed.end())
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
			child->BestPrevAction = *act_it;
			open.insert(child);
		}
	}
	assert(open.size() > 0);
	next = *open.begin();
	return true;
}

/**
 * does not clean up the search space here.
 */
void Incremental_AStar::cleanup(){
	// next = NULL;
	std::cout << "TotalReuse: " << totalReuse << std::endl;
	std::cout << "reuseTime total in this iteration: " << reuseTime[iteration] << std::endl;
	std::cout << "ReusePrev in this iteration: " << reusePrev << std::endl;
}

void Incremental_AStar::updateOpenAndClose(DdNode* newStart)
{
    while(!open.empty())
    {
        popen.insert(*open.begin());
        open.erase(open.begin());
    }
    while(!closed.empty())
    {
        pclosed.insert(*closed.begin());
        closed.erase(closed.begin());
    }
    // Create a new start Node
    if(StateNode::generated.find(newStart) == StateNode::generated.end())
    {
		StateNode* nS = new StateNode();
		nS->StateNo = state_count++;
		nS->dd = newStart;
		Cudd_Ref(nS->dd);

		nS->horizon = next->horizon + 1;
		nS->kld = 1.0;
		// nS->goalSatisfaction = nS->isGoal();// 更新该结点是否满足
		nS->ExtendedGoalSatisfaction = nS->goalSatisfaction;
		nS->prReached = next->prReached * 1.0;         // HACK
		nS->g = next->g + 1;// action的cost默认为1
		nS->BestAction = NULL;// 最佳的下一个动作
		nS->horizon = next->horizon + 1;

		nS->PrevActions = new ActionNodeList();
		nS->NextActions = new ActionNodeList();		// Here (not in constructor) because it messes up StateNode::expand
		// momo007 here need to extra code to link the stateNode and ActionNode
		list<StateNode *> m_states;
		m_states.push_back(nS);
		getHeuristic(&m_states, nS, 0);
		m_states.clear();
		StateNode::generated[newStart] = nS;
		open.insert(nS);        
		Start = nS;
	}
	else if(popen.find(StateNode::generated[newStart])!= popen.end())
    {
        StateNode *s = StateNode::generated[newStart];
        s->g = 0;
		s->f = s->g + s->h;
		open.insert(s);
		popen.erase(s);
		Start = s;
	}
	else if(pclosed.find(StateNode::generated[newStart])!= pclosed.end())
    {
        StateNode *s = StateNode::generated[newStart];
        s->g = 0;
		s->f = s->g + s->h;
		s->Expanded = iteration + 1;
		closed.insert(s);
		pclosed.erase(s);
		for (ActionNodeList::iterator act_it = s->NextActions->begin(); act_it != s->NextActions->end(); act_it++)
		{
			if((*act_it)->NextState && (*act_it)->NextState->State)
				updateLists(s, (*act_it)->NextState->State, *act_it);
		}
		Start = s;
	}
	assert(open.size() > 0);
	next = *open.begin();
}

void Incremental_AStar::updateLists(StateNode* parent, StateNode* child, ActionNode* actNode)
{
	// new iteration research the old node.
	// if(parent->Expanded > child->Expanded)
	// {
	if (popen.find(child) != popen.end())
	{
		child->BestPrevAction = actNode;
		child->g = parent->g + 1;
		child->f = child->g + child->h;
		open.insert(child);
		popen.erase(child);
	}
	else if(pclosed.find(child)!= pclosed.end())
	{
		child->BestPrevAction = actNode;
		child->g = parent->g + 1;
		child->f = child->g + child->h;
		child->Expanded = parent->Expanded;

		closed.insert(child);
		pclosed.erase(child);
		for (ActionNodeList::iterator act_it = child->NextActions->begin(); act_it != child->NextActions->end(); act_it++)
		{
			if((*act_it)->NextState && (*act_it)->NextState->State)
				updateLists(child, (*act_it)->NextState->State, *act_it);
		}
	}
	// }
	else
	{
		if(open.find(child)!=open.end() && parent->g + 1 < child->g)
		{
			child->g = parent->g + 1;
			child->f = child->f + child->g;
			child->BestPrevAction = actNode;
			open.erase(child);
			open.insert(child);
		}
	}
}