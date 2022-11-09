#include "moastar_heuristics.h"
#include "globals.h"
#include "lug.h"
#include "math.h"

void computeUniformGrid(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		int gridPoints){



	double goalSat = goal_threshold;
	int MGS = NUMBER_OF_MGS;

	double gridSpacing = 1.0/(gridPoints);
	double gridValue = gridSpacing;

	for(;gridValue <= 1.0; gridValue += gridSpacing){
		std::list<StateNode*> states;
		states.push_back(successor);
		getHeuristic(&states, i->stateNode, 0);


		if(LUG_FOR==SPACE)
			goal_threshold = gridValue;//1.0;
		//NUMBER_OF_MGS = MGS * gridValue;
		getHeuristic(&states, i->stateNode, 0);

		for(std::list<StateNode*>::iterator j = states.begin();
		j != states.end(); j++){
			MOValueFunctionPoint *pt = new MOValueFunctionPoint(
					(actionNode->Cost + (i)->g),
					successor->h,
					actionNode->Cost + (i)->g + GWEIGHT*successor->h,
					gridValue,
					gridValue,
					//1.0,//successor->goalSatisfaction,
					1,
					1,
					actionNode,
					0,
					successor);
			pt->bestPoints.push_back(i);
			pts->push_back(pt);
		}

	}
	//cout << "done"<<endl;

	goal_threshold = goalSat;
	NUMBER_OF_MGS = MGS;





}

void computeLogGrid(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		int gridPoints){



	double goalSat = goal_threshold;
	int MGS = NUMBER_OF_MGS;

	double gridValue;

	for(int k = 1; k <= gridPoints; k++){

		gridValue = 1.0 - (1/pow(2, k));

		std::list<StateNode*> states;
		states.push_back(successor);
		getHeuristic(&states, i->stateNode, 0);


		if(LUG_FOR==SPACE)
			goal_threshold = gridValue;//1.0;
		//NUMBER_OF_MGS = MGS * gridValue;
		getHeuristic(&states, i->stateNode, 0);

		for(std::list<StateNode*>::iterator j = states.begin();
		j != states.end(); j++){
			MOValueFunctionPoint *pt = new MOValueFunctionPoint(
					(actionNode->Cost + (i)->g),
					successor->h,
					actionNode->Cost + (i)->g + GWEIGHT*successor->h,
					gridValue,
					gridValue,
					//1.0,//successor->goalSatisfaction,
					1,
					1,
					actionNode,
					0,
					successor);
			pt->bestPoints.push_back(i);
			pts->push_back(pt);
		}

	}
	//cout << "done"<<endl;

	goal_threshold = goalSat;
	NUMBER_OF_MGS = MGS;





}

int numSolutionsInNeighborhood(double value, MOValueFunction *NDpoints, double NEIGHBORHOOD_EPSILON){
	int count = 0;
	for(__gnu_cxx::MOValueFunctionHash::iterator i = NDpoints->points.begin(); i != NDpoints->points.end(); i++){
		if(!(*i)->solution)
			continue;
		double distance = abs((*i)->goalSatisfaction - value);
		if(distance < NEIGHBORHOOD_EPSILON)
			count++;
	}
	return count;
}


std::map<double, RelaxedPlanLite*>::iterator sup(std::map<double, RelaxedPlanLite*> *rps,
		double pr_val){

	//std::cout << "Sup = " << pr_val << std::endl;
	std::map<double, RelaxedPlanLite*>::iterator i = rps->find(pr_val);
	if(i != rps->end()){
		//std::cout << "i = " << (*i).first << std::endl;
		i++;
		//std::cout << "i++ = " << (*i).first << std::endl;
		return i;
	}
	else {// pr_val not in map
		i = rps->upper_bound(pr_val);

		if(i == rps->end()){
			//std::cout << "No UB " << std::endl;
			return rps->end(); //no upper bound found
		}

		bool dec = false;
		while((*i).first >= pr_val && i != rps->begin()){
			dec = true;
			i--;
			//std::cout << "i-- = " << (*i).first << std::endl;
		}

		if(dec)
			i++; //went past pr_val by one, so go back
		//std::cout << "i++ = " << (*i).first << std::endl;
		return i;
	}

}

std::map<double, RelaxedPlanLite*>::iterator inf(std::map<double, RelaxedPlanLite*> *rps,
		double pr_val){
	std::map<double, RelaxedPlanLite*>::iterator i = rps->find(pr_val);
	if(i != rps->end())
		i--;
	if(i != rps->begin())
		return i;

	return rps->end();
}

void computeInsensitiveSubplansPoints(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		int numPoints,
		MOValueFunction *NDpoints){

}

