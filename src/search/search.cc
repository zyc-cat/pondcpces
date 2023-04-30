#include "search.h"

#include "globals.h"	 // StateHash, PF_LUG, NUMBER_OF_MGS
#include "cudd/dd.h"	 // DdNode
#include "lug.h"		 // getHeuristic, increment_heuristic
#include "graph.h"		 // StateNode, StateDistribution, ActionNode, ActionNodeList
#include "movalfn.h"	 // MOValueFunction, MOValueFunctionPoint, getMOLAOHeuristics
#include "solve.h"		 // gWeight, gNumActions
#include "sample_size.h" // random_walk_sample_size
#include "lao_wrapper.h" // everything else (ex. computeGoalSatisfaction and other functions)

#include <limits> // epsilon
#include <list>
#include <float.h>
#include <queue>
#include <iomanip>
#include <fstream>

using namespace __gnu_cxx;
using namespace std;

extern int num_lugs;

#ifdef PPDDL_PARSER
extern void computeSuccessors(pair<const Action *const, DdNode *> *action, struct StateNode *parent, list<DdNode *> *successors, list<DdNode *> *reasons, list<double> *probabilities, list<double> *rewards, list<double> *klds, int reps);
#else
extern void computeSuccessors(alt_action *a, struct StateNode *parent, list<DdNode *> *successors, list<DdNode *> *reasons, list<double> *probabilities, list<double> *rewards, int reps);
#endif

SearchType Search::type; // TO-DO: Make it non-static once everything's in order

Search::Search(SearchType type)
{
	this->type = type;
}

