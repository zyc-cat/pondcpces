#include "hop_advance.h"

#include "globals.h"						// verbosity
#include "graph.h"              // StateNode, ActionNode
#include "../ppddl/actions.h"
#include <set>
#include <limits>
#include <cassert>

using namespace std;

extern void printBDD(DdNode*);

bool use_gs, use_h;

extern int g_hLimit;

bool debug = false;//true;
bool quiet = false;

static	DdNode	*background, *zero;

static bool
HOP_ddTestMintermAux(
  DdManager * dd /* manager */,
  DdNode * node /* current node */,
  int * list /* current recursion path */,
  int testIndex)
{
		bool result = false;
    DdNode	*N,*Nv,*Nnv;
    int		i,v,index;

    N = Cudd_Regular(node);

    if (cuddIsConstant(N)) {
			if (node != background && node != zero) {
				for (i = 0; i < dd->size; i++) {
					v = list[i];
					if (v == 0){}
					else if (v == 1){ if (i == testIndex) result = true; }
				}
			}
    } else {
			Nv  = cuddT(N);
			Nnv = cuddE(N);
			if (Cudd_IsComplement(node)) {
				Nv  = Cudd_Not(Nv);
				Nnv = Cudd_Not(Nnv);
			}
			index = N->index;
			list[index] = 0;
			result = result || HOP_ddTestMintermAux(dd,Nnv,list,testIndex);
			list[index] = 1;
			result = result || HOP_ddTestMintermAux(dd,Nv,list,testIndex);
			list[index] = 2;
    }
    return result;

} /* end of ddPrintMintermAux */


bool
HOP_TestMinterm(
  DdManager * manager,
  DdNode * node,
  int testIndex)
{
    int		i, *list;
    bool result = false;

    background = manager->background;
    zero = Cudd_Not(manager->one);
    list = ALLOC(int,manager->size);
    if (list == NULL) {
			manager->errorCode = CUDD_MEMORY_OUT;
			return(0);
    }
    for (i = 0; i < manager->size; i++) list[i] = 2;
    result = HOP_ddTestMintermAux(manager,node,list,testIndex);
    FREE(list);
    return result;

} /* end of Cudd_PrintMinterm */

void printHOP(DdNode* dd){
	printBDD(dd);

	bool up = HOP_TestMinterm(manager, dd, 0);
	bool down = !up;

	bool closed = HOP_TestMinterm(manager, dd, 6);
	bool open = !closed;

	int floor;
	if(HOP_TestMinterm(manager, dd, 12))
		floor = 2;
	else if(HOP_TestMinterm(manager, dd, 10))
		floor = 1;
	else
		floor = 0;

	bool in_up = HOP_TestMinterm(manager, dd, 24);
	bool in_down = HOP_TestMinterm(manager, dd, 16);

	bool waiting_up_f0 = HOP_TestMinterm(manager, dd, 4);
	bool waiting_up_f1 = HOP_TestMinterm(manager, dd, 8);
	bool waiting_up_f2 = HOP_TestMinterm(manager, dd, 14);

	bool waiting_down_f0 = HOP_TestMinterm(manager, dd, 22);
	bool waiting_down_f1 = HOP_TestMinterm(manager, dd, 20);
	bool waiting_down_f2 = HOP_TestMinterm(manager, dd, 18);

	for(int i = 3; i >= -1; i--){
		// floor number
		if(i >= 0 && i < 3)
			cout << i;
		else
			cout << " ";
		cout << " ";
		// elevator direction left-side, and up passenger
		if(up && i == floor + 1)
			cout << "/";
		else if(down && i == floor - 1)
			cout << "\\";
		else if(i == floor){
			if(in_up)
				cout << "^";
			else
				cout << "_";
		}
		else
			cout << " ";
		// elevator direction right-side, and down passenger
		if(up && i == floor + 1)
			cout << "\\";
		else if(down && i == floor - 1)
			cout << "/";
		else if(i == floor){
			if(in_down)
				cout << "v";
			else
				cout << "_";
		}
		else
			cout << " ";
		// elevator door
		if(open && i == floor)
			cout << "O";
		else if(closed && i == floor)
			cout << "X";
		else
			cout << " ";
		cout << " ";
		// Waiting up
		if((waiting_up_f0 && i == 0) || (waiting_up_f1 && i == 1) || (waiting_up_f2 && i == 2))
			cout << "^";
		else
			cout << " ";
		// Waiting down
		if((waiting_down_f0 && i == 0) || (waiting_down_f1 && i == 1) || (waiting_down_f2 && i == 2))
			cout << "v";
		else
			cout << " ";
		cout << endl;
	}
}


bool HAStateComparator::operator() (const HAStateNode& lhs, const HAStateNode& rhs) const{
	if(lhs.state == rhs.state && lhs.horizon == rhs.horizon)
		return false;
/*
	double lhs_f = lhs.horizon + lhs.state->h;
	double rhs_f = rhs.horizon + rhs.state->h;
	if(lhs_f != rhs_f)
		return lhs_f < rhs_f;
*/
///*
	if(lhs.state->h != rhs.state->h)
		return lhs.state->h < rhs.state->h;

	if(lhs.state->goalSatisfaction != rhs.state->goalSatisfaction)
		return lhs.state->goalSatisfaction > rhs.state->goalSatisfaction;
//*/

	if(lhs.state->randPriority != rhs.state->randPriority)
		return lhs.state->randPriority < rhs.state->randPriority;

	return lhs.state < rhs.state;
}

