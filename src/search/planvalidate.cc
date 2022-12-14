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
bool Planvalidate::planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode *&ce){
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
        // 备份初始状态，以便计算到不满足动作条件蕴涵或者最终状态不是目标状态时，能够得到该轮循环中的初始状态(即反例)
        DdNode *back_up = *s_it;  
        Cudd_Ref(back_up);  // 引用计数
        // 遍历candidateplan
        for (std::vector<const Action *>::iterator a_it = candidateplan.begin(); a_it != candidateplan.end(); a_it++)
        {
            // 打印当前规划, 看是否有效，到底是否全部动作执行完毕才判断的最后一个节点是否是目标状态
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
                std::cout << "当前后继状态为：" << std::endl;
                printBDD(sta); // 打印当前蕴涵成功时的后继状态
            }
            else
            {
                ce = back_up;  // 将当前状态赋给ce
                std::cout << "当前状态不蕴涵当前动作前提条件，找到反例，打印反例：" << std::endl;
                printBDD(ce);  // 打印反例，看反例是否找到的是对的
                return true;
            }
        }
        // 判断最后一个状态节点是不是目标状态
        if (bdd_entailed(manager, sta, b_goal_state))
        {
            // 最后一个状态节点是目标状态，那么说明未找到反例，继续
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
            printBDD(ce);  // 如果最终状态不是目标状态，那么当前的初始状态就是反例，将其打印看是否正确
            std::cout << "反例打印完毕" << endl;
            // return false;  // 这里应该是return true 找到了反例
            return true;
        }
    }
    if(init_states.empty()){
        cout << "采样的初始状态集合为空,反例为空" << endl;
        return false;
    }
    // 如果所有初始状态都检验了一遍，反例还是没有找到，就结束
    if (ce == NULL)
    {
        cout << "反例为空，未找到反例" << endl;
        return false;
    }
}


// 返回true，说明查找反例成功
// 返回false，说明反例不存在
// bool Planvalidate::planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode *&ce){
//     DdNode *tmp = formula_bdd(problem->init_formula(),false); // 🐶获取problem的初始状态
//     list<DdNode*> init_states; // 🐶多个可能的初始状态
//     // getHeuristic  --> double getCardinality(DdNode* d)
//     double x = getCardinality(tmp);
//     pickKRandomWorlds(tmp, x, &init_states); // 🐶且会自动打印printBDD(*init_it)  
//     // 利用Cudd And neg(初始化取的那个可能的初始状态)--》 暂时先不考虑了
//     list<DdNode *>::iterator s_it = init_states.begin();
//     // 遍历初始状态
//     for (list<DdNode *>::iterator s_it = init_states.begin(); s_it != init_states.end(); s_it++)
//     {
//         DdNode *sta = *s_it;
//         // 备份初始状态，以便计算到不满足动作条件蕴涵或者最终状态不是目标状态时，能够得到该轮循环中的初始状态(即反例)
//         DdNode *back_up = *s_it;  
//         Cudd_Ref(back_up);  // 引用计数
//         // 遍历candidateplan
//         for (std::vector<const Action *>::iterator a_it = candidateplan.begin(); a_it != candidateplan.end(); a_it++)
//         {
//             // 打印当前规划, 看是否有效，到底是否全部动作执行完毕才判断的最后一个节点是否是目标状态
//             std::cout << "打印candidateplan中的动作" << std::endl;
//             (*a_it)->print(std::cout, my_problem->terms());
//             std::cout << "\n";
//             DdNode *preBdd = action_preconds.find(*a_it)->second;
//             if (bdd_entailed(manager, sta, preBdd))
//             {
//                 std::cout << "打印当前未执行本轮动作时的状态BDD" << std::endl;
//                 printBDD(sta); 
//                 cout << "当前状态蕴涵当前动作前提条件" << endl;
//                 DdNode *successor = progress(sta, *a_it);
//                 Cudd_RecursiveDeref(manager, sta);
//                 sta = successor;
//                 std::cout << "当前后继状态为：" << std::endl;
//                 printBDD(sta); // 打印当前蕴涵成功时的后继状态
//             }
//             else
//             {
//                 ce = back_up;  // 将当前状态赋给ce
//                 std::cout << "当前状态不蕴涵当前动作前提条件，找到反例，打印反例：" << std::endl;
//                 printBDD(ce);  // 打印反例，看反例是否找到的是对的
//                 return true;
//             }
//         }
//         // 判断最后一个状态节点是不是目标状态
//         if (bdd_entailed(manager, sta, b_goal_state))
//         {
//             // 最后一个状态节点是目标状态，那么说明未找到反例，继续
//             std::cout << "打印最后一个状态节点" << std::endl;
//             printBDD(sta);
//             std::cout << "当前初始状态不是反例，继续遍历下一个初始状态" << std::endl;
//             continue;
//         } else
//         {
//             ce = back_up;
//             std::cout << "打印最终状态" << std::endl;
//             printBDD(sta);
//             std::cout << "最终状态不是目标状态，找到反例" << std::endl;
//             printBDD(ce);  // 如果最终状态不是目标状态，那么当前的初始状态就是反例，将其打印看是否正确
//             std::cout << "反例打印完毕" << endl;
//             // return false;  // 这里应该是return true 找到了反例
//             return true;
//         }
//     }
//     if(init_states.empty()){
//         cout << "采样的初始状态集合为空,反例为空" << endl;
//         return false;
//     }
//     // 如果所有初始状态都检验了一遍，反例还是没有找到，就结束
//     if (ce == NULL)
//     {
//         cout << "反例为空，未找到反例" << endl;
//         return false;
//     }
//     // // 取出的其他可能的初始状态还需要与初始化时取出的不同
//     // return false;  // --> main_counter中调用为!planvalidate()，打印规划
// }