void computeIndependentPointsBinarySearch(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		int numPoints,
		MOValueFunction *NDpoints){


	//want to find hte greatest value that does not +ve interact with greater value

	double goalSat = goal_threshold;
	int MGS = NUMBER_OF_MGS;

	//	double gridSpacing = 1.0/(numPoints);
	//	double gridValue = 1.0; //gridSpacing;

	//RelaxedPlanLite* referenceRP = NULL;
	double interactionWithReference = 0.0;
	double INTERACTION_THRESHOLD = 0.1;

	std::map<double, RelaxedPlanLite* > rps;
	std::map<double, RelaxedPlanLite* > independentRps;
	std::stack<double> binarySearchStack;
	binarySearchStack.push(1.0);

	//for(;gridValue > 0.0; gridValue -= gridSpacing){
	int depth = 0;
	while(binarySearchStack.size() > 0){
		depth++;
		if(depth > 20)
			break;

		double pr_val = binarySearchStack.top();
		binarySearchStack.pop();

		//std::cout << "Bin Search: " << pr_val << std::endl;

		std::list<StateNode*> states;
		states.push_back(successor);


		if(LUG_FOR==SPACE)
			goal_threshold = pr_val;
		//NUMBER_OF_MGS = MGS * gridValue;
		getHeuristic(&states, i->stateNode, 0);

		RelaxedPlanLite *rp = successor->rpIncrement;
		rps[pr_val] = rp;




		std::map<double, RelaxedPlanLite*>::iterator u = sup(&independentRps, pr_val);//independentRps.upper_bound(pr_val);
		//std::cout << "Got SUP = " << (*u).first << std::endl;
		if(u != independentRps.end()){
			//std::cout << "Upper Ind = " << (*u).first << std::endl;
			interactionWithReference = rp->computeInteraction((*u).second);
			//std::cout << "Interact = " << interactionWithReference << std::endl;
			if(interactionWithReference <= INTERACTION_THRESHOLD && rp->h_value() < (*u).second->h_value()){
				independentRps[pr_val] = rp;

				std::map<double, RelaxedPlanLite*>::iterator u1 = sup(&rps, pr_val);
				//std::cout << "u1 = " << (*u1).first  << std::endl;
				binarySearchStack.push(pr_val + ((*u1).first - pr_val)/2.0);
				//std::cout << "Push0 = " << (pr_val + ((*u1).first - pr_val)/2.0) << std::endl;

				std::map<double, RelaxedPlanLite*>::iterator l = inf(&rps, pr_val);
				//std::cout << "Lower = " << (*l).first << std::endl;
				if(l != rps.end() && (*l).first != pr_val){
					binarySearchStack.push(pr_val - (pr_val - (*l).first)/2.0);
					//std::cout << "Push1 = " << (pr_val - (pr_val - (*l).first)/2.0) << std::endl;

					//recheck to see if lower bound interacts with new value, if so, remove lb
					std::map<double, RelaxedPlanLite*>::iterator l1  = independentRps.find((*l).first);
					if(l1 != independentRps.end()){
						interactionWithReference = (*l1).second->computeInteraction(rp);
						//std::cout << "Interact Lower = " << interactionWithReference << std::endl;
						if(interactionWithReference > INTERACTION_THRESHOLD && rp->h_value() > (*l1).second->h_value()){
							//std::cout << "Remove Ind = " << (*l1).first << std::endl;
							independentRps.erase((*l1).first);
						}
					}
				}
				else{
					binarySearchStack.push(pr_val/2.0);
					//std::cout << "Push2 = " << (pr_val/2.0) << std::endl;
				}

			}
		}
		else {
			independentRps[pr_val] = rp;
			binarySearchStack.push(pr_val/2.0);
			//std::cout << "Push3 = " << (pr_val/2.0) << std::endl;
		}


	}
	for(std::map<double, RelaxedPlanLite*>::iterator j = independentRps.begin();
	j != independentRps.end(); j++){


		//if(interactionWithReference <= INTERACTION_THRESHOLD){

		//			for(std::list<StateNode*>::iterator j = states.begin();
		//			j != states.end(); j++){
		//std::cout << "Gen Dpt: " << gridValue << std::endl;

		MOValueFunctionPoint *pt = new MOValueFunctionPoint(
				(actionNode->Cost + (i)->g),
				successor->h,
				actionNode->Cost + (i)->g + GWEIGHT*(*j).second->h_value(),
				(*j).first,
				(*j).first,
				//1.0,//successor->goalSatisfaction,
				1,
				1,
				actionNode,
				0,
				successor);
		pt->bestPoints.push_back(i);
		pts->push_back(pt);
		//}
		//}
	}
	//	for(std::map<double, RelaxedPlanLite*>::iterator i = rps.begin(); i != rps.end(); i++){
	//		if((*i).second != NULL){
	//			delete (*i).second;
	//		}
	//		rps.erase(i);
	//	}
}


