#ifndef PLANVALIDATE_H
#define PLANVALIDATE_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planvalidate{
public:
    Planvalidate();

    bool planvalidate(DdNode *&ce);

    // DdNode* regress(pair<const Action *const, DdNode *> *a, DdNode *parent);
    // DdNode *regress(DdNode *image, DdNode *parent);

protected:
    StateNode *next;
};

#endif  // PLANVALIDATE_H