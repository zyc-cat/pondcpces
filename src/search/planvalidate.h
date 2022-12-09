#ifndef PLANVALIDATE_H
#define PLANVALIDATE_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planvalidate{
public:
    Planvalidate();

    bool planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode* ce);
    // bool planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode& ce);
    
protected:
    StateNode *next;
};

#endif  // PLANVALIDATE_H