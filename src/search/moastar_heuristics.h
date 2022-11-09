/*
 * moastar_heuristics.h
 *
 *  Created on: Nov 4, 2008
 *      Author: danbryce
 */

#ifndef MOASTAR_HEURISTICS_H_
#define MOASTAR_HEURISTICS_H_

#include "movalfn.h"
#include <list>

void computeMAOStarHeuristics(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		MOValueFunction *NDpoints);

#endif /* MOASTAR_HEURISTICS_H_ */