void computeIndependentPoints(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		int numPoints,
		MOValueFunction *NDpoints){

	double goalSat = goal_threshold;
	int MGS = NUMBER_OF_MGS;

	double gridSpacing = 1.0/(numPoints);
	double gridValue = 0.0 + gridSpacing;

	RelaxedPlanLite* referenceRP = NULL;
	double referenceRP_threshold =0.0;
	std::map<double, RelaxedPlanLite* > rps;


	double interactionWithReference = 0.0;
	double INTERACTION_THRESHOLD = .70;
	int nPts = 0;
	StateNode *parent = actionNode->PrevState;


	for(;( gridValue <= 1.0 && nPts < numPoints); gridValue += gridSpacing, nPts++){
	  for(__gnu_cxx::MOValueFunctionHash::iterator j = parent->moValueFunction->points.begin();
	      j != parent->moValueFunction->points.end(); j++){
	    if((*j)->gridValue == gridValue){
	      std::list<StateNode*> states;
	      states.push_back(successor);
	      
	      
	      if(LUG_FOR==SPACE)
		goal_threshold = gridValue;//1.0;
	      //NUMBER_OF_MGS = MGS * gridValue;
	      //	std::cout << "gettting RP" << std::endl;
	      
	      getHeuristic(&states, i->stateNode, 0);
	      //referenceRP = successor->rpIncrement;
	      // referenceRP_threshold = gridValue;
	      RelaxedPlanLite *rp = successor->rpIncrement;
	      rps[gridValue] = rp;
	      
		for(std::list<StateNode*>::iterator j = states.begin();
			j != states.end(); j++){
		  //	std::cout << "Gen Dpt: " << gridValue << std::endl;

				MOValueFunctionPoint *pt = new MOValueFunctionPoint(
						(actionNode->Cost + (i)->g),
						successor->h,
						actionNode->Cost + (i)->g + GWEIGHT*successor->h,
						gridValue,
						gridValue,
						//1.0,//successor->goalSatisfaction,
						1,
						1,
						actionNode,
						0,
						successor);
				pt->bestPoints.push_back(i);
				pts->push_back(pt);
		}
	    }
	  }
	}
	nPts = 0;
	gridValue = 1.0;

	//	std::cout << "Enter CIP" << std::endl;
	for(;(gridValue > 0.0 && nPts < numPoints); gridValue -= gridSpacing, nPts++){
	bool keepFromParent = false;
	std::list<StateNode*> states;
	if(rps[gridValue] == NULL){
	     //referenceRP && referenceRP_threshold == gridValue){
	  //  keepFromParent = true;
	//   }
// 	  else{

		states.push_back(successor);


		if(LUG_FOR==SPACE)
			goal_threshold = gridValue;//1.0;
		//NUMBER_OF_MGS = MGS * gridValue;
		//	std::cout << "gettting RP" << std::endl;

		getHeuristic(&states, i->stateNode, 0);
	
 		RelaxedPlanLite *rp = successor->rpIncrement;
// 		rps[gridValue] = rp;

		//	std::cout << "got RP" << std::endl;

		//if parent had this point, keep it around
 	
// 		for(__gnu_cxx::MOValueFunctionHash::iterator j = parent->moValueFunction->points.begin();
// 		j != parent->moValueFunction->points.end(); j++){
// 			if((*j)->gridValue == gridValue)
// 				keepFromParent = true;
// 		}


// 		if(!keepFromParent){
// 		if(!referenceRP)
// 		  referenceRP = successor->rpIncrement;
// 		else{ //have reference RP for other h-value, compute interaction
// 		  interactionWithReference = referenceRP->computeInteraction(successor->rpIncrement);
// 		  //std::cout << "Interaction = " << interactionWithReference << std::endl;
// 		  if(interactionWithReference < INTERACTION_THRESHOLD){
// 		    delete referenceRP;
// 		    referenceRP = successor->rpIncrement;
// 		  }
// 		}
// 		}
		// }

// 		if(!keepFromParent){
		bool independent = true;
		//std::cout << "new pt" << std::endl;
		for(std::map<double, RelaxedPlanLite*>::iterator j = rps.begin();
		    j != rps.end(); j++){
		  if((*j).second == rp)
		    continue;
		  //	std::cout << "computing interaction" << std::endl;
		  interactionWithReference = rp->computeInteraction((*j).second);
		  //			if(interactionWithReference  == 0)
		  //	std::cout << std::endl << "Independet" << std::endl;

// 		  if(interactionWithReference <= INTERACTION_THRESHOLD){
// 		    // std::cout << std::endl << "Compare RPS: " << gridValue << " " << (*j).first << std::endl;
		  // std::cout << "Interaction = " << interactionWithReference << std::endl;
// 		  }
			// 	if(interactionWithReference < INTERACTION_THRESHOLD)
// 					break;
		  if(interactionWithReference >= INTERACTION_THRESHOLD){
		    independent = false;
		    break;
		  }
		}
// 		}



		if(independent){
		  // std::cout << "IND" << std::endl;
		   //teractionWithReference < INTERACTION_THRESHOLD || keepFromParent){
		  rps[gridValue] = rp;
			for(std::list<StateNode*>::iterator j = states.begin();
			j != states.end(); j++){
			  //	std::cout << "Gen Dpt: " << gridValue << std::endl;

				MOValueFunctionPoint *pt = new MOValueFunctionPoint(
						(actionNode->Cost + (i)->g),
						successor->h,
						actionNode->Cost + (i)->g + GWEIGHT*successor->h,
						gridValue,
						gridValue,
						//1.0,//successor->goalSatisfaction,
						1,
						1,
						actionNode,
						0,
						successor);
				pt->bestPoints.push_back(i);
				pts->push_back(pt);
			}
		} 
	}
	}

	//	std::cout << "done w/ pt"<<std::endl<<std::endl;
	//	std::cout << "Exit CIP" << std::endl;

	//	for(std::map<double, RelaxedPlanLite*>::iterator i = rps.begin(); i != rps.end(); i++){
	//		if((*i).second != NULL){
	//			delete (*i).second;
	//		}
	//		rps.erase(i);
	//	}
	//	delete referenceRP;
}

