#include "aostar.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "track.h"
#include "graph.h"
#include "lao.h"
#include "backup.h"
#include "solve.h"
#include "../globals.h"
#include "lao_wrapper.h"        // printAction
#include "graph.h"              // StateNode::expand
#include "vi.h"
#include "lug.h"
#include <iostream>
#include <float.h>
#include <math.h>
#include "../ppddl/mtbdd.h"

using namespace __gnu_cxx;
using namespace std;


AOStar::AOStar()
: StepSearch(AOSTAR){
}

void AOStar::setup(StateNode *q){
    StepSearch::setup(q);
    closed.clear();
    open.clear();
    open.key_comp().init(StateComparator::F_VAL);
    m_depth = -1;
    start->m_status = UNEXPANDED;
    start->PrevActions = new ActionNodeList();
    start->BestPrevAction = NULL;
    start->NextActions = new ActionNodeList();
    open.insert(start);
}


// check
void AOStar::goal_propagate(StateNode* n1, StateNode* n2)
{
    // 当前状态出发，传递后继状态
    bool flagDel;
    ActionNode *temp = NULL;
    ActionNodeList::iterator act_it;
    for (act_it = n1->NextActions->begin(); act_it != n1->NextActions->end(); ++act_it)
    {
        ActionNode *action = *act_it;
        StateDistribution *nextStateWarp = action->NextState;
        StateNode *nextState;
        flagDel = false;
        // 目前仅考虑 or结点,如果and结点需要在处理.
        while (nextStateWarp != NULL)
        {
            nextState = nextStateWarp->State;
            if (nextState != n2)
            {
                flagDel = true;
                isolation_propagate(nextState);
                // 将Action从两个Node之间断开，还需要释放内存
                nextState->PrevActions->remove(action);
            }
            else
                temp = action;
            nextStateWarp = nextStateWarp->Next;
        }
        if(flagDel) delete action;// release the memory
    }
    n1->NextActions->clear();
    assert(temp);
    n1->NextActions->push_back(temp);

    for (act_it = n1->PrevActions->begin(); act_it != n1->PrevActions->end();++act_it)
    {
        ActionNode *action = *act_it;
        StateNode *another = NULL;
        StateNode *preState = action->PrevState;
        if (action->act->hasObservation()) // sensor action
        {
            if(action->NextState->State == n1)
            {
                another = action->NextState->Next->State;
                if(another->m_status == GOAL_REACHABLE)
                {
                    preState->m_status = GOAL_REACHABLE;
                    goal_propagate(preState, n1, another);
                }
            }
            else
            {
                another = action->NextState->State;
                if(another->m_status == GOAL_REACHABLE)
                {
                    preState->m_status = GOAL_REACHABLE;
                    goal_propagate(preState, another, n1);
                }
            }
        }
        else
        {
            preState->m_status = GOAL_REACHABLE;
            goal_propagate(preState, n1);
        }
    }
}

