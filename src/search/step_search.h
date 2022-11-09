#ifndef STEP_SEARCH_H
#define STEP_SEARCH_H

#include "search.h"

// Forward Declarations
class StateNode;
class ActionNode;

class StepSearch : public Search{
protected:
	StepSearch(SearchType inheriting_type);
public:
	virtual ~StepSearch();

	virtual void search();// override

	virtual void setup(StateNode* start);// 初始化
	virtual bool step() = 0;// 不同更新一个结点的方法
	virtual double getQuality() = 0;
	virtual void store();// 存储当前sta的最佳action
	virtual void recall();// 取出恢复sta的最佳action
	// 应该表示某个状态完成了搜索
	virtual void commit();
	virtual void cleanup();

protected:
	StateNode* start;
	ActionNode* bestFirstAction;
};

#endif	// STEP_SEARCH_H
