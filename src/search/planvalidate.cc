#include "planvalidate.h"

#include "graph_wrapper.h" 
#include "lao_wrapper.h" 
#include "dd.h" 
#include "globals.h"
#include "astar.h"
#include "lug.h"

using namespace std;

Planvalidate::Planvalidate():random_sample_time(0) {}

/**
 * 返回true，说明查找反例成功
 * 返回false，说明反例不存在
*/
DdNode *Planvalidate::backwardToInitial(DdNode *curr, DdNode* remove)
{
    DdNode *tmp = Cudd_bddAnd(manager, Cudd_Not(remove), curr);
    Cudd_Ref(tmp);
    for (std::vector<const Action *>::reverse_iterator a_it = reverse_action.rbegin(); a_it != reverse_action.rend(); ++a_it)
    {
        // std::cout << "regression:";
        pair<const Action *const, DdNode *> act_pair(*a_it, action_preconds[*a_it]);
        DdNode *successor = regression(tmp, &act_pair);
        Cudd_Ref(successor);
        /*if(successor == Cudd_ReadLogicZero(manager))
        {
             cout << "currrent:\n";
             printBDD(tmp);
             cout << "after regression\n";
             printBDD(successor);
             cout << "transition action:\n";
             printBDD(groundActionDD(**a_it));
             cout << flush << endl;
             assert(false);
        }*/
        Cudd_RecursiveDeref(manager, tmp);
        tmp = successor;
        if(tmp == Cudd_ReadLogicZero(manager))
        {
            break;
        }
    }
    // DdNode *tmp2 = tmp;
    // Cudd_Ref(tmp2);
    // for (std::vector<const Action *>::iterator a_it = reverse_action.begin(); a_it != reverse_action.end(); ++a_it)
    // {
    //     pair<const Action *const, DdNode *> act_pair(*a_it, action_preconds[*a_it]);
    //     if(bdd_entailed(manager, tmp2,  action_preconds[*a_it]) == 0)
    //         assert(false);
    //     DdNode *successor = progress(tmp2, &act_pair);
    //     Cudd_Ref(successor);
    //     Cudd_RecursiveDeref(manager, tmp2);
    //     tmp2 = successor;
    // }

    // assert(tmp2 == Cudd_bddAnd(manager, Cudd_Not(remove), curr));

    DdNode* counter = Cudd_bddAnd(manager, tmp, init_states);
    Cudd_Ref(counter);
    Cudd_RecursiveDeref(manager, tmp);

    if (counter == Cudd_ReadLogicZero(manager))
    {
        std::cout << "=====================" << std::endl;
        std::cout << "Counter sample is false" << std::endl;
        std::cout << "=====================" << std::endl;
        std::cout << "Random sample one:" << std::endl;
        random_sample_time += 1;
        return getKSample(init_states);
    }
    else
    {
        int cardSize = getCardinality(counter);
        std::cout << "Get counter sample:" << cardSize << std::endl;
        if(cardSize > counterSize)
        {
            // DdNode* t = pickKRandomWorlds(counter, counterSize);
            DdNode* t = getKSample(counter);
            Cudd_Ref(t);
            // printBDD(t);
            cout << "Add Counter Sample Size:" << getCardinality(t) << endl;
            Cudd_RecursiveDeref(manager, counter);
            counter = t;
        }
        return counter;
    }
}

DdNode *Planvalidate::getKSample(DdNode* counter)
{
    DdNode **kbdd = Cudd_bddPickArbitraryMinterms(manager, counter, current_state_vars, num_alt_facts,counterSize);

    DdNode *res = Cudd_ReadLogicZero(manager);
    Cudd_Ref(res);
    DdNode *temp;
    for (int i = 0; i < counterSize; ++i)
    {
        temp = Cudd_bddOr(manager, kbdd[i], res);
        Cudd_Ref(temp);
        Cudd_RecursiveDeref(manager, res);
        res = temp;
    }
    return temp;
}
bool Planvalidate::planvalidate(DdNode *&ce){

    if(init_states == Cudd_ReadLogicZero(manager))
    {
        std::cout << "===================================" << std::endl;
        std::cout << "Candidate Sample state is empty, couter sample does not exist" << std::endl;
        return false;
    }

    std::cout << "start validate:";
    // printBDD(init_states);
    DdNode *curr = init_states; // 当前状态
    Cudd_Ref(curr);
    reverse_action.clear();
    for (std::vector<const Action *>::iterator act_it = candidateplan.begin(); act_it != candidateplan.end(); act_it++)
    {
        const Action *action = *act_it;
        DdNode *preBdd = action_preconds.find(*act_it)->second;
        Cudd_Ref(preBdd);
        if (bdd_entailed(manager, curr, preBdd))
        {
            reverse_action.push_back(action);
            // cout << "当前状态变量蕴涵当前动作前提条件" << endl;
            pair<const Action *const, DdNode *> act_pair(*act_it, action_preconds[*act_it]);
            DdNode *successor = progress(curr, &act_pair);
            Cudd_Ref(successor);
            Cudd_RecursiveDeref(manager, curr);
            curr = successor;
        }
        else
        {
            std::cout << "The action precondition does not sat in current state, find counter sample" << std::endl;
            ce = backwardToInitial(curr, preBdd);
            return ce != Cudd_ReadLogicZero(manager);
        }
    }
    // 若执行完候选规划中的所有动作，检查最终状态是否满足目标
    if (bdd_entailed(manager, curr, b_goal_state))
    {
        std::cout << "The Goal is sat in Final state, valid plan" << std::endl;
        return false;
    }
    else
    {
        std::cout << "The Goal does not sat in Final state, find counter sample:" << std::endl;
        ce = backwardToInitial(curr, b_goal_state);
        return ce != Cudd_ReadLogicZero(manager);
    } 
}

int Planvalidate::getRandomSampleTime()
{
    return this->random_sample_time;
}