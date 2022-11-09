#ifndef HOP_ADVANCE_H
#define HOP_ADVANCE_H

#include "graph.h"									// StateComparator, StateNode
#include <set>

class HAStateNode{
public:
	HAStateNode(StateNode* state, int horizon){
		this->state = state;
		this->horizon = horizon;
	}

	StateNode* state;
	int horizon;
};

class HAStateComparator{
public:
	HAStateComparator() {}
  bool operator() (const HAStateNode& lhs, const HAStateNode& rhs) const;
};

class HOPAdvance{
public:
  HOPAdvance(ActionNode* start);
  ~HOPAdvance();

	bool step();
	double getQuality();
	int getRand();

protected:
	std::set<HAStateNode, HAStateComparator> open;
	std::set<HAStateNode, HAStateComparator> closed;
	HAStateNode next;
	bool done;
	bool first;
	int curRand;
	StateNode* start;
	int hop_horizon;
};

#endif  // HOP_ADVANCE_H
