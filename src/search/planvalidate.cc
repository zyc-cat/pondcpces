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
bool Planvalidate::planvalidate(DdNode *&ce){

    if(init_states == Cudd_ReadLogicZero(manager))
    {
        std::cout << "===================================" << std::endl;
        std::cout << "初始状态集合为空,反例不存在" << std::endl;
        return false;
    }


    DdNode *curr = init_states; // 当前状态变量  
    Cudd_Ref(curr);
    std::vector<const Action *> reverse_action;

    for (std::vector<const Action *>::iterator act_it = candidateplan.begin(); act_it != candidateplan.end(); act_it++)
    {
        const Action *action = *act_it;
        reverse_action.push_back(action);
        DdNode *preBdd = action_preconds.find(*act_it)->second;
        Cudd_Ref(preBdd);
        if (bdd_entailed(manager, curr, preBdd))
        {
            // cout << "当前状态变量蕴涵当前动作前提条件" << endl;
            DdNode *successor = progress(curr, *act_it);
            Cudd_Ref(successor);
            Cudd_RecursiveDeref(manager, curr);
            curr = successor;
        }
        else
        {

            //（1）获取不满足的那部分,这里是不满足当前动作的前提条件
            DdNode *stav = Cudd_bddAnd(manager, curr, preBdd);
            Cudd_Ref(stav);
            DdNode *tmp1 = Cudd_bddAnd(manager, Cudd_Not(stav), curr);
            Cudd_RecursiveDeref(manager, stav);
            Cudd_Ref(tmp1);
            Cudd_RecursiveDeref(manager, curr);
            curr = tmp1; 
            
            std::cout << "当前状态变量不蕴涵当前动作前提条件，逆推寻找反例：" << std::endl;
            for (std::vector<const Action *>::reverse_iterator a_it = reverse_action.rbegin(); a_it != reverse_action.rend();++a_it)
            {
                const Action *r_action = *a_it;
                DdNode *t = groundActionDD(*r_action);
                Cudd_Ref(t);

                // （2）将不满足的部分转化成后继状态表示
                DdNode *successor = Cudd_bddVarMap(manager, curr);
                Cudd_Ref(successor);

                // （3）根据后继状态表示以及动作前提条件BDD获取当前状态
                DdNode *tmp4 = Cudd_bddAndAbstract(manager, successor, t, next_state_cube);
                Cudd_Ref(tmp4);
                Cudd_RecursiveDeref(manager, curr);
                curr = tmp4;
                Cudd_RecursiveDeref(manager, t);
                Cudd_RecursiveDeref(manager, successor);
            }

            ce = Cudd_bddAnd(manager, curr, init_states);

            if (ce == Cudd_ReadLogicZero(manager))
            {
                std::cout << "=====================" << std::endl;
                std::cout << "反例为false" << std::endl;
                std::cout << "=====================" << std::endl;
                return false;
            }else{
                std::cout << "找到反例" << std::endl;
                return true;
            }

            
        }
    }
    // 若执行完候选规划中的所有动作，检查最终状态是否满足目标
    if (bdd_entailed(manager, curr, b_goal_state))
    {
        std::cout << "最终状态满足目标, 当前规划有效" << std::endl;
        return false;
    }
    else
    {
        std::cout << "最终状态不满足目标，逆推寻找反例：" << std::endl;
        // (1) 获取不满足的部分,这里是不满足目标状态的那一部分状态
        DdNode *stav = Cudd_bddAnd(manager, curr, b_goal_state);
        Cudd_Ref(stav);
        DdNode *tmp3 = Cudd_bddAnd(manager, Cudd_Not(stav), curr);
        Cudd_RecursiveDeref(manager,stav);
        Cudd_Ref(tmp3);
        Cudd_RecursiveDeref(manager, curr);
        curr = tmp3;

        for (std::vector<const Action *>::reverse_iterator a_it = reverse_action.rbegin(); a_it != reverse_action.rend();++a_it)
        {
            const Action *r_action = *a_it;
            DdNode *t = groundActionDD(*r_action);
            Cudd_Ref(t);

            // （2）将不满足的部分转化成后继状态表示
            DdNode *successor = Cudd_bddVarMap(manager, curr);
            Cudd_Ref(successor);

            //（3）根据后继状态表示以及动作BDD获取当前状态
            DdNode *tmp4 = Cudd_bddAndAbstract(manager, successor, t, next_state_cube);
            Cudd_RecursiveDeref(manager, curr);
            curr = tmp4;
            Cudd_RecursiveDeref(manager, t);
            Cudd_RecursiveDeref(manager, successor);
        }

        ce = Cudd_bddAnd(manager, curr, init_states);

        if (ce == Cudd_ReadLogicZero(manager))
        {
            std::cout << "=====================" << std::endl;
            std::cout << "反例为false" << std::endl;
            std::cout << "=====================" << std::endl;
            return false;
        }else{
            std::cout << "找到反例" << std::endl;
            return true;
        }
        
    } 
}