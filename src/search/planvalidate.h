#ifndef PLANVALIDATE_H
#define PLANVALIDATE_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planvalidate{
public:
    Planvalidate();

    bool planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode* ce);
    void init(std::list<DdNode*> init_states, std::list<StateNode*>* s);

protected:
    StateNode *next;
};

#endif  // PLANVALIDATE_H