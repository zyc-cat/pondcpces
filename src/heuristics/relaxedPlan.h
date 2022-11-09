#ifndef RELAXED_PLAN_H
#define RELAXED_PLAN_H

struct DdNode;
struct DdManager;
//#include "cudd/cudd.h"
//#include "cudd/cuddObj.hh"
//#include "nusmv/dd/dd.h"
#include "lug/labelledElement.h"
#include "lug/ipp.h"
//#include <stl.h>

#include "graph_wrapper.h"

//using namespace std;

extern BitVector* new_bit_vector(int);
extern int gft_vector_length;
extern FactInfoPair *new_fact_info_pair(FactInfo*, FactInfo*);
extern FactInfo *new_FactInfo( void );
extern void print_BitVector( BitVector* , int  );
extern void free_fact_info_pair( FactInfoPair *p );
extern void printBDD(DdNode*);
extern void printFact(int);
extern DdNode*  ARE_MUTEX_EFS(int, EfNode*, EfNode*, DdNode*, DdNode*);

//extern DdNode* Cudd_IthVar(DdManager*, int);
extern int BRANCH_SCHEME;
extern DdManager* manager;

//DdNode* removeWorlds(double,double);
DdNode* removeWorldsToImproveCost(double, int);
//DdNode* addWorldsUntilCostIncreases(double tau, int graph_levels);
class RelaxedPlan {


public:

	std::set<LabelledAction*>* action_levels[IPP_MAX_PLAN];  //OpNodes
	std::set<LabelledEffect*>* effect_levels[IPP_MAX_PLAN];  //EfNodes
	std::list<DdNode*>* partitions[IPP_MAX_PLAN];
	std::list<LabelledEffect*>* ef_mutexes_a[IPP_MAX_PLAN];
	std::list<LabelledEffect*>* ef_mutexes_b[IPP_MAX_PLAN];
	std::list<LabelledFormula*>* b_goal_levels[IPP_MAX_PLAN];

	int plan_length;
	RelaxedPlan* next;

	double getAverageNumActions();
	DdNode* getStatesEntered(int level);// 到达任意层goal的状态集合
	double getRelaxedConformantCost();
	double restrictRP(DdNode* state);// 计算当状态覆盖action_level多少个动作
	int isMutex(EfNode* e1, EfNode* e2, int time);// 根据效果的互斥列表判断
	int partitionBreaksMutex(std::list<DdNode*>* parts, int time);
	void computeStaticMutexes();
	double getExpectedCost();
	int getRelaxedNumEffects();// 计算relaxed plan中条件/无条件effect的总数
	void unsetEffectRPFlags();// 清空action中EffectNode的 in relaxed plan标记
	/*   void insertEfs(std::set<LabelledElement*>* efs, int k); */
	int getEffectInteractionFactor();// 计算relaxed plan中最大的一层mutex数。
	// int numNonNoops(set<LabelledElement*>* ops);// not define
	int getRelaxedConformantNumActions();// 计算relaxed plan的长度
	void display();
	void push_up_actions();
//	double removeActsToMeetTau(double, double);

	DdNode* particleProbabilityMass(DdNode *belief,
			DdNode* stateParticleMap,
			DdNode* goal_particles);
	double extrapolateCost(DdNode* particleMass, double tau);
	RelaxedPlan(RelaxedPlan*);
	RelaxedPlan(int length, RelaxedPlan* nxt) :
		plan_length(length)
		{

		for (int i = 0; i < plan_length; i++){
			action_levels[i] = new std::set<LabelledAction*>();
			b_goal_levels[i] = new std::list<LabelledFormula*>();
			effect_levels[i] = new std::set<LabelledEffect*>();
			ef_mutexes_a[i] = NULL;
			ef_mutexes_b[i] = NULL;
			partitions[i]=NULL;
		}
		next = nxt;
		}
	~RelaxedPlan();

};
/**
 * 轻量化relaxed plan
 */
class RelaxedPlanLite{
public:
	RelaxedPlanLite(){
		for(int i = 0; i < IPP_MAX_PLAN; i++){
			actions[i] = new std::set<int>();
			propositions[i] = new std::set<int>();
		}
	}
	~RelaxedPlanLite(){
		for(int i = 0; i < IPP_MAX_PLAN; i++){
			if(actions[i])
				delete actions[i];
			if(propositions[i])
				delete propositions[i];
		}
	}

	void unionRP(RelaxedPlan* rp);
	double h_value();
	double computeInteraction(RelaxedPlanLite* rp);
	std::map<int, std::set<int>*> actions;
	std::map<int, std::set<int>*> propositions;
};

#endif
