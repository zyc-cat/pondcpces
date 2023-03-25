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

    DdNode *curr = init_states; // 当前状态变量  
    Cudd_Ref(curr);
    std::vector<const Action *> reverse_action;

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

    for (std::vector<const Action *>::iterator act_it = candidateplan.begin(); act_it != candidateplan.end(); act_it++)
    {
        const Action *action = *act_it;
        reverse_action.push_back(action);
        DdNode *preBdd = action_preconds.find(*act_it)->second;
        Cudd_Ref(preBdd);
        if (bdd_entailed(manager, curr, preBdd))
        {
            cout << "当前状态变量蕴涵当前动作前提条件" << endl;
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
            std::cout << "不满足前提条件的那部分状态：" << std::endl;
            printBDD(curr);
            
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
                DdNode *tmp4 = Cudd_bddAndAbstract(manager, successor, t, successor_state_cube);
                Cudd_Ref(tmp4);
                Cudd_RecursiveDeref(manager, curr);
                curr = tmp4;
                // std::cout << "打印逆推时的当前变量" << std::endl;
                // printBDD(curr);   
            }

            Cudd_Ref(init_states);
            ce = Cudd_bddAnd(manager, curr, init_states);
            std::cout << "逆推后去除不满足初始状态部分得到的反例：" << std::endl;
            printBDD(ce);         
            //  TODO: 为什么逆推为ce得到false
            if (ce == Cudd_ReadLogicZero(manager))
            {
                printBDD(ce);
                return false;
            }

            return true;
        }
    }
    // 若执行完候选规划中的所有动作，检查最终状态是否满足目标
    if (bdd_entailed(manager, curr, b_goal_state))
    {
        // std::cout << "打印当前状态curr" << std::endl;
        // printBDD(curr);
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
        std::cout << "不满足目标状态的那部分状态：" << std::endl;
        printBDD(curr);

        for (std::vector<const Action *>::reverse_iterator a_it = reverse_action.rbegin(); a_it != reverse_action.rend();++a_it)
        {
            const Action *r_action = *a_it;
            DdNode *t = groundActionDD(*r_action);
            Cudd_Ref(t);

            // （2）将不满足的部分转化成后继状态表示
            DdNode *successor = Cudd_bddVarMap(manager, curr);
            Cudd_Ref(successor);

            //（3）根据后继状态表示以及动作BDD获取当前状态
            DdNode *tmp4 = Cudd_bddAndAbstract(manager, successor, t, successor_state_cube);
            Cudd_Ref(tmp4);
            Cudd_RecursiveDeref(manager, curr);
            curr = tmp4;
            std::cout << "打印逆推时的当前变量" << std::endl;
            printBDD(curr); 
        }

        Cudd_Ref(init_states);
        ce = Cudd_bddAnd(manager, curr, init_states);
        std::cout << "逆推后去除不满足初始状态部分得到的反例：" << std::endl;
        printBDD(ce);

        // TODO: 为什么逆推为ce得到false
        if (ce == Cudd_ReadLogicZero(manager))
        {
            printBDD(ce);
            return false;
        }
        return true;
    } 
    if (NULL == ce)
    {
        // 未找到反例，退出
        return false;
    }
}