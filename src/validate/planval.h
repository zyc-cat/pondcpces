#ifndef PLANVAL_H
#define PLANVAL_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planval{
public:
    Planval();

    bool planval(const Problem* problem, std::vector<const Action*> &candplan, StateNode* ce);

protected:
    StateNode *next; // 当前结点
};

#endif  // PLANVAL_H