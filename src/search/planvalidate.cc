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
    if (!init_states)
    {
        std::cout << "===================================" << std::endl;
        std::cout << "初始状态集合为空,反例不存在" << std::endl;
        return false;
    }

    DdNode *sta = init_states;  // 初始化当前状态变量sta
    Cudd_Ref(sta);
    std::vector<const Action *> reverse_action; // 存放逆推时需要的动作(已经执行的动作)

    // 执行候选规划
    for (std::vector<const Action *>::iterator act_it = candidateplan.begin(); act_it != candidateplan.end(); act_it++)
    {
        const Action *action = *act_it;
        reverse_action.push_back(action);
        DdNode *preBdd = action_preconds.find(*act_it)->second;
        // 检查当前状态是否蕴涵动作前提条件
        if (bdd_entailed(manager, sta, preBdd))
        {
            cout << "当前状态变量蕴涵当前动作前提条件" << endl;
            // 计算当前状态的后继状态
            DdNode *successor = progress(sta, *act_it);
            Cudd_Ref(successor);
            Cudd_RecursiveDeref(manager, sta);
            sta = successor;
        }
        else
        {
            // 当前状态变量sta不满足动作前提条件，通过逆推数组里面的动作
            std::cout << "当前状态变量不蕴涵当前动作前提条件，逆推寻找反例：" << std::endl;
            for (std::vector<const Action *>::iterator a_it = reverse_action.begin(); a_it != reverse_action.end(); a_it++)
            {
                const Action *r_action = *a_it;
                DdNode *prec = action_preconds.find(*a_it)->second;

                // 获取不满足动作前提条件的那部分状态
                DdNode *stav = Cudd_bddAnd(manager, sta, prec);
                Cudd_Ref(stav);
                DdNode *tmp1 = Cudd_bddAnd(manager, Cudd_Not(stav), sta);
                Cudd_Ref(tmp1);
                Cudd_RecursiveDeref(manager, sta);
                Cudd_RecursiveDeref(manager, stav);
                sta = tmp1;
                std::cout << "不满足前提条件的那部分状态：" << std::endl;
                printBDD(sta);  // 这里得到BDD为false ???

                // 根据当前动作的bdd, 得到当前状态变量的前一个状态变量
                DdNode *tmp2 = Cudd_bddAndAbstract(manager, sta, prec, current_state_cube);
                Cudd_Ref(tmp2);
                sta = Cudd_bddVarMap(manager, tmp2);
                Cudd_RecursiveDeref(manager, tmp2);
                printBDD(sta);
            }
            // 将最终得到的状态赋给反例
            ce = sta;
            return true;
        }
    }
    // 若执行完候选规划中的所有动作，检查最终状态是否满足目标
    if (bdd_entailed(manager, sta, b_goal_state))
    {
        std::cout << "打印当前状态sta" << std::endl;
        printBDD(sta);
        std::cout << "最终状态满足目标, 初始状态集合不存在反例,当前规划有效" << std::endl;
        return false;
    }
    else
    {
        // 初始状态集合中存在反例，从最终状态逆推寻找不满足的初始状态，即反例
        std::cout << "最终状态不满足目标，逆推寻找反例：" << std::endl;
        for (std::vector<const Action *>::iterator a_it = reverse_action.begin(); a_it != reverse_action.end(); a_it++)
        {
            // 获取动作
            const Action *r_action = *a_it;
            // 获取动作的前提条件BDD
            DdNode *prec = action_preconds.find(*a_it)->second;
        
            DdNode *stav = Cudd_bddAnd(manager, sta, prec);
            Cudd_Ref(stav);
            DdNode *tmp3 = Cudd_bddAnd(manager, Cudd_Not(stav), sta);
            Cudd_Ref(tmp3);
            Cudd_RecursiveDeref(manager, sta);
            Cudd_RecursiveDeref(manager, stav);
            sta = tmp3;
            std::cout << "不满足前提条件的那部分状态：" << std::endl;
            printBDD(sta);  // 这里得到BDD为false ???

            // 根据当前动作的bdd, 得到当前状态变量的前一个状态变量
            DdNode *tmp4 = Cudd_bddAndAbstract(manager, sta, prec, current_state_cube);
            Cudd_Ref(tmp4);
            sta = Cudd_bddVarMap(manager, tmp4);
            Cudd_RecursiveDeref(manager, tmp4);
            printBDD(sta);  
        }
        // 将最终的状态变量赋给反例ce
        ce = sta;
        printBDD(ce);
        return true;
    } 

    if (NULL == ce)
    {
        // 未找到反例，退出
        return false;
    }
}