HOPAdvance::HOPAdvance(ActionNode* start)
: next(start->newSample(), start->PrevState->horizon + 1){
	open.clear();
	open.insert(next);
	done = false;
	first = true;
	curRand = rand();
	this->start = start->PrevState;
	hop_horizon = start->PrevState->horizon + g_hLimit;
	if(hop_horizon > max_horizon)
		hop_horizon = max_horizon;
}

HOPAdvance::~HOPAdvance(){
}

bool HOPAdvance::step(){
char line[256];

	if(first){
		first = false;
		return true;
	}

	if(done || open.empty() || next.horizon >= /*max_horizon*/hop_horizon){
		done = true;
		return false;
	}
do{
	open.erase(open.begin());
	closed.insert(next);
	next.state->g = next.state->horizon = next.horizon;
	next.state->addAllActions();

	if(false && !debug){
		/*
		cout << HOP_TestMinterm(manager, next.state->dd, 4)
		<< HOP_TestMinterm(manager, next.state->dd, 8)
		<< HOP_TestMinterm(manager, next.state->dd, 14)
		<< HOP_TestMinterm(manager, next.state->dd, 18)
		<< HOP_TestMinterm(manager, next.state->dd, 20)
		<< HOP_TestMinterm(manager, next.state->dd, 22)
		<< endl;
		*/
		if((HOP_TestMinterm(manager, next.state->dd, 4)  && HOP_TestMinterm(manager, next.state->dd, 2)  &&  HOP_TestMinterm(manager, next.state->dd, 0) && !HOP_TestMinterm(manager, next.state->dd, 6))
		|| (HOP_TestMinterm(manager, next.state->dd, 22) && HOP_TestMinterm(manager, next.state->dd, 2)  && !HOP_TestMinterm(manager, next.state->dd, 0) && !HOP_TestMinterm(manager, next.state->dd, 6))
		|| (HOP_TestMinterm(manager, next.state->dd, 8)  && HOP_TestMinterm(manager, next.state->dd, 10) &&  HOP_TestMinterm(manager, next.state->dd, 0) && !HOP_TestMinterm(manager, next.state->dd, 6))
		|| (HOP_TestMinterm(manager, next.state->dd, 20) && HOP_TestMinterm(manager, next.state->dd, 10) && !HOP_TestMinterm(manager, next.state->dd, 0) && !HOP_TestMinterm(manager, next.state->dd, 6))
		|| (HOP_TestMinterm(manager, next.state->dd, 14) && HOP_TestMinterm(manager, next.state->dd, 12) &&  HOP_TestMinterm(manager, next.state->dd, 0) && !HOP_TestMinterm(manager, next.state->dd, 6))
		|| (HOP_TestMinterm(manager, next.state->dd, 18) && HOP_TestMinterm(manager, next.state->dd, 12) && !HOP_TestMinterm(manager, next.state->dd, 0) && !HOP_TestMinterm(manager, next.state->dd, 6))
		)
			debug = true;
	}

	if(debug && !quiet){
		cout << "STATE " << next.state->StateNo << ":" << endl;
		printHOP(next.state->dd);
//		getchar();
	}

	double new_horizon = next.horizon + 1;
	for(ActionNodeList::iterator act_it = next.state->NextActions->begin(); act_it != next.state->NextActions->end(); act_it++){
		ActionNode* action = *act_it;
		StateNode* child = action->newSample();

		if(debug && !quiet){
			cout << "ACTION: " << action->act->name() << "\t" << "(STATE " << child->StateNo <<  ")" << endl;
			printHOP(child->dd);
//			getchar();
		}

		HAStateNode newState(child, new_horizon);
		if(closed.count(newState) <= 0)
			open.insert(newState);
	}

	next = *open.begin();
	curRand = rand();

	if(debug){
		quiet = false;
		cin.getline(line, 256);
		if(line[0] == 'q' || line[0] == 'Q')
			quiet = true;
		int num = 0;
		if(quiet)
			num = atoi(line+1);
		else
			num = atoi(line);
		if(num > 0){
			for(set<HAStateNode, HAStateComparator>::iterator it = open.begin(); it != open.end(); it++)
			{
				HAStateNode curState = (*it);
				if(curState.state->StateNo == num){
					next = curState;
					num = -1;
					break;
				}
			}
		}
		if(num > 0){
			for(set<HAStateNode, HAStateComparator>::iterator it = closed.begin(); it != closed.end(); it++)
			{
				HAStateNode curState = (*it);
				if(curState.state->StateNo == num){
					next = curState;
					break;
				}
			}
		}
	}
}while(debug);
	return true;
}

double HOPAdvance::getQuality(){
///*
	if(use_gs && !use_h)
		return next.state->goalSatisfaction;	// Assuming conformant search
	else if(!use_gs && use_h)
		return (1.0 - (next.state->h / start->h));

	return next.state->goalSatisfaction + (1.0 - (next.state->h / start->h));
//*/
//	return -(next.state->h);
}

int HOPAdvance::getRand(){
	return curRand;
//	return next.state->goalSatisfaction;
}