// check
void AOStar::goal_propagate(StateNode *par, StateNode* n1, StateNode* n2)
{
    // 当前状态出发，传递后继状态

    bool flagDel;
    ActionNodeList::iterator act_it;
    ActionNode *temp = NULL;

    for (act_it = par->NextActions->begin(); act_it != par->NextActions->end(); ++act_it)
    {
        ActionNode *action = *act_it;
        StateDistribution *nextStateWarp = action->NextState;
        StateNode *nextState;
        flagDel = false;
        while (nextStateWarp != NULL)
        {
            nextState = nextStateWarp->State;
            if(nextState != n1 && nextState != n2)// 不等于目前处理的and
            {
                flagDel = true;
                isolation_propagate(nextState);
                // 将Action从两个Node之间断开，还需要释放内存
                nextState->PrevActions->remove(action);
            }
            else// not need to delete
            {
                temp = action;// recode the action
                break;
            }
            nextStateWarp = nextStateWarp->Next;
        }
        if(flagDel) delete action;
    }
    par->NextActions->clear();
    assert(temp);
    par->NextActions->push_back(temp);

    for (act_it = par->PrevActions->begin(); act_it != par->PrevActions->end(); ++act_it)
    {
        ActionNode *action = *act_it;
        StateNode *another = NULL;
        StateNode *preState = action->PrevState;// 获取前继状态
        if (action->act->hasObservation()) // sensor action
        {
            if(action->NextState->State == par)
            {
                another = action->NextState->Next->State;
                if(another->m_status == GOAL_REACHABLE)
                {
                    preState->m_status = GOAL_REACHABLE;
                    goal_propagate(preState, par, another);
                }
            }
            else
            {
                another = action->NextState->State;
                if(another->m_status == GOAL_REACHABLE)
                {
                    preState->m_status = GOAL_REACHABLE;
                    goal_propagate(preState, another, par);
                }
            }
        }
        else
        {
            preState->m_status = GOAL_REACHABLE;
            goal_propagate(preState, par);
        }
    }
}
// check
void AOStar::dead_propagate(StateNode* node)
{
    // 当前结点存在前驱action
    while(node->PrevActions->size()>0)
    {
        // 获取当前动作和preState
        ActionNode *action = node->PrevActions->front();
        StateNode *another = NULL;
        StateNode *preState = action->PrevState;
        if (action->act->hasObservation())
        {
            // 获取另一个And-State
            if(action->NextState->State == node)
            {
                another = action->NextState->Next->State;
            }
            else
            {
                another = action->NextState->State;
            }
            assert(another);
            isolation_propagate(another); // isolate another
            // remove the action node
            another->PrevActions->remove(action);
            preState->NextActions->remove(action);
            if(preState->NextActions->size() == 0)
            {
                preState->m_status = DEAD;
                dead_propagate(preState);
            }
        }
        else
        {
            preState->NextActions->remove(action);//remove the actionNode
            if(preState->NextActions->size() == 0)
            {
                preState->m_status = DEAD;
                dead_propagate(preState);
            }
        }
        // 当前结点断开
        node->PrevActions->remove(action);
        delete action;// release the memory
    }
}
// check only change the status
void AOStar::isolation_propagate(StateNode* node)
{
    node->m_active_in_transitions--;
    if(node->m_active_in_transitions || node->m_status == GOAL_REACHABLE)
        return;
    if(node->m_status == UNEXPANDED)
    {
        node->m_status == ISOLATED;
        return;
    }
    for (ActionNodeList::iterator ite = node->NextActions->begin(); ite != node->NextActions->end(); ++ite)
    {
        isolation_propagate((*ite)->NextState->State);
        if((*ite)->NextState->Next)
        {
            isolation_propagate((*ite)->NextState->Next->State);
        }
    }
}
// check
void AOStar::reconnection_propagate(StateNode* node)
{
    node->m_active_in_transitions++;
    if(node->m_active_in_transitions > 1 || node->m_status == GOAL_REACHABLE)
        return;
    if(node->m_status == ISOLATED)
        node->m_status == UNEXPANDED;
    else if(node->m_status == OUT_ISOLATED)
    {
        node->m_status = UNEXPANDED;
        open.insert(node);
    }
    else
    {
        for (ActionNodeList::iterator ite = node->NextActions->begin(); ite != node->NextActions->end();++ite)
        {
            reconnection_propagate((*ite)->NextState->State);
            if((*ite)->NextState->Next)
            {
                reconnection_propagate((*ite)->NextState->Next->State);
            }
        }
    }
}
// done
bool AOStar::new_action_transition(StateNode* node, ActionNode* action, DdNode* successor)
{
    bool isNew = StateNode::generated.count(successor) == 0;
    list<StateNode *> m_states;
    StateNode *child;
    if (isNew)
    {
        child = new StateNode();
        child->StateNo = state_count++;
        child->dd = successor;

        child->horizon = node->horizon + 1;
		child->kld = 1.0;
		child->goalSatisfaction = child->isGoal();// 更新该结点是否满足
		child->ExtendedGoalSatisfaction = child->goalSatisfaction;
        child->prReached = node->prReached * 1.0;
        child->g = node->g + 1;
        child->BestAction = NULL;
        child->horizon = node->horizon + 1;
        
        child->PrevActions = new ActionNodeList();
        child->NextActions = new ActionNodeList();
        action->NextState = new StateDistribution();

        action->NextState->State = child;
        action->NextState->Next = NULL;
        child->PrevActions->push_back(action);

        list<StateNode *> m_states;
        m_states.push_back(child);
        getHeuristic(&m_states, node, node->horizon + 1);
        m_states.clear();
        // following from cnf planner
        if(child->isGoal()){// 当前满足goal
            child->m_status = GOAL_REACHABLE;
            node->m_status = GOAL_REACHABLE;
            goal_propagate( node, child);
        }
        else
        {
            open.insert(child);
        }
        StateNode::generated[successor] = child;
    }
    else
    {
        child = StateNode::generated[successor]; //直接获取stateNode
        action->NextState = new StateDistribution();
        action->NextState->State = child;
        action->NextState->Next = NULL;
        child->PrevActions->push_back(action);
        // Dead-end
        if (child->dd == node->dd || child->m_status == DEAD)
        {
            return false;
        }
        if(child->m_status == GOAL_REACHABLE)
        {
            node->m_status = GOAL_REACHABLE;
            goal_propagate(node, child);
        }else
        {
            reconnection_propagate(child);
        }
    }
    // link node and successor
    return true;
}