void computeWRTNeighborhoodDensity(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		int numPoints,
		MOValueFunction *NDpoints){

	double goalSat = goal_threshold;
	int MGS = NUMBER_OF_MGS;

	double gridSpacing = 1.0/(numPoints);
	double gridValue = gridSpacing;

	double NEIGHBORHOOD_EPSILON = gridSpacing/2;
	double MAX_NEIGHBORHOOD_SIZE = 1;

	for(;gridValue <= 1.0; gridValue += gridSpacing){

		if(numSolutionsInNeighborhood(gridValue, NDpoints, NEIGHBORHOOD_EPSILON) >=
				MAX_NEIGHBORHOOD_SIZE){
			double p = 0.95;
			double rand = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
			StateNode *parent = actionNode->PrevState;
			if(rand <= p)
				continue;
		}


		std::list<StateNode*> states;
		states.push_back(successor);
		//		getHeuristic(&states, i->stateNode, 0);


		if(LUG_FOR==SPACE)
			goal_threshold = gridValue;//1.0;
		//NUMBER_OF_MGS = MGS * gridValue;
		getHeuristic(&states, i->stateNode, 0);

		for(std::list<StateNode*>::iterator j = states.begin();
		j != states.end(); j++){
			//std::cout << "Gen Dpt: " << gridValue << std::endl;

			MOValueFunctionPoint *pt = new MOValueFunctionPoint(
					(actionNode->Cost + (i)->g),
					successor->h,
					actionNode->Cost + (i)->g + GWEIGHT*successor->h,
					gridValue,
					gridValue,
					//1.0,//successor->goalSatisfaction,
					1,
					1,
					actionNode,
					0,
					successor);
			pt->bestPoints.push_back(i);
			pts->push_back(pt);
		}

	}
	//cout << "done"<<endl;

	goal_threshold = goalSat;
	NUMBER_OF_MGS = MGS;


}

bool independent(RelaxedPlan *reducedRP, RelaxedPlan *referenceRP){
	return true;
}

bool removeWorldFromRP(RelaxedPlan* reducedRP, DdNode *world){

	bool firstLevChanged = false;

	for(int j = reducedRP->plan_length-1; j >= 0; j--){

		for(std::set<LabelledAction*>::iterator a = reducedRP->action_levels[j]->begin();
		a != reducedRP->action_levels[j]->end(); a++){
			DdNode* oldLabel = (*a)->label;
			Cudd_Ref(oldLabel);

			DdNode *newLabel = Cudd_bddAnd(manager, (*a)->label, Cudd_Not(world));
			Cudd_Ref(newLabel);
			Cudd_RecursiveDeref(manager, (*a)->label);
			(*a)->label = newLabel;
			Cudd_Ref((*a)->label);
			Cudd_RecursiveDeref(manager, newLabel);

			if(j == 0 &&
					(*a)->label == Cudd_ReadLogicZero(manager) &&
					oldLabel != Cudd_ReadLogicZero(manager) &&
					!(*a)->elt->is_noop){
				firstLevChanged = true;
			}
			Cudd_RecursiveDeref(manager, oldLabel);

		}
	}
	return firstLevChanged;
}

bool computeReducedRP(RelaxedPlan* reducedRP, RelaxedPlan* referenceRP, int numSamples, std::set<DdNode*> *worldsInRP){

	bool removeWorld = false;
	//start at end of plan
	for(int j = reducedRP->plan_length-1; j >= 0; j--){
		//std::cout << "In level: " << j << std::endl;

		for(std::set<DdNode*>::iterator i = worldsInRP->begin();
		i != worldsInRP->end(); i++){

			//find first world that includes action in last action layer, and remove it

			//			std::cout << "Checking if world removes actions" << std::endl;
			//			printBDD(*i);


			for(std::set<LabelledAction*>::iterator a = reducedRP->action_levels[j]->begin();
			a != reducedRP->action_levels[j]->end(); a++){

				//		std::cout << "Action: " << (*a)->elt->alt_index << std::endl;

				if((*a)->elt->is_noop)
					continue;

				DdNode *p = Cudd_bddIntersect(manager, *i, (*a)->label);
				Cudd_Ref(p);

				//				std::cout << "Checking Intersection with action" << std::endl;
				//				printBDD(p);

				if(p != Cudd_ReadLogicZero(manager))
					removeWorld = true;
				Cudd_RecursiveDeref(manager, p);

				if(removeWorld){

					worldsInRP->erase(*i);
					return removeWorldFromRP(reducedRP, *i);
				}
			}
		}



	}
	return false;//reducedRP;
}

