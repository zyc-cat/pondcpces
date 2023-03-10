#ifndef PLANVALIDATE_H
#define PLANVALIDATE_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planvalidate{
public:
    Planvalidate();

    bool planvalidate(DdNode *&ce);

protected:
    StateNode *next;
};

#endif  // PLANVALIDATE_H