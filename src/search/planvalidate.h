#ifndef PLANVALIDATE_H
#define PLANVALIDATE_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planvalidate{
public:
    Planvalidate();

    bool planvalidate(DdNode *&ce);

    DdNode *backwardToInitial(DdNode *, DdNode*);

    DdNode *getKSample(DdNode*);

protected:
    StateNode *next;
    std::vector<const Action *> reverse_action;
};

#endif  // PLANVALIDATE_H