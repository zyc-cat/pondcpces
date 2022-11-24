#ifndef PLANVAL_H
#define PLANVAL_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planval{
public:
    Planval();

    bool planvalidate1(const Problem* problem, std::vector<const Action*> &candplan, DdNode* ce);

protected:
    StateNode *node; // 当前结点
};

#endif  // PLANVAL_H