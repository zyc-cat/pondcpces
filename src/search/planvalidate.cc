#include "planvalidate.h"

#include "graph_wrapper.h" 
#include "lao_wrapper.h" 
#include "dd.h" 
#include "globals.h"
#include "astar.h"
#include "lug.h"

using namespace std;

Planvalidate::Planvalidate(){}

/**
 * 返回true，说明查找反例成功
 * 返回false，说明反例不存在
*/
DdNode *Planvalidate::backwardToInitial(DdNode *curr, DdNode* remove)
{
    DdNode *tmp = Cudd_bddAnd(manager, Cudd_Not(remove), curr);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager, curr);
    curr = tmp; 
    
    std::cout << "当前状态变量不蕴涵当前动作前提条件，逆推寻找反例：" << std::endl;
    for (std::vector<const Action *>::reverse_iterator a_it = reverse_action.rbegin(); a_it != reverse_action.rend();++a_it)
    {
        pair<const Action *const, DdNode *> act_pair(*a_it, action_preconds[*a_it]);
        DdNode *successor = regression(curr, &act_pair);
        Cudd_RecursiveDeref(manager, curr);
        curr = successor;
    }

    DdNode* counter = Cudd_bddAnd(manager, curr, init_states);
    Cudd_Ref(counter);
    Cudd_RecursiveDeref(manager, curr);

    if (counter == Cudd_ReadLogicZero(manager))
    {
        std::cout << "=====================" << std::endl;
        std::cout << "反例为false" << std::endl;
        std::cout << "=====================" << std::endl;
        return Cudd_ReadLogicZero(manager);
    }
    else
    {
        int cardSize = getCardinality(counter);
        std::cout << "找到反例:" << cardSize << std::endl;
        if(cardSize > counterSize)
        {
            DdNode* t = pickKRandomWorlds(counter, counterSize);
            Cudd_Ref(t);
            Cudd_RecursiveDeref(manager, counter);
            counter = t;
        }
        return counter;
    }
}
bool Planvalidate::planvalidate(DdNode *&ce){

    if(init_states == Cudd_ReadLogicZero(manager))
    {
        std::cout << "===================================" << std::endl;
        std::cout << "候选Sample状态集合为空,反例不存在" << std::endl;
        return false;
    }


    DdNode *curr = init_states; // 当前状态
    Cudd_Ref(curr);
    reverse_action.clear();
    for (std::vector<const Action *>::iterator act_it = candidateplan.begin(); act_it != candidateplan.end(); act_it++)
    {
        const Action *action = *act_it;
        reverse_action.push_back(action);
        DdNode *preBdd = action_preconds.find(*act_it)->second;
        Cudd_Ref(preBdd);
        if (bdd_entailed(manager, curr, preBdd))
        {
            // cout << "当前状态变量蕴涵当前动作前提条件" << endl;
            pair<const Action *const, DdNode *> act_pair(*act_it, action_preconds[*act_it]);
            DdNode *successor = progress(curr, &act_pair);
            Cudd_RecursiveDeref(manager, curr);
            curr = successor;
        }
        else
        {
            ce = backwardToInitial(curr, preBdd);
            return ce != Cudd_ReadLogicZero(manager);
        }
    }
    // 若执行完候选规划中的所有动作，检查最终状态是否满足目标
    if (bdd_entailed(manager, curr, b_goal_state))
    {
        Cudd_RecursiveDeref(manager, curr);
        std::cout << "最终状态满足目标, 当前规划有效" << std::endl;
        return false;
    }
    else
    {
        std::cout << "最终状态不满足目标，逆推寻找反例：" << std::endl;
        ce = backwardToInitial(curr, b_goal_state);
        return ce != Cudd_ReadLogicZero(manager);
    } 
}