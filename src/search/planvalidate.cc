#include "planvalidate.h"

#include "graph_wrapper.h"  // -> formula_bdd
#include "lao_wrapper.h" // -> progress()
#include "dd.h"  // -> bdd_entailed()
#include "globals.h"
#include "astar.h"
#include "lug.h"

using namespace std;

Planvalidate::Planvalidate(){}

/**
 * 返回true，说明查找反例成功
 * 返回false，说明反例不存在
*/
bool Planvalidate::planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode* ce){
    DdNode *tmp = formula_bdd(problem->init_formula(),false); // 🐶获取problem的初始状态
    list<DdNode*> init_states; // 🐶多个可能的初始状态
    // getHeuristic  --> double getCardinality(DdNode* d)
    double x = getCardinality(tmp);
    pickKRandomWorlds(tmp, x, &init_states); // 🐶且会自动打印printBDD(*init_it)

    // 利用Cudd And neg(初始化取的那个可能的初始状态)--》 暂时先不考虑了

    list<DdNode *>::iterator s_it = init_states.begin();
    // 遍历初始状态
    for (list<DdNode *>::iterator s_it = init_states.begin(); s_it != init_states.end(); s_it++)
    {
        DdNode *sta = *s_it;
        DdNode *back_up = *s_it;
        Cudd_Ref(back_up);
        // 遍历candidateplan
        for (std::vector<const Action *>::iterator a_it = candidateplan.begin(); a_it != candidateplan.end(); a_it++)
        {
            DdNode *preBdd = action_preconds.find(*a_it)->second;  // 🐶
            if (bdd_entailed(manager, sta, preBdd))
            {
                cout << "蕴涵成功" << endl;
                DdNode *successor = progress(sta, *a_it); // 🐶
                Cudd_RecursiveDeref(manager, sta);
                sta = successor;
                printBDD(sta);
            }
            else
            {
                ce = back_up;  // 将当前状态赋给ce
                std::cout << "状态不蕴涵当前动作条件，找到反例" << std::endl;
                return true;
            }
        }
        // 判断最后一个状态节点是不是目标状态
            if (bdd_entailed(manager, sta, b_goal_state))
            {
                std::cout << "当前初始状态不是反例，继续遍历下一个初始状态" << std::endl;
                continue;
            }
            else
            {
                ce = back_up;
                std::cout << "最终状态不是目标状态，找到反例" << std::endl;
                return false;
            }
    }
    if (ce == NULL)
    {
        cout << "反例为空，未找到反例" << endl;
        return false;
    }
    // // 取出的其他可能的初始状态还需要与初始化时取出的不同
    // cout << "planvalidate调用成功" << endl; // 🐶
    // return false;  // --> main_counter中调用为!planvalidate()，打印规划

}