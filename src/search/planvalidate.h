#ifndef PLANVALIDATE_H
#define PLANVALIDATE_H

#include "graph.h"    // std::vector
#include "actions.h"	// Action*

class Planvalidate{
public:
    Planvalidate();

    // bool planvalidate(const Problem* problem, std::vector<const Action*> &candplan, DdNode *&ce);
    bool planvalidate(std::vector<const Action*> &candplan, DdNode *&ce);
    void initial_states(const Problem* problem, std::list<DdNode *> &is);  // 提取所有的初始状态集

protected:
    StateNode *next;
};

#endif  // PLANVALIDATE_H