#ifndef ASTAR_H
#define ASTAR_H

#include "step_search.h"

#include "graph.h"									// StateComparator, StateNode, ActionNode
#include <set>
#include <deque>
#include <utility>									// pair

class AStar : public StepSearch{
public:
  AStar();

	void setup(StateNode* start);
	bool step();
	double getQuality();// 忽略，conformnat
	/**
	 * 更新当前节点及前驱节点都搜索完成，not used
	 */
	void commit();// 从next结点开始，向上传递给祖先结点更新solve情况
	void cleanup();// 情况状态结点的最佳动作和解决情况

	void setBestAct(StateNode* state);

	void printBestPlan();

	void planlist(std::vector<const Action*> &candplan);

protected:
	std::set<StateNode*> closed;
	std::set<StateNode*, StateComparator> open;// 通过启发式函数进行排序
	StateNode* next;// 下一个处理结点
	bool first;
};

#endif  // ASTAR_H
