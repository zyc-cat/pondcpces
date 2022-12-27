#include "planvalidate.h"

#include "graph_wrapper.h" 
#include "lao_wrapper.h" 
#include "dd.h" 
#include "globals.h"
#include "astar.h"
#include "lug.h"

using namespace std;

Planvalidate::Planvalidate(){}

void Planvalidate::initial_states(const Problem* problem, std::list<DdNode *> &is){
    DdNode *tmp = formula_bdd(problem->init_formula(),false);
    double x = getCardinality(tmp);
    pickKRandomWorlds(tmp, x, &is); 
}

/**
 * 返回true，说明查找反例成功
 * 返回false，说明反例不存在
*/
bool Planvalidate::planvalidate(std::vector<const Action*> &candplan, DdNode *&ce){
    if (init_states.empty())
    {
        std::cout << "采样的初始状态集合为空,反例不可能存在" << std::endl;
        return false;
    }

    for (std::list<DdNode *>::iterator s_it = init_states.begin(); s_it != init_states.end(); s_it++)
    {
        DdNode *sta = *s_it; 
        DdNode *back_up = *s_it; 
        Cudd_Ref(back_up); 
        for (std::vector<const Action *>::iterator a_it = candidateplan.begin(); a_it != candidateplan.end(); a_it++)
        {
            std::cout << "打印candidateplan中的动作" << std::endl;
            (*a_it)->print(std::cout, my_problem->terms());
            std::cout << "\n";

            DdNode *preBdd = action_preconds.find(*a_it)->second;
            if (bdd_entailed(manager, sta, preBdd))
            {
                std::cout << "打印当前未执行本轮动作时的状态BDD" << std::endl;
                printBDD(sta); 
                cout << "当前状态蕴涵当前动作前提条件" << endl;
                DdNode *successor = progress(sta, *a_it);
                Cudd_RecursiveDeref(manager, sta);
                sta = successor;
            }
            else
            {
                ce = back_up; 
                std::cout << "当前状态不蕴涵当前动作前提条件，找到反例，打印反例：" << std::endl;
                printBDD(ce);
                return true;
            }
        }

        if (bdd_entailed(manager, sta, b_goal_state))
        {
            std::cout << "打印最后一个状态节点" << std::endl;
            printBDD(sta);
            std::cout << "当前初始状态不是反例，继续遍历下一个初始状态" << std::endl;
            continue;
        } else
        {
            ce = back_up;
            std::cout << "打印最终状态" << std::endl;
            printBDD(sta);
            std::cout << "最终状态不是目标状态，找到反例" << std::endl;
            printBDD(ce); 
            std::cout << "反例打印完毕" << endl;
            return true;
        }
    }
    if (ce == NULL)
    {
        cout << "遍历完所有的采样初始状态，仍未找到反例，退出查找反例" << endl;
        return false;
    }
}