//void  stripRelaxedPlanForPoints(StateNode *successor,
//		ActionNode *actionNode,
//		MOValueFunctionPoint *i,
//		std::list<MOValueFunctionPoint*> *pts,
//		int numPoints,
//		MOValueFunction *NDpoints){
//	std::list<StateNode*> states;
//	states.push_back(successor);
//	StateNode *parent = actionNode->PrevState;
//
//	if(LUG_FOR==SPACE)
//		goal_threshold = 1.0;
//	getHeuristic(&states, i->stateNode, 0);
//	for(std::list<StateNode*>::iterator j = states.begin();
//	j != states.end(); j++){
//
//		RelaxedPlan *reducedRP = new RelaxedPlan(successor->currentRP);
//		std::set<DdNode*> worldsInRP (new_samples.begin(), new_samples.end());
//		std::map<int, std::pair<bool, double>* > hvalues;
//		bool firstLevChanged = true;
//		for(int k = NUMBER_OF_MGS; k > 0; k--){
//
//			//std::cout << "|worldsInRP|= " << worldsInRP.size() << std::endl;
//
//			//while samples exist
//			// remove sample
//			// if reduced rp is independent
//			//   add h-value
//
//			if(k != NUMBER_OF_MGS)
//				firstLevChanged = (firstLevChanged || computeReducedRP(reducedRP, successor->currentRP, k, &worldsInRP));
//			//std::cout << "Got Reduced RP" << std::endl;
//			//reducedRP->display();
//
//			//			hvalues[k] = new std::pair<bool, double>(firstLevChanged, reducedRP->getRelaxedConformantCost());
//			//		}
//			//
//			//		double oldCost = hvalues[NUMBER_OF_MGS]->second;
//			//
//			//		std::map<int, double> keptHvalues;
//			//		bool firstLevChanged = false;
//			//		keptHvalues[NUMBER_OF_MGS] = hvalues[NUMBER_OF_MGS]->second;
//			//		int oldK = NUMBER_OF_MGS-1;
//			//
//			//		//for(int k = NUMBER_OF_MGS-1; k > 0; k--){
//			//		while(oldK > 0){
//			//			//find boundary of range of k with same cost
//			//			//std::cout << "oldK = " << oldK << std::endl;
//			//			int newK = oldK;
//			//			int oldCost = hvalues[oldK]->second;
//			//			int newCost = oldCost;
//			//			firstLevChanged = false;
//			//			while(newCost == oldCost && newK > 0){
//			//				//std::cout << newK << std::endl;
//			//
//			//				firstLevChanged = (firstLevChanged || hvalues[newK]->first);
//			//
//			//				newK--;
//			//				if(newK > 0){
//			//					newCost = hvalues[newK]->second;
//			//				}
//			//			}
//			//
//			//			for(int k = oldK; k > newK; k--){
//			//				if(k == (oldK) && firstLevChanged)
//			//					keptHvalues[k] = hvalues[k]->second;
//			//				else
//			//					keptHvalues[k] = -1;
//			//			}
//			//			oldK = newK;
//			//		}
//			//		for(int k = NUMBER_OF_MGS; k > 0; k--){
//
//
//			double h = reducedRP->getRelaxedConformantCost();//keptHvalues[k];
//			double gridValue = (double)k/NUMBER_OF_MGS;
//
//			//if parent had this point, keep it around
//			//			bool keepFromParent = false;
//			//			for(__gnu_cxx::MOValueFunctionHash::iterator j = parent->moValueFunction->points.begin();
//			//			j != parent->moValueFunction->points.end(); j++){
//			//				if((*j)->gridValue == gridValue)
//			//					keepFromParent = true;
//			//			}
//
//			//			std::cout << gridValue << " "
//			//			<< h << " "
//			//			<< hvalues[k]->second  << " "
//			//			<< keepFromParent  << " "
//			//			<< hvalues[k]->first  << " "
//			//			<< std::endl;
//
//			//			if(h > -1 //|| keepFromParent
//			//					){
//			//
//			//				h = hvalues[k]->second;
//
//			//reducedRP->getRelaxedConformantCost();
//			//std::cout << "Add " << std::endl;
//
//			if(firstLevChanged && k%4==0){
//				firstLevChanged = false;
//				//				std::cout << gridValue << " "
//				//				<< h << " "
//				//				//<< hvalues[k]->second  << " "
//				//				//<< keepFromParent  << " "
//				//				//<< hvalues[k]->first  << " "
//				//				<< std::endl;
//				//				reducedRP->display();
//				MOValueFunctionPoint *pt = new MOValueFunctionPoint(
//						(actionNode->Cost + (i)->g),
//						h,
//						actionNode->Cost + (i)->g + GWEIGHT*h,
//						gridValue,
//						gridValue,
//						//1.0,//successor->goalSatisfaction,
//						1,
//						1,
//						actionNode,
//						0,
//						successor);
//				pt->bestPoints.push_back(i);
//				pts->push_back(pt);
//			}
//		}
//		delete reducedRP;
//		//		for(int k = NUMBER_OF_MGS; k > 0; k--){
//		//			delete hvalues[k];
//		//		}
//	}
//
//	//std::cout << "**************************************" << std::endl;
//	delete successor->currentRP;
//}

int numNonFalseLabelActs(std::set<LabelledAction*> *acts){
	int num = 0;
	for(std::set<LabelledAction*>::iterator j = acts->begin();
	j != acts->end(); j++){

		if((*j)->elt->is_noop)
			continue;

		DdNode *p = (*j)->label;
		Cudd_Ref(p);

		//				std::cout << "Checking Intersection with action" << std::endl;
		//				printBDD(p);

		if(p != Cudd_ReadLogicZero(manager))
			num++;
		Cudd_RecursiveDeref(manager, p);

	}
	return num;

}

