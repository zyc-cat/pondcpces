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

    DdNode *getTerm(DdNode *);

    int getRandomSampleTime();

protected:
    StateNode *next;
    std::vector<const Action *> reverse_action;
    int random_sample_time; // 随机采样的次数
};

#endif  // PLANVALIDATE_H