void Search::init(int num_acts, DdNode *b_initial_state, DdNode *b_goal_state)
{
	int state, action, nextStateSuccess, nextStateFail, numFields, terminal, start[6], i, numStates;
	struct StateNode *stateNode;
	struct ActionNode *actionNode;
	struct StateDistribution *stateDist;
	list<StateNode *> states;
	double gEpsilon = 0.001;

	StateNode::expandedStates = 0;

	gWeight = GWEIGHT;
	state_count = 0;
	StateIndex = new StateHash();
	LeafStates = new StateHash();
	gNumActions = num_acts;

	Goal = new StateNode();
	Goal->h = 0;
	Goal->f = 0;
	Goal->g = 0;
	Goal->goalSatisfaction = 1.0;
	Goal->dd = b_goal_state;
	Cudd_Ref(Goal->dd);
	Goal->Terminal = 1;
	Goal->meanFirstPassage = 0.0;
	Goal->Solved = 1;
	Goal->Expanded = 0;
	Goal->StateNo = state_count++;
	Goal->ExtendedGoalSatisfaction = 1.0;

	if (type == MOLAO || type == MOASTAR)
		Goal->moValueFunction->points.insert(new MOValueFunctionPoint(0.0, 0.0,
																	  0.0, 1.0,
																	  1.0, 1, 0.0,
																	  NULL, true,
																	  Goal));

	StateIndex->add(Goal);

	Start = new StateNode();
	Start->prReached = 1.0;
	Start->dd = b_initial_state;
	Start->BestAction = NULL;
	Cudd_Ref(Start->dd);

	//  printBDD(Start->dd);

	// conformant planning无概率，这部分可能需要修改
#ifdef PPDDL_PARSER
	if (OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY)
	{ // cost
		if (total_goal_reward < 0.0)
			Start->h = total_goal_reward;
		else
			Start->h = 0.0;
	}
	else if (!OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY) // probability
		Start->h = -1.0;
#else
	Start->h = 0.0;
#endif

	Start->StateNo = state_count++;

#ifdef PPDDL_PARSER
	Start->goalSatisfaction = computeGoalSatisfaction(Start->dd);
#endif

	if (type == MOLAO || type == MOASTAR)
	{
		Start->moValueFunction->points.insert(new MOValueFunctionPoint(0.0, 0.0,
																	   0.0,
																	   Start->goalSatisfaction,
																	   Start->goalSatisfaction,
																	   1, 0.0,
																	   NULL, true,
																	   Start));
		Start->moValueFunction->points.insert(new MOValueFunctionPoint(0.0, Start->h,
																	   Start->h,
																	   Start->goalSatisfaction,
																	   Start->goalSatisfaction,
																	   1, 0.0,
																	   NULL, false,
																	   Start));
	}

	terminalAction = new Action("TerminalAction");

	if (PF_LUG)
	{
		if (NUMBER_OF_MGS < 1.0)
			NUMBER_OF_MGS = random_walk_sample_size(Start, NUMBER_OF_MGS);
	}

	/**
	 * momo007 2022.06.01 not used, 
	 * only use partial obs(Contigent) and null obs(Conformant)
	 */
	// insert dummy action that partitions starting belief
	if (OBSERVABILITY == OBS_FULL && (numStates = (int)(Cudd_CountMinterm(manager, Start->dd, 2 * num_alt_facts) - pow(2, num_alt_facts))) && numStates > 1)
	{
		cout << "FO" << endl;
		assert(0);
		list<DdNode *> obs_result;

#ifdef PPDDL_PARSER
		list<double> values;
		// momo007 need rewrite to bdd
		get_states_from_add(Start->dd, &obs_result, &values);
#else
		get_states_from_bdd(Start->dd, &obs_result);
#endif

		i = 0;
		actionNode = new ActionNode();
		actionNode->ActionNo = -1;

#ifdef PPDDL_PARSER
#else
		actionNode->act = new alt_action();
#endif
		// 添加初始状态节点和动作结点的连接
		actionNode->NextState = NULL;
		actionNode->PrevState = Start;
		Start->NextActions = new ActionNodeList;
		Start->NextActions->push_back(actionNode);
		Start->BestAction = actionNode;
		Start->Expanded = 1;
		actionNode->Cost = 0;

#ifdef PPDDL_PARSER
		list<double>::iterator v = values.begin();
#endif

		for (list<DdNode *>::iterator i = obs_result.begin(); i != obs_result.end(); i++)
		{
			if (bdd_entailed(manager, *i, b_goal_state))
			{
				stateNode = Goal;
			}
			else
			{
				stateNode = new StateNode();
				states.push_back(stateNode);
				stateNode->NextActions = NULL;
				stateNode->dd = *i;
				Cudd_Ref(stateNode->dd);
				stateNode->Terminal = 0;
				StateIndex->add(stateNode);
			}

			stateDist = CreateStateDistribution();
			stateDist->Next = NULL;
			stateDist->State = stateNode;
			stateDist->reason = *i;
			Cudd_Ref(stateDist->reason);
			stateDist->Next = actionNode->NextState;
			actionNode->NextState = stateDist;

#ifdef PPDDL_PARSER
			stateDist->Prob = *v;
			v++;
#else
			stateDist->Prob = 1.0 / (double)numStates;
#endif
			if (stateNode->PrevActions == NULL)
				stateNode->PrevActions = new ActionNodeList;
			stateNode->PrevActions->push_front(actionNode);
			Cudd_RecursiveDeref(manager, *i);
		}

		if (!USE_CARD_GRP && (LUG_FOR == SPACE || LUG_FOR == INCREMENTAL) && num_lugs == 0)
			getHeuristic(NULL, NULL, 0);

		if (DO_INCREMENTAL_HEURISTIC)
			increment_heuristic(Start);
		else if (!DEFERRED_EVALUATION)
			getHeuristic(&states, Start, 0);
	}
	else
	{

		if (!USE_CARD_GRP && (LUG_FOR == SPACE || LUG_FOR == INCREMENTAL) && HEURISTYPE != NONE && HEURISTYPE != CARD && num_lugs == 0)
		{
			std::cout << "LUG_FOR = SPACE\n";
			getHeuristic(NULL, NULL, 0);
			//  printBDD(Start->dd);
		}
		else if (USE_CARD_GRP)
		{
			std::cout << "USE CARD GRP\n";
			HEURISTYPE = CARD;
		}

		states.push_back(Start);
		if (type == MOLAO){
			std::cout << "MOLAO heuristic[waring!!!]\n";
			assert(0);
			getMOLAOHeuristics(&states, Start);
		}
		else if (!DEFERRED_EVALUATION)
		{
			std::cout << "!DEFERRED EVALUATION\n";
			getHeuristic(&states, Start, 0);
			// cout << "H(I) = " << Start->h << endl; // 注释掉输出
		}
	}

	StateNode::generated[Start->dd] = Start;
	StateNode::generated[Goal->dd] = Goal;
}

Search::~Search()
{
}

void Search::printStateList()
{
	ActionNodeList::iterator tmpAct;
	StateDistribution *dist;
	cout << endl
		 << "******************State List*********************" << endl;
	for (StateHash::iterator i = StateIndex->begin(); i != StateIndex->end(); i++)
	{
		cout << "################################" << endl;
		cout << "State " << (*i).second->StateNo
			 << " h = " << (*i).second->h
			 << " g = " << (*i).second->g
			 << " f = " << (*i).second->f
			 << " Satis = " << (*i).second->goalSatisfaction
			 << " Expanded = " << (*i).second->Expanded
			 << " Terminal = " << (*i).second->Terminal
			 << " E[R] = " << (*i).second->expDiscRew
			 << endl;
		if (type == MOLAO)
			(*i).second->moValueFunction->print(cout);

		printBDD((*i).second->dd);
		cout << "Supported by: " << endl;
		if ((*i).second->PrevActions != NULL)
		{
			for (tmpAct = (*i).second->PrevActions->begin(); tmpAct != (*i).second->PrevActions->end(); tmpAct++)
			{
				ActionNode *action = *tmpAct;
				if (action->PrevState->BestAction &&
					action->PrevState->BestAction->ActionNo == action->ActionNo)
					cout << "*BEST*";
				cout << " " << action->ActionNo << " ";
				printAction(action);
				cout << " Cost = " << action->Cost << endl;
				cout << " from state = " << action->PrevState->StateNo << endl;
			}
		}

		cout << "Applied in: " << endl;
		if ((*i).second->NextActions != NULL)
		{
			for (tmpAct = (*i).second->NextActions->begin(); tmpAct != (*i).second->NextActions->end(); tmpAct++)
			{
				ActionNode *action = *tmpAct;
				if (action->PrevState->BestAction &&
					action->PrevState->BestAction->ActionNo == action->ActionNo)
					cout << "*BEST*";
				printAction(action);
				cout << "leads to: ";
				for (dist = action->NextState; dist != NULL; dist = dist->Next)
					cout << dist->State->h << ":" << dist->Prob << ":" << dist->State->StateNo << " ";
				cout << endl;
			}
		}
	}

	cout << endl
		 << "******************End State List***********************" << endl;
}

