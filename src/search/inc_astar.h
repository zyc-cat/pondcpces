#ifndef INC_ASTAR_H
#define INC_ASTAR_H

#include "astar.h"

#include "graph.h"									// StateComparator, StateNode, ActionNode
#include <set>
#include <deque>
#include <utility>									// pair
#include "inc_astar.h"
#include "astar.h"

class Incremental_AStar : public AStar{
public:
    Incremental_AStar();

	void setup(StateNode* start);
	bool step();
	void cleanup();
    void search() override;
    void updateOpenAndClose(DdNode *);

protected:
    void updateLists(StateNode* parent, StateNode* child, ActionNode* actNode);
    // 上一轮迭代的情况
    std::set<StateNode *> pclosed; 
	std::set<StateNode*, StateComparator> popen;
};

#endif  // INC_ASTAR_H
