#include "planval.h"

#include "graph_wrapper.h" 
#include "lao_wrapper.h" // -> progress()
#include "dd.h"  // -> bdd_entailed()
#include "globals.h"


using namespace std;

Planval::Planval(){}

/**
 * 返回true，说明查找反例成功
 * 返回false，说明反例不存在
*/
// bool Planval::planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode* ce){
//     /**
//      * 1. void pickKRandomWorlds(DdNode* dd, int k, list<DdNode*>* worlds)获取所有分离的可能的初始状态
//      *    意思就是每个可能的初始状态可以状态可以从worlds中单独取出来)
//      *                DdNode* dd，当前Problem的初始状态(总) --> 怎么表示呢
//      *                list<DdNode *> worlds;
//      *                pickKRandomWorlds(tmp1, 1, &worlds);
//      * 
//      * 2. 遍历循环worlds中的每个初始状态：
//      *      2.1 遍历候选规划中的动作
//      *          1）判断当前状态是否蕴含(entail)当前动作的前提条件
//      *              首先提取当前候选规划的动作的前提条件
//      *              DdNode *preBdd = action_preconds.find(action->act)->second;
//      *              std::map<const Action*, DdNode*> action_preconds; --> 全局变量
//      *              int bdd_entailed(DdManager * dd, DdNode * f, DdNode * g)
//      *              - 如果蕴含，则执行DdNode *successor = progress(next->dd, action->act);计算后继状态，并将successor赋给next，继续判断
//      *                      (next是当前状态，action是从候选规划中取出来的当前动作)
//      *              - 若不蕴含，则返回本次循环的初始状态(反例)，结束外循环
//      *          2）基于当前初始状态，遍历完候选规划的所有动作都没有问题，就使用next->isGoal()判断当前状态next是否是目标状态
//      *              - 如果是，则结束内循环，开始下一个可能的初始状态进行循环
//      *              - 若不是，则返回本次循环的初始状态，结束外循环
//      * 3. 如果遍历完所有的初始状态都没有找到反例，那么返回false
//      * 4. 在主函数判断该函数的真值
//      *      - 如果为true,就将反例与当前初始状态b_initial_state合并进入搜索
//      *      - 如果为false,就直接打印输出当前candidateplan                
//     */

/**
 * 返回true，说明查找反例成功
 * 返回false，说明反例不存在
*/
bool Planval::planvalidate1(const Problem* problem, std::vector<const Action*> &candplan, DdNode* ce){
    DdNode* tmp = formula_bdd(problem->init_formula(),false);  // 获取problem的初始状态
    list<DdNode *> worlds;  // 存放多个可能的初始状态
    pickKRandomWorlds(tmp, problem->init_atoms().size(), &worlds);
    // 外层循环(遍历可能的初始状态)
    for (list<DdNode *>::iterator init_it = worlds.begin(); init_it != worlds.end(); init_it ++){
        // 内层循环(candidateplan中的动作)
        for (std::vector<const Action *>::iterator act_it = candidateplan.begin(); act_it != candidateplan.end(); act_it++){
            DdNode *preBdd = action_preconds.find(*act_it)->second;
            if (bdd_entailed(manager, *init_it, preBdd)){
                DdNode *ddnode = node->dd; // 当前Dd节点
                // DdNode *successor = ;
                ddnode = progress(ddnode, *act_it);
            }else{
                ce = *init_it;  // 将当前初始状态赋给反例
                break;  // 找到反例，结束最外层循环
            }
        }
        if (node->isGoal()){
            continue;  // 当前初始状态不是反例，继续遍历下一个初始状态
        }else{
            ce = *init_it;
            break;
        }
    }
    if (ce == NULL){
        cout << "No counterexample found" << endl;
        return false;
    }

    cout << "Found the counterexample" << endl;
    return true;
}