bool AOStar::new_sensing_transition(StateNode* node, ActionNode* action, DdNode* c1, DdNode* c2)
{
    StateNode *lchild = NULL, *rchild = NULL;
    bool lExist = StateNode::generated.count(c1) != 0;
    bool rExist = StateNode::generated.count(c2) != 0;
    short lstatus = lExist ? StateNode::generated[c1]->m_status : -1;
    short rstatus = rExist ? StateNode::generated[c2]->m_status : -1;
    if (lExist && lstatus == DEAD ||
        rExist && rstatus == DEAD)
    {
        // not need to create the StateNode
        return false;
    }

    if( lExist && (lstatus == ISOLATED || lstatus == OUT_ISOLATED) ||
        rExist && (lstatus == ISOLATED || rstatus == OUT_ISOLATED))
    {
        // not need to create StateNode
        std::cout << "ISOLATION, IGNORE SENSING";
        return false;
    }

    // Treat StateNode lcild
    if(lExist == false)
    {
        // TO.DO
        // create the new state node
        // test goal reachable
        // insert into open
        // insert into StateNode::generated table.
        lchild = new StateNode();
        lchild->StateNo = state_count++;
        lchild->dd = c1;

        lchild->horizon = node->horizon + 1;
		lchild->kld = 1.0;
		lchild->goalSatisfaction = lchild->isGoal();// 更新该结点是否满足
		lchild->ExtendedGoalSatisfaction = lchild->goalSatisfaction;
        lchild->prReached = node->prReached * 1.0;
        lchild->g = node->g + 1;
        lchild->BestAction = NULL;
        lchild->horizon = node->horizon + 1;
        
        lchild->PrevActions = new ActionNodeList();
        lchild->NextActions = new ActionNodeList();
        action->NextState = new StateDistribution();

        action->NextState->State = lchild;
        action->NextState->Next = NULL;
        lchild->PrevActions->push_back(action);

        list<StateNode *> m_states;
        m_states.push_back(lchild);
        getHeuristic(&m_states, node, node->horizon + 1);
        m_states.clear();

        if(lchild->isGoal())
        {
            lchild->m_status = GOAL_REACHABLE;
        }
        open.insert(lchild);
        StateNode::generated[c1] = lchild;
    }
    else
    {
        // no create the new StateNode
        lchild = StateNode::generated[c1];
        action->NextState = new StateDistribution();
        action->NextState->State = lchild;
        action->NextState->Next = NULL;
        lchild->PrevActions->push_back(action);
        if (lchild->m_status == ISOLATED)
        {
            lchild->m_status = UNEXPANDED;
        }
        else if(lchild->m_status == OUT_ISOLATED)
        {
            lchild->m_status = UNEXPANDED;
            open.insert(lchild);
        }
    }
    // 当前连接和goal_propagate顺序需要注意下,可能有bug.
    // linke node and lchild;
    if (rExist == false)
    {
        // std::cout << "a1\n";
        rchild = new StateNode();
        rchild->StateNo = state_count++;
        rchild->dd = c2;

        rchild->horizon = node->horizon + 1;
		rchild->kld = 1.0;
		rchild->goalSatisfaction = rchild->isGoal();// 更新该结点是否满足
		rchild->ExtendedGoalSatisfaction = rchild->goalSatisfaction;
        rchild->prReached = node->prReached * 1.0;
        rchild->g = node->g + 1;
        rchild->BestAction = NULL;
        rchild->horizon = node->horizon + 1;
        
        rchild->PrevActions = new ActionNodeList();
        rchild->NextActions = new ActionNodeList();
        action->NextState->Next = new StateDistribution();
        action->NextState->Next->State = rchild;
        action->NextState->Next->Next = NULL;
        rchild->PrevActions->push_back(action);

        list<StateNode *> m_states;
        m_states.push_back(rchild);
        getHeuristic(&m_states, node, node->horizon + 1);
        m_states.clear();
        // std::cout << "a2\n";
        if (rchild->isGoal())
        {
            rchild->m_status = GOAL_REACHABLE;
            if(lchild->m_status == GOAL_REACHABLE)
            {
                node->m_status = GOAL_REACHABLE;
                goal_propagate(node, lchild, rchild);
            }
        }
        else
        {
            open.insert(rchild);
        }
        // std::cout << "a3\n";
        StateNode::generated[c2] = rchild;
    }
    else
    {
        // std::cout << "b1\n";
        rchild = StateNode::generated[c2];
        action->NextState->Next = new StateDistribution();
        action->NextState->Next->State = rchild;
        action->NextState->Next->Next = NULL;
        rchild->PrevActions->push_back(action);
        rchild->m_active_in_transitions++;
        if (rchild->m_status == GOAL_REACHABLE)
        {
            if(lchild->m_status == GOAL_REACHABLE)
            {
                // std::cout << "e1\n";
                node->m_status = GOAL_REACHABLE;
                goal_propagate(node, lchild, rchild);
            }
        }
        else if(rchild->m_status == ISOLATED){
            rchild->m_status = UNEXPANDED;
        }
        else if(rchild->m_status == OUT_ISOLATED)
        {
            rchild->m_status = UNEXPANDED;
            open.insert(rchild);
        }
        else
        {
            // std::cout << "e2\n";
            reconnection_propagate(rchild);
            if(lchild->m_status == EXPANDED)
            {
                // std::cout << "e3\n";
                reconnection_propagate(lchild);
            }
        }
        // std::cout << "b2\n";
    }
    // link node and rchild
    return true;
}
// check
StateNode* AOStar::first_unexpanded()
{
    StateNode *cs;
    while (open.size() && (*open.begin())->m_status != UNEXPANDED)
    {
        if((*open.begin())->m_status == ISOLATED)
            (*open.begin())->m_status = OUT_ISOLATED;
        open.erase(open.begin());
    }
    if(open.size()){
        cs = *open.begin();
        open.erase(open.begin());
        return cs;
    }
    return NULL;
}
// check
bool decides(DdNode* cur, DdNode* literal)
{
    if(bdd_entailed(manager,cur, literal) || bdd_entailed(manager, cur, Cudd_Not(literal)))
    {
        // cout << "Not optimize\n";
        return true;
    }
    else
        return false;
}
// 还缺少状态更新的部分
bool AOStar::expand(StateNode* node)
{
    StateNode *n1, *n2;
    map<const Action *, DdNode *>::iterator ite;
    ActionNodeList::iterator act_ite;
    // get the noop and executable action list
    for (ite = action_preconds.begin(); ite != action_preconds.end(); ite++)
    {
        const Action *act = ite->first;
        if (act->name().compare("noop_action") == 0)
            continue;
        DdNode *preBdd = ite->second;// 获取前提条件
        Cudd_Ref(preBdd);
        if (bdd_isnot_one(manager, bdd_imply(manager, node->dd, preBdd)))
        {
            continue;
        }
        
        if(act->hasObservation()) // sensor action
        {
            DdNode *effBDD = NULL;
            if (action_observations.find(act) != action_observations.end())
            {
                effBDD = action_observations[act]->front();
            }
            else
            {
                effBDD = getObservationDD(*act);
            }
            if(decides(node->dd, effBDD))
            {
                continue;
            }
            DdNode *s1 = Cudd_bddAnd(manager, node->dd, effBDD);
            DdNode *s2 = Cudd_bddAnd(manager, node->dd, Cudd_Not(effBDD));
            Cudd_Ref(s1);
            Cudd_Ref(s2);
            ActionNode *actNode = new ActionNode();
            actNode->act = act;
            actNode->PrevState = node;
            actNode->ActionNo = act->id();
            actNode->NextState = NULL;
            // 创建连接
            node->NextActions->push_back(actNode);
            // std::cout << "enter1\n";
            new_sensing_transition(node, actNode,s1, s2);
            // std::cout << "done1\n";
            if(Start->m_status == GOAL_REACHABLE)
            {
                return true;
            }
            if(node->m_status == GOAL_REACHABLE)
            {
                return false;
            }
        }
        else
        {
            // pair<const Action *const, DdNode *> act_pair(act, preBdd);
            // DdNode *successor = progress(&act_pair, node->dd);//根据当前状态和动作计算后继状态
            DdNode *successor = progress(node->dd, act);
            // 前面已经完成了Precondition测试
            if(bdd_is_zero(manager,successor))
            {
                continue;
            }
            if(successor == node->dd)
            {
                // Cudd_RecursiveDeref(manager, successor);
                continue;
            }
            // 创建ActionNode
            ActionNode *actNode = new ActionNode();
            actNode->act = act;
            actNode->PrevState = node;// 每个actNode只有一个前驱StateNode
            actNode->ActionNo = act->id();
            actNode->NextState = NULL;
            // 创建连接
            node->NextActions->push_back(actNode);
            // std::cout << "enter2\n";
            new_action_transition(node, actNode, successor);
            // std::cout << "done2\n";
            if(Start->m_status == GOAL_REACHABLE){
                return true;
            }
            if(node->m_status == GOAL_REACHABLE){
                return false;
            }
        }
    }
    // 不存在ActionNode没有StateNode连接
    if(node->NextActions->size()==0)
    {
        node->m_status = DEAD;
        cout << "DEAD " << node->StateNo << endl;
        dead_propagate(node);
        if(Start->m_status = DEAD)
            return true;
    }
    return false;
}
void AOStar::search()
{
    setup(Start);
    StateNode *node;

    if(Start->isGoal()){
        std::cout << "The initial state satisies the goal" << std::endl;
        return;
    }
    expandedNodes = 0;

    while (node = first_unexpanded())
    {
        expandedNodes++; // 拓展结点数+1
        std::cout << "expand node" << node->f << " " << node->g << " " << node->h << std::endl;
        if (expand(node))
        {
            break;
        }
    }
    if(Start->m_status != GOAL_REACHABLE){
        std::cout << "No plan was found.\n";
    }
    else{
        std::cout << "Solution was found\n";
        solution_print(Start, 0);
        cout << "Sizeof solution: " << m_commandNo.size() << "\t Depth: " << m_depth << endl;
    }
}
// check
void AOStar::solution_print(StateNode* node, int level)
{
    int i;
    map<StateNode *, int>::iterator it;
    for (int i = 0; i < level; ++i) std::cout << " ";
    if(node->isGoal())
    {
        if(m_depth < node->m_depth)
            m_depth = node->m_depth;
        return;
    }
    if(node->NextActions->size() == 0)
    {
        cout << "DEAD-END, WRONG PLAN!";
        return;
    }
    ActionNode* actNode = node->NextActions->front();
    const Action *act = actNode->act;
    std::cout << m_commandNo.size() << ": ";
    // << act->name() << std::endl;
    act->print(std::cout, my_problem->terms());
    std::cout << std::endl;
    m_commandNo.insert(make_pair(node, m_commandNo.size()));
    if (!act->hasObservation())
    {
        StateNode *successor = actNode->NextState->State;
        it = m_commandNo.find(successor);
        if ( it == m_commandNo.end())
        {
            successor->m_depth = node->m_depth + 1;
            solution_print(successor, level);
        }
        else
        {
            if(successor->m_depth <= node->m_depth)
            {
                successor->m_depth = node->m_depth + 1;
                depth_propagate(successor);
            }
            for (int i = 0; i < level;++i)
                std::cout << " ";
            std::cout << "GOTO " << it->second << std::endl;
        }
    }
    else
    {
        // 小于2个结果
        if(node->NextActions->front()->NextState == NULL || node->NextActions->front()->NextState->Next == NULL)
        {
            std::cout << "WRONG TRANSITION STRUCTURE!";
            return;
        }
        cout << " IF:\n";
        StateNode *lc = actNode->NextState->State;
        it = m_commandNo.find(lc);
        if ( it == m_commandNo.end())
        {
            lc->m_depth = node->m_depth + 1;
            solution_print(lc, level + 1);
        }
        else
        {
            if(lc->m_depth <= node->m_depth)
            {
                lc->m_depth = node->m_depth + 1;
                depth_propagate(lc);
            }
            for (int i = 0; i <= level;++i) std::cout << " ";
            cout << "GOTO " << it->second << endl;
        }

        for (int i = 0; i < level;++i) cout << " ";
        cout << "IF NOT:\n";
        StateNode *rc = actNode->NextState->Next->State;
        it = m_commandNo.find(rc);
        if( it == m_commandNo.end())
        {
            rc->m_depth = node->m_depth + 1;
            solution_print(rc, level + 1);
        }
        else
        {
            if(rc->m_depth <= node->m_depth)
            {
                rc->m_depth = node->m_depth + 1;
                dead_propagate(rc);
            }
            for (int i = 0; i <= level;++i) std::cout << " ";
            cout << "GOTO " << it->second << endl;
        }
    }
    return;
}

// check
void AOStar::depth_propagate(StateNode *node)
{
    ActionNodeList::iterator it;
    StateNode *st;
    for (it = node->NextActions->begin(); it != node->NextActions->end(); ++it)
    {
        st = (*it)->NextState->State;
        if(st->m_status <= node->m_depth)
        {
            st->m_depth = node->m_depth + 1;
            depth_propagate(st);
        }
        if((*it)->NextState->Next)
        {
            st = (*it)->NextState->Next->State;
            if(st->m_depth <= node->m_depth)
            {
                st->m_depth = node->m_depth + 1;
                depth_propagate(st);
            }
        }
    }
    if(node->NextActions->size() ==0 && m_depth < node->m_depth)
    {
        m_depth = node->m_depth;
    }
}