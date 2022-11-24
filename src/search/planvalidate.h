#ifndef PLANVALIDATE_H
#define PLANVALIDATE_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planvalidate{
public:
    Planvalidate();

    bool planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode* ce);

protected:
    StateNode *node; // 当前结点
};

#endif  // PLANVAL_H