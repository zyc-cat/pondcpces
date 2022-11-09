#include "ehc.h"

#include "globals.h"
#include "astar.h"
//#include "lao_wrapper.h"        // expandState
#include "graph.h" // StateNode::expand
#include "lug.h"
#include "math.h"
#include "backup.h"
#include <float.h>
#include <algorithm>

using namespace __gnu_cxx;
using namespace std;

extern double gWeight;

EHC::EHC()
	: Search(EHC_SEARCH)
{
	reEvaluateChildren = false;
	INITLUG = false;
}

bool EHC::card_less_than(StateNode *a, StateNode *b)
{
	if (a == Goal || a->goalSatisfaction >= goal_threshold)
		return true;
	else
		return Cudd_CountMinterm(manager, a->dd, num_alt_facts) < Cudd_CountMinterm(manager, b->dd, num_alt_facts);
}

void EHC::printPlan(StateNode *tail)
{
	if (tail == Start)
	{
		cout << "START" << endl;
		return;
	}
	else
		printPlan(tail->PrevActions->Node->PrevState);

	if (tail->PrevActions->Node)
		printAction(tail->PrevActions->Node);
	else
		cout << "NO BEST" << endl;
}

void EHC::backupSet(StateHash *nodes)
{
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (StateHash::iterator i = nodes->begin(); i != nodes->end(); i++)
		{
			double of = (*i).second->f;
			Backup((*i).second, (*i).second->g);
			if (of != (*i).second->f)
				changed = true;
		}
	}
}

StateNode *EHC::findBetterNode(StateNode *root)
{
	std::deque<StateNode *> StateQ;
	std::list<StateNode *> tmpList;
	StateQ.push_back(root);

	StateNode *bestSuccessor = root;
	double oldSuccessorH, oldRootH = root->h;
	StateHash Nodes;

	USE_GLOBAL_RP = true;

	getHeuristic(&StateQ, root, 0);
	cout << "\t\t\t[" << root->h << "]" << flush;

	while (!StateQ.empty())
	{
		StateNode *front = StateQ.front();
		StateQ.pop_front();

		if (front->NextActions == NULL)
		{
			cout << "." << flush;
			front->expand();
			ExpandedStates++;
		}
		else if (front->BestAction != NULL && front != root)
		{
			cout << "CYCLE " << front->StateNo << endl;
			bestSuccessor = NULL;
			break;
		}

		if (front->NextActions == NULL)
		{
			cout << "DEAD END " << endl;
			front->h = DBL_MAX;
			front->g = 0;
			front->Terminal = 1;
			bestSuccessor = NULL;
			break;
		}

		for (ActionNodeList::iterator act = front->NextActions->begin(); act != front->NextActions->end(); act++)
		{
			// assuming each act has one successor because otherwise we would need AO*
			StateNode *successor = (*act)->NextState->State;

			if (front == root && successor->h != DBL_MAX)
			{
				tmpList.push_back(successor);
				getHeuristic(&tmpList, successor, 0);
				tmpList.clear();
			}

			if (!Nodes.contains(successor))
			{ // new node
				NodesExpanded.add(successor);
				Nodes.add(successor);
				StateQ.push_back(successor);
			}

			if (successor->h < bestSuccessor->h)
				bestSuccessor = successor;

			if (successor == Goal || successor->goalSatisfaction >= goal_threshold)
			{
				successor->Terminal = 1;
				bestSuccessor = successor;
				cout << "GOAL" << endl;
				break;
			}
		}

		if (bestSuccessor != NULL && bestSuccessor != root && (bestSuccessor == Goal || bestSuccessor->goalSatisfaction >= goal_threshold))
		{
			cout << "GOAL" << endl;
			bestSuccessor->h = 0;
			bestSuccessor->g = 0;
			break;
		}
		else if (bestSuccessor != NULL && bestSuccessor != root)
		{
			if (!USE_GLOBAL_RP && !USE_CARD_GRP)
			{
				oldSuccessorH = bestSuccessor->h;
				USE_GLOBAL_RP = true;
				tmpList.push_back(bestSuccessor);
				getHeuristic(&tmpList, bestSuccessor, 0);
			}
			else if (USE_CARD_GRP)
			{
				HEURISTYPE = CARD;
				tmpList.push_back(bestSuccessor);
				getHeuristic(&tmpList, bestSuccessor, 0);
			}
			break;
		}

		StateQ.sort(h_less_than);
	}
	bestSuccessor = recursive(bestSuccessor);

	backupSet(&Nodes);

	if (bestSuccessor != Goal && bestSuccessor->goalSatisfaction < goal_threshold)
		return NULL;
	else
		return bestSuccessor;
}