void Search::graphToFile(const char *filename)
{
	ofstream graphFile(filename);

	hash_set<StateNode *, PointerHasher> closed;
	closed.insert(Start);

	queue<StateNode *> open;
	open.push(Start);

	StateNode *front = NULL;

	graphFile << endl
			  << "delimiter" << endl
			  << setiosflags(ios::fixed) << setprecision(3);

	while (!open.empty())
	{
		front = open.front();
		open.pop();

		graphFile << "OBS_" << front->StateNo << endl
				  << front->g << endl
				  << front->h << endl
				  << front->goalSatisfaction << endl
				  << front->prReached << endl;

		if (front->Expanded > 0)
		{
			for (ActionNodeList::iterator act_it = front->NextActions->begin(); act_it != front->NextActions->end(); act_it++)
			{
				ActionNode *action = *act_it;
				graphFile << "ACT_";
				if (action->ActionNo == -1)
					graphFile << "TERM";
				else
					graphFile << action->ActionNo;
				graphFile << endl;
				action->act->print(graphFile, (*my_problem).terms());
				graphFile << endl;
				for (StateDistribution *dist = action->NextState; dist != NULL; dist = dist->Next)
				{
					StateNode *obs = dist->State;
					graphFile << "OBS_" << obs->StateNo << endl
							  << dist->Prob << endl;
					if (closed.count(obs) <= 0)
					{
						open.push(obs);
						closed.insert(obs);
					}
				}
				graphFile << "END" << endl;
			}
		}
		graphFile << "END" << endl;
	}

	graphFile.close();
}

void Search::incremental_search()
{
	set<StateNode *> open;
	set<StateNode *> closed;
	StateNode *state;// search node in graph

	double minGoalSat, maxGoalSat;

	search_goal_threshold = numeric_limits<double>::epsilon();

	while (true)
	{
		cout << "*** Incremental Policy Creation, goal threshold = " << search_goal_threshold << endl;
		resetPolicy();
		search();// do the forward search， using the inheriting class implementations
		// check the threshold if sat
		if (Start->ExtendedGoalSatisfaction >= plan_success_threshold)
			return;
		// if not sat, use the bfs search
		minGoalSat = 1.0;
		maxGoalSat = 0.0;

		closed.clear();
		open.clear();
		open.insert(Start);
		while (!open.empty())
		{
			state = *open.begin();// pop one from open
			open.erase(open.begin());
			closed.insert(state);// add to closed
			// reach the terminal, use the node to update the interval
			if (state->Terminal > 0)
			{
				if (minGoalSat > state->goalSatisfaction)
					minGoalSat = state->goalSatisfaction;
				if (maxGoalSat < state->goalSatisfaction)
					maxGoalSat = state->goalSatisfaction;
			}
			// if contain successor state, use them to do new iteration
			if (state->BestAction != NULL)
			{
				for (StateDistribution *dist = state->BestAction->NextState; dist != NULL; dist = dist->Next)
				{
					StateNode *successor = dist->State;
					if (closed.count(successor) <= 0)// if not travel, add to open
						open.insert(successor);
				}
			}
		}
		// check if sat the goal threshold
		if (maxGoalSat < search_goal_threshold)
			return;
		/**
		 * momo007 这里后续可能需要调整，返回最大还是最小。
		 */
		search_goal_threshold = numeric_limits<double>::epsilon() + minGoalSat; // Alan- MIN? MAX?
	}
}

void Search::resetPolicy()
{
	set<StateNode *> open;
	set<StateNode *> closed;
	StateNode *state;

	open.insert(Start);
	while (!open.empty())
	{
		state = *open.begin();
		open.erase(open.begin());
		closed.insert(state);

		if (state->BestAction != NULL)
		{
			for (StateDistribution *dist = state->BestAction->NextState; dist != NULL; dist = dist->Next)
			{
				StateNode *next = dist->State;
				if (closed.count(next) <= 0)
					open.insert(next);
			}
		}
		// reset to the defualt null and  zero
		state->BestAction = NULL;
		state->Solved = 0;
	}
}
