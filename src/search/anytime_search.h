#ifndef ANYTIME_SEARCH_H
#define ANYTIME_SEARCH_H

#include "search.h"

//#include "globals.h"						// PointerHasher
//#include "graph.h"
#include <set>
#include <time.h>               // clock
#include <iostream>
#include <fstream>

// Forward Declarations
class StateNode;

// Anytime Search class
class AnytimeSearch : public Search{
protected:
	AnytimeSearch(SearchType inheriting_type);

public:
	virtual ~AnytimeSearch();

	void search();

protected:
	virtual bool solve(StateNode* start) = 0;

	void valueIteration();
	void writePS();

	clock_t startTime;
	std::ofstream planSuccessCSV;
	std::ofstream planSuccessTXT;
};

#endif  // ANYTIME_SEARCH_H
