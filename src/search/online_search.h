#ifndef ONLINE_SEARCH_H
#define ONLINE_SEARCH_H

#include "search.h"

#include <string>

// Forward Declarations
class ActionNode;
class StepSearch;

class OnlineSearch : public Search{
public:
	OnlineSearch(StepSearch* search = NULL, int hLimit = -1);
	virtual ~OnlineSearch();

	void search();

protected:
	StepSearch* step_search;

	std::string readString(int fd);
	double readNum(int fd);
	void sendAction(int fd, ActionNode* action);

	int hLimit;
};

#endif	// ONLINE_SEARCH_H