void removeActAndParticles(RelaxedPlan* reducedRP, int *k, std::set<DdNode*> *worldsInRP){

	//find act with min particles
	int num = 0;
	int minNum = NUMBER_OF_MGS;
	LabelledAction *minAct = NULL;
	for(std::set<LabelledAction*>::iterator j = reducedRP->action_levels[0]->begin();
	j != reducedRP->action_levels[0]->end(); j++){
		if((*j)->elt->is_noop || (*j)->label == Cudd_ReadLogicZero(manager))
			continue;
		num = //Cudd_CountMinterm(manager, (*j)->label, 2*num_alt_facts);
			countParticles((*j)->label);
		if(num < minNum){
			minNum = num;
			minAct = *j;
		}
	}


	//remove particles of min act from all acts
	//std::set<DdNode*> worldsToRemove;
	//std::cout << "Found Min: " << num << std::endl;

	if(num == 0){
		*k = 0;
		return;
	}

	//printBDD(minAct->label);
	std::set<DdNode*> toRemove;
	for(std::set<DdNode*>::iterator i = worldsInRP->begin();
	i != worldsInRP->end(); i++){
		bool removeWorld = false;
		//std::cout << "HO " << std::endl;
//printBDD(minAct->label);

		if(!*i || !minAct || !minAct->label || minAct->label == Cudd_ReadLogicZero(manager))
			break;

		//printBDD(*i);


		DdNode *p = Cudd_bddIntersect(manager, *i, minAct->label);

		Cudd_Ref(p);
		if(p != Cudd_ReadLogicZero(manager)){
			//worldsToRemove.insert(*i);
			removeWorld = true;
		}
		Cudd_RecursiveDeref(manager, p);
		if(removeWorld){
			//std::cout << "HI " << std::endl;

			//worldsInRP->erase(*i);
			toRemove.insert(*i);
			removeWorldFromRP(reducedRP, *i);
			//std::cout << "HI a" << std::endl;

		}

	}
	//std::cout << "HI b: " << toRemove.size() << std::endl;

	//worldsInRP->erase(toRemove.begin(), toRemove.end());
	toRemove.clear();
	*k -= minNum;
	//std::cout << "worlds left : " << *k << std::endl;



}


//
//void  stripRelaxedPlanForPoints2(StateNode *successor,
//		ActionNode *actionNode,
//		MOValueFunctionPoint *i,
//		std::list<MOValueFunctionPoint*> *pts,
//		int numPoints,
//		MOValueFunction *NDpoints){
//	std::list<StateNode*> states;
//	states.push_back(successor);
//	StateNode *parent = actionNode->PrevState;
//
//	if(LUG_FOR==SPACE)
//		goal_threshold = 1.0;
//	getHeuristic(&states, i->stateNode, 0);
//	for(std::list<StateNode*>::iterator j = states.begin();
//	j != states.end(); j++){
//
//		RelaxedPlan *reducedRP = new RelaxedPlan(successor->currentRP);
//		std::set<DdNode*> worldsInRP (new_samples.begin(), new_samples.end());
//		//std::map<int, std::pair<bool, double>* > hvalues;
//		//bool firstLevChanged = true;
//		//for(int k = NUMBER_OF_MGS; k > 0; k--){
//		int numActs = numNonFalseLabelActs(reducedRP->action_levels[0]);
//		//std::cout << "numActs= " << numActs << std::endl;
//		int k = NUMBER_OF_MGS;
//
//
//		//reducedRP->display();
//	MOValueFunctionPoint *pt = new MOValueFunctionPoint(
//			(actionNode->Cost + (i)->g),
//			reducedRP->getRelaxedConformantCost(),
//			actionNode->Cost + (i)->g + GWEIGHT*reducedRP->getRelaxedConformantCost(),
//			1.0,
//			1.0,
//			//1.0,//successor->goalSatisfaction,
//			1,
//			1,
//			actionNode,
//			0,
//			successor);
//	pt->bestPoints.push_back(i);
//	pts->push_back(pt);
//
//		while(numActs > 0){
//
////			std::cout << "|worldsInRP|= " << k << std::endl;
////			std::cout << "numActs= " << numActs << std::endl;
//
//			//while samples exist
//			// remove sample
//			// if reduced rp is independent
//			//   add h-value
//
//
//
//			if(numActs > 1){
//			removeActAndParticles(reducedRP, &k, &worldsInRP);
//			numActs = numNonFalseLabelActs(reducedRP->action_levels[0]);
//			}
//			//			if(k != NUMBER_OF_MGS)
//			//				firstLevChanged = (firstLevChanged || computeReducedRP(reducedRP, successor->currentRP, k, &worldsInRP));
//						//std::cout << "Got Reduced RP" << std::endl;
//			//reducedRP->display();
//
//			//			hvalues[k] = new std::pair<bool, double>(firstLevChanged, reducedRP->getRelaxedConformantCost());
//			//		}
//			//
//			//		double oldCost = hvalues[NUMBER_OF_MGS]->second;
//			//
//			//		std::map<int, double> keptHvalues;
//			//		bool firstLevChanged = false;
//			//		keptHvalues[NUMBER_OF_MGS] = hvalues[NUMBER_OF_MGS]->second;
//			//		int oldK = NUMBER_OF_MGS-1;
//			//
//			//		//for(int k = NUMBER_OF_MGS-1; k > 0; k--){
//			//		while(oldK > 0){
//			//			//find boundary of range of k with same cost
//			//			//std::cout << "oldK = " << oldK << std::endl;
//			//			int newK = oldK;
//			//			int oldCost = hvalues[oldK]->second;
//			//			int newCost = oldCost;
//			//			firstLevChanged = false;
//			//			while(newCost == oldCost && newK > 0){
//			//				//std::cout << newK << std::endl;
//			//
//			//				firstLevChanged = (firstLevChanged || hvalues[newK]->first);
//			//
//			//				newK--;
//			//				if(newK > 0){
//			//					newCost = hvalues[newK]->second;
//			//				}
//			//			}
//			//
//			//			for(int k = oldK; k > newK; k--){
//			//				if(k == (oldK) && firstLevChanged)
//			//					keptHvalues[k] = hvalues[k]->second;
//			//				else
//			//					keptHvalues[k] = -1;
//			//			}
//			//			oldK = newK;
//			//		}
//			//		for(int k = NUMBER_OF_MGS; k > 0; k--){
//
//
//			double h = reducedRP->getRelaxedConformantCost();//keptHvalues[k];
//			double gridValue = (double)k/NUMBER_OF_MGS;
//
//			//if parent had this point, keep it around
//			//			bool keepFromParent = false;
//			//			for(__gnu_cxx::MOValueFunctionHash::iterator j = parent->moValueFunction->points.begin();
//			//			j != parent->moValueFunction->points.end(); j++){
//			//				if((*j)->gridValue == gridValue)
//			//					keepFromParent = true;
//			//			}
//
//			//			std::cout << gridValue << " "
//			//			<< h << " "
//			//			<< hvalues[k]->second  << " "
//			//			<< keepFromParent  << " "
//			//			<< hvalues[k]->first  << " "
//			//			<< std::endl;
//
//			//			if(h > -1 //|| keepFromParent
//			//					){
//			//
//			//				h = hvalues[k]->second;
//
//			//reducedRP->getRelaxedConformantCost();
//			//std::cout << "Add " << std::endl;
//
//			//if(firstLevChanged && k%4==0){
//				//firstLevChanged = false;
////								std::cout << gridValue << " "
////								<< h << " "
////								//<< hvalues[k]->second  << " "
////								//<< keepFromParent  << " "
////								//<< hvalues[k]->first  << " "
////								<< std::endl;
//								//reducedRP->display();
//				MOValueFunctionPoint *pt = new MOValueFunctionPoint(
//						(actionNode->Cost + (i)->g),
//						h,
//						actionNode->Cost + (i)->g + GWEIGHT*h,
//						gridValue,
//						gridValue,
//						//1.0,//successor->goalSatisfaction,
//						1,
//						1,
//						actionNode,
//						0,
//						successor);
//				pt->bestPoints.push_back(i);
//				pts->push_back(pt);
//				if(numActs == 1)
//					break;
//			}
//		//}
//		delete reducedRP;
//		//		for(int k = NUMBER_OF_MGS; k > 0; k--){
//		//			delete hvalues[k];
//		//		}
//	}
//	//exit(0);
//	//std::cout << "**************************************" << std::endl;
//	delete successor->currentRP;
//}

