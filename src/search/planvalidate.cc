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

    // TODO: 不满足目标条件，逆推寻找反例

    // 执行候选规划，
    for (std::vector<const Action *>::iterator act_it = candidateplan.begin(); act_it != candidateplan.end(); act_it++)
    {
        const Action *action = *act_it;
        reverse_action.push_back(action);
        DdNode *preBdd = action_preconds.find(*act_it)->second;
        // 检查当前状态是否蕴涵动作前提条件
        if (bdd_entailed(manager, sta, preBdd))
        {
            cout << "当前状态变量蕴涵当前动作前提条件" << endl;
            DdNode *successor = progress(sta, *act_it);
            Cudd_Ref(successor);
            Cudd_RecursiveDeref(manager, sta);
            sta = successor;
        }
        else
        {

            //（1）获取不满足的那部分,这里是不满足当前动作的前提条件
            DdNode *stav = Cudd_bddAnd(manager, sta, preBdd);
            Cudd_Ref(stav);
            DdNode *tmp1 = Cudd_bddAnd(manager, Cudd_Not(stav), sta);
            Cudd_Ref(tmp1);
            // Cudd_RecursiveDeref(manager, sta);
            Cudd_RecursiveDeref(manager, stav);
            sta = tmp1; 
            std::cout << "不满足前提条件的那部分状态：" << std::endl;
            printBDD(sta);
            
            std::cout << "当前状态变量不蕴涵当前动作前提条件，逆推寻找反例：" << std::endl;
            for (std::vector<const Action *>::reverse_iterator a_it = reverse_action.rbegin(); a_it != reverse_action.rend();++a_it)
            {
                // TODO: 对不满足前提条件的状态进行逆推
                const Action *r_action = *a_it;
                // 获取动作的BDD
                DdNode *t = groundActionDD(*r_action);

                // （2）将不满足的部分转化成后继状态表示
                DdNode *successor = Cudd_bddVarMap(manager, sta);
                Cudd_Ref(successor);

                // 计算后继状态的cube
                DdNode **successor_state_vars = new DdNode *[num_alt_acts];
                for (int i = 0; i < num_alt_facts; i++)
                {
                    successor_state_vars[i] = Cudd_bddIthVar(manager, 2 * i + 1);
                    Cudd_Ref(successor_state_vars[i]);
                    Cudd_bddSetPairIndex(manager, 2 * i, 2 * i + 1);
                }
                DdNode *successor_state_cube = Cudd_bddComputeCube(manager, successor_state_vars, 0, num_alt_acts);
                Cudd_Ref(successor_state_cube);

                // （3）根据后继状态表示以及动作前提条件BDD获取当前状态
                DdNode *tmp4 = Cudd_bddAndAbstract(manager, successor, t, successor_state_cube);
                Cudd_Ref(tmp4);
                sta = tmp4;
                std::cout << "打印逆推时的当前变量" << std::endl;
                printBDD(sta);   
            }
            ce = sta;
            return true;
        }
    }
    // 若执行完候选规划中的所有动作，检查最终状态是否满足目标
    if (bdd_entailed(manager, sta, b_goal_state))
    {
        std::cout << "打印当前状态sta" << std::endl;
        printBDD(sta);
        std::cout << "最终状态满足目标, 当前规划有效" << std::endl;
        return false;
    }
    else
    {
        // TODO: 对不满足目标状态的状态进行逆推
        std::cout << "最终状态不满足目标，逆推寻找反例：" << std::endl;
        // (1) 获取不满足的部分,这里是不满足目标状态的那一部分状态
        DdNode *stav = Cudd_bddAnd(manager, sta, b_goal_state);
        Cudd_Ref(stav);
        DdNode *tmp3 = Cudd_bddAnd(manager, Cudd_Not(stav), sta);
        Cudd_Ref(tmp3);
        // Cudd_RecursiveDeref(manager, sta);
        Cudd_RecursiveDeref(manager, stav);
        sta = tmp3;
        std::cout << "不满足目标状态的那部分状态：" << std::endl;
        printBDD(sta);

        for (std::vector<const Action *>::reverse_iterator a_it = reverse_action.rbegin(); a_it != reverse_action.rend();++a_it)
        {
            const Action *r_action = *a_it;
            // DdNode *prec = action_preconds.find(*a_it)->second;
            // 获取动作的BDD
            DdNode *t = groundActionDD(*r_action);

            // （2）将不满足的部分转化成后继状态表示
            DdNode *successor = Cudd_bddVarMap(manager, sta);
            Cudd_Ref(successor);


            // 计算后继状态的cube
            DdNode **successor_state_vars = new DdNode *[num_alt_acts];
            for (int i = 0; i < num_alt_facts; i++)
            {
                successor_state_vars[i] = Cudd_bddIthVar(manager, 2 * i + 1);
                Cudd_Ref(successor_state_vars[i]);
                Cudd_bddSetPairIndex(manager, 2 * i, 2 * i + 1);
            }
            DdNode *successor_state_cube = Cudd_bddComputeCube(manager, successor_state_vars, 0, num_alt_acts);
            Cudd_Ref(successor_state_cube);

            //（3）根据后继状态表示以及动作BDD获取当前状态
            DdNode *tmp4 = Cudd_bddAndAbstract(manager, successor, t, successor_state_cube);
            Cudd_Ref(tmp4);
            sta = tmp4;
            std::cout << "打印逆推时的当前变量" << std::endl;
            printBDD(sta); 
        }
        
        ce = sta;
        std::cout << "打印反例：" << std::endl;
        printBDD(ce);
        return true;
    } 
    if (NULL == ce)
    {
        // 未找到反例，退出
        return false;
    }
}