StateNode *EHC::recursive(StateNode *root)
{
	list<StateNode *> tmpList;
	StateNode *rnode;
	ActionNode *bestAct;

	if (root == NULL)
	{
		cout << "NULL" << endl;
		return NULL;
	}

	if (root->Terminal == 1)
	{
		cout << "Hit terminal " << root->StateNo
			 << (root == Goal || root->goalSatisfaction >= goal_threshold ? " GOAL" : " NO GOAL")
			 << (root->Solved ? " Solved" : " Not Solved")
			 << endl;

		if (root != Goal && root->goalSatisfaction < goal_threshold)
		{
			root->h = DBL_MAX;
			root->g = 0;
			root->Solved = 0;
			root = NULL;
		}
		return root;
	}

	if (root->h == DBL_MAX)
	{
		cout << "BAD SUFFIX " << root->StateNo << endl;
		return NULL;
	}

	if (root->BestAction != NULL && !reEvaluateChildren)
		rnode = recursive(root->BestAction->NextState->State);
	else
	{
		// get h for root to check improvement
		if (reEvaluateChildren && root->Terminal != 1)
		{
			tmpList.push_back(root);
			getHeuristic(&tmpList, root, 0);
			tmpList.clear();
		}

		cout << '\t';
		if (HEURISTYPE != CARD)
			cout << '\t';

		cout << "[" << root->h << "]" << endl;

		StateNode *bestNode = NULL;
		if (root == Goal || root->goalSatisfaction >= goal_threshold)
		{
			Start->ExtendedGoalSatisfaction = root->goalSatisfaction;
			root->Terminal = 1;
			root->h = 0;
			root->g = 0;
			rnode = root;
		}

		if (!root->BestAction && root->Terminal != 1)
		{
			root->expand();
			ExpandedStates++;
			if (!root->NextActions)
			{
				cout << "DEAD_END " << root->StateNo << endl;
				root->h = DBL_MAX;
				root->g = 0;
				root->Terminal = 1;
				root->Solved = 0;
				return NULL;
			}
		}

		for (ActionNodeList::iterator act = root->NextActions->begin(); act != front->NextActions->end(); act++)
		{
			// assuming each act has one successor because otherwise we would need AO*
			StateNode *successor = (*act)->NextState->State;

			if (reEvaluateChildren && successor->h != DBL_MAX)
			{
				tmpList.push_back(successor);
				getHeuristic(&tmpList, root, 0);
				tmpList.clear();
			}

			if (!NodesExpanded.contains(successor)) // new node
				NodesExpanded.add(successor);

			if (successor == Goal || successor->goalSatisfaction >= goal_threshold)
			{
				bestNode = successor;
				bestAct = act->Node;
				cout << "GOAL" << endl;
				break;
			}

			if (bestNode == NULL || successor->h < bestNode->h)
				bestNode = successor;
		}

		if (bestNode == Goal || bestNode->goalSatisfaction >= goal_threshold)
		{
			cout << "GOT GOAL" << endl;
			bestNode->h = 0;
			bestNode->g = 0;
			Backup(root, root->g);
			root->Solved;
			root->BestAction = bestAct;
			return bestNode;
		}

		if (bestNode->h < root->h)
		{
			if (USE_CARD_GRP && HEURISTYPE == LUGRP)
			{
				HEURISTYPE = CARD;
				reEvaluateChildren = true;
				tmpList.push_back(root);
				getHeuristic(&tmpList, root, 0);
				tmpList.clear();
			}
			rnode = recursive(bestNode);
		}
		else
		{
			// if using card, re-evaluate children with LUGRP to see if it finds better children.
			if (USE_CARD_GRP && HEURISTYPE == CARD)
			{
				if (!INITLUG)
				{
					initLUG(&action_preconds, b_goal_state);
					INITLUG = true;
					getHeuristic(NULL, NULL, 0);
				}
				HEURISTYPE = LUGRP;
				USE_GLOBAL_RP = true;
				reEvaluateChildren = true;
				rnode = recursive(root);
			}
			else
			{
				if (!INITLUG)
				{
					initLUG(&action_preconds, b_goal_state);
					INITLUG = true;
					getHeuristic(NULL, NULL, 0);
				}
				HEURISTYPE = LUGRP;
				rnode = findBetterNode(root);
			}
		}
	}

	if (root->Terminal == 0)
		Backup(root, root->g);

	return rnode;
}

void EHC::search()
{
	StateNode *end;

	ExpandedStates = 0;
	Start->ExtendedGoalSatisfaction = 1.0; // hack
	Start->f *= gWeight;

	while (Start->Solved == 0 && Start->h != DBL_MAX)
	{
		if (USE_CARD_GRP)
			HEURISTYPE = CARD;
		reEvaluateChildren = false;
		cout << "**New EHC Trial**" << endl;
		end = recursive(Start);
	}

	if (Start->h == DBL_MAX)
	{
		cout << "No Solution" << endl;
		return;
	}

	if (end == NULL)
	{
		Search *search = new AStar();
		search->search();
		delete search;
	}
	else
	{
		cout << "ending EHC " << end->StateNo << endl;
		cout << Start->ExtendedGoalSatisfaction << endl;
	}

	cout << "Expanded States = " << ExpandedStates << endl;
}