void computeNextNonDom(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		int numPoints,
		MOValueFunction *NDpoints){


	/*
	 * With probability p compute only those h-values that look like parent values that were good.
	 * With probability 1-p compute all h-values.
	 */


	double p = 0.95;
	double rand = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
	StateNode *parent = actionNode->PrevState;
	if(rand <= p && parent != Start){
		//don't go here for children of root node.

		int numPushed = 0;
		for(__gnu_cxx::MOValueFunctionHash::iterator j = parent->moValueFunction->points.begin();
		j != parent->moValueFunction->points.end(); j++){
			__gnu_cxx::MOValueFunctionHash::iterator e = NDpoints->points.find(*j);

			if(e != NDpoints->points.end()){
				double goalSat = goal_threshold;
				goal_threshold = (*j)->goalSatisfaction;
				std::list<StateNode*> tmpList1;
				tmpList1.push_back(successor);
				getHeuristic(&tmpList1, i->stateNode, 0);
				tmpList1.clear();
				goal_threshold = goalSat;

				//std::cout << "Gen pt: " << (*j)->goalSatisfaction << std::endl;

				MOValueFunctionPoint *pt = new MOValueFunctionPoint(
						(actionNode->Cost + (i)->g),
						successor->h,
						actionNode->Cost + (i)->g + GWEIGHT*successor->h,
						(*j)->goalSatisfaction,
						(*j)->goalSatisfaction,
						//1.0,//successor->goalSatisfaction,
						1,
						1,
						actionNode,
						0,
						successor);
				pt->bestPoints.push_back(i);
				pts->push_back(pt);
				numPushed++;
			}
		}
		//std::cout << "Pushed: " << numPushed << std::endl;
	}
	else{
		//std::cout << "Grid" << std::endl;
		computeUniformGrid(successor, actionNode, i, pts, numPoints);
		//computeWRTNeighborhoodDensity(successor, actionNode, i, pts, numPoints, NDpoints);
	}



}




void computeSingleThresholdPoint(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts){
	std::list<StateNode*> tmpList1;
	tmpList1.push_back(successor);
	getHeuristic(&tmpList1, i->stateNode, 0);
	tmpList1.clear();

	MOValueFunctionPoint *pt = new MOValueFunctionPoint(
			(actionNode->Cost + (i)->g),
			successor->h,
			actionNode->Cost + (i)->g + GWEIGHT*successor->h,
			1,
			goal_threshold,
			//1.0,//successor->goalSatisfaction,
			1,
			1,
			actionNode,
			0,
			successor);
	pt->bestPoints.push_back(i);
	pts->push_back(pt);
}


