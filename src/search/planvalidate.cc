#include "planvalidate.h"

#include "graph_wrapper.h" 
#include "lao_wrapper.h" // -> progress()
#include "dd.h"  // -> bdd_entailed()
#include "globals.h"


using namespace std;

Planvalidate::Planvalidate(){}

/**
 * 返回true，说明查找反例成功
 * 返回false，说明反例不存在
*/
bool Planvalidate::planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode* ce){
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