void gridOracle(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		int numPoints,
		MOValueFunction *NDpoints){

	//when probability of being in corner is a new soln., then make h-value for state


	//get pr of being at n,n
	int atxn = 0;
	int atyn = 2;
	int cnt = 6;
	DdNode *atxnDD = Cudd_bddIthVar(manager, atxn);
	Cudd_Ref(atxnDD); 
	DdNode *atynDD = Cudd_bddIthVar(manager, atyn);
	Cudd_Ref(atynDD); 
	DdNode *ncntDD = //Cudd_Not(
	Cudd_bddIthVar(manager, cnt);
	//);
	Cudd_Ref(ncntDD); 
	
	DdNode *atnDD1 = Cudd_bddAnd(manager, atynDD, atxnDD);
	Cudd_Ref(atnDD1);
	DdNode *atnDD = Cudd_bddAnd(manager, atnDD, ncntDD);
	Cudd_Ref(atnDD);
			DdNode *tmp1 = Cudd_BddToAdd(manager, atnDD);
		Cudd_Ref(tmp1);
		DdNode *tmp = Cudd_addApply(manager, Cudd_addTimes,
				successor->dd, tmp1);
		Cudd_Ref(tmp);
		double prAtNN = get_sum(tmp);
		Cudd_RecursiveDeref(manager, tmp);
		Cudd_RecursiveDeref(manager, tmp1); 
	Cudd_RecursiveDeref(manager, atxnDD); 
	Cudd_RecursiveDeref(manager, atynDD); 
	Cudd_RecursiveDeref(manager, atnDD); 
	Cudd_RecursiveDeref(manager, atnDD1); 
	
	
	//check if pr of being at n,n is new soln.
	double h = 10;
	if(prAtNN > 0){
	std::cout << "Pr(NxN) = " << prAtNN << std::endl;

		MOValueFunctionPoint *pt = new MOValueFunctionPoint(
			(actionNode->Cost + (i)->g),
			h, //hvalue
			actionNode->Cost + (i)->g + h,
			1,
			prAtNN,
			//1.0,//successor->goalSatisfaction,
			1,
			1,
			actionNode,
			0,
			successor);
	pt->bestPoints.push_back(i);
	pts->push_back(pt);
	}
	//gen h-value for climbing ladder
	
StateNode *parent = actionNode->PrevState;

	if(parent == Start)
	computeSingleThresholdPoint(successor, actionNode, i, pts);
	else
		for(__gnu_cxx::MOValueFunctionHash::iterator j = parent->moValueFunction->points.begin();
		j != parent->moValueFunction->points.end(); j++){
			__gnu_cxx::MOValueFunctionHash::iterator e = NDpoints->points.find(*j);

			if(e != NDpoints->points.end()){
				double goalSat = goal_threshold;
				goal_threshold = (*j)->goalSatisfaction;
				std::list<StateNode*> tmpList1;
				tmpList1.push_back(successor);
				getHeuristic(&tmpList1, i->stateNode, 0);
				tmpList1.clear();
				goal_threshold = goalSat;

				//std::cout << "Gen pt: " << (*j)->goalSatisfaction << std::endl;

				MOValueFunctionPoint *pt = new MOValueFunctionPoint(
						(actionNode->Cost + (i)->g),
						successor->h,
						actionNode->Cost + (i)->g + GWEIGHT*successor->h,
						(*j)->goalSatisfaction,
						(*j)->goalSatisfaction,
						//1.0,//successor->goalSatisfaction,
						1,
						1,
						actionNode,
						0,
						successor);
				pt->bestPoints.push_back(i);
				pts->push_back(pt);
				//numPushed++;
			}
		}

}

void computeMAOStarHeuristics(StateNode *successor,
		ActionNode *actionNode,
		MOValueFunctionPoint *i,
		std::list<MOValueFunctionPoint*> *pts,
		MOValueFunction *NDpoints){
	int h = 3;

	int numPoints = 2;

	//std::cout << "H for new node " << std::endl;

	switch (h) {
	case 0 :
		computeSingleThresholdPoint(successor, actionNode, i, pts);
		break;
	case 1 :
		computeUniformGrid(successor, actionNode, i, pts, numPoints);
		break;
	case 2 :
		computeLogGrid(successor, actionNode, i, pts, numPoints);
		break;
	case 3 :
		computeNextNonDom(successor, actionNode, i, pts, numPoints, NDpoints);
		break;
	case 4:
		computeWRTNeighborhoodDensity(successor, actionNode, i, pts, numPoints, NDpoints);
		break;
	case 5:
		computeIndependentPoints(successor, actionNode, i, pts, numPoints, NDpoints);
		break;
	case 6:
		computeIndependentPointsBinarySearch(successor, actionNode, i, pts, numPoints, NDpoints);
		break;
	case 7:
		computeInsensitiveSubplansPoints(successor, actionNode, i, pts, numPoints, NDpoints);
		break;
//	case 8:
//		stripRelaxedPlanForPoints(successor, actionNode, i, pts, numPoints, NDpoints);
//	case 9:
//		stripRelaxedPlanForPoints2(successor, actionNode, i, pts, numPoints, NDpoints);
	case 10:
		gridOracle(successor, actionNode, i, pts, numPoints, NDpoints);

	}

}


/*
  - By default, compute the deepest point.  
    - Deepest point changes depending on diversification.
  - Diversify when subplans are independent.
    - Cost of diversification must be low, but not miss good points
  - Consolidate when subplans positively interact.
    - Automatically consolidate by not diversifying

 */
