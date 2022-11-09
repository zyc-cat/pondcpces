#include "relaxedPlan.h"
#include <stdio.h>
#include "cudd.h"
#include "cudd/dd.h"
#include "globals.h"
#include "math.h"
#include "float.h"
#include <algorithm>
using namespace std;

//DdNode* addWorldsUntilCostIncreases(double tau, int graph_levels){
//	//want the set of worlds that gives enough probability of
//	//satisfaction but with minimal cost
//
//	//algorithm is to greedily add worlds based on
//	//propagated cost.
//
//	DdNode *worldsToSupport = Cudd_ReadLogicZero(manager);
//	Cudd_Ref(worldsToSupport);
//	double goalCost = 0.0;
//	double newGoalCost = 0.0;
//	double sample_cost, min_sample_cost;
//	DdNode* min_sample;
//	int numWorldsAdded = 0;
//	int numSupporters = 0.0;
//	double allowableIncrease = 0.0;
//
//	while ((newGoalCost - goalCost) < allowableIncrease ||
//			(double)numSupporters/(double)NUMBER_OF_MGS < tau){
//
//		goalCost = newGoalCost;
//		//find most costly sample
//		min_sample_cost = DBL_MAX;
//		for(list<DdNode*>::iterator i = samples.begin();
//		i != samples.end(); i++){
//
//			if(!bdd_entailed(manager, *i, goal_samples) ||
//					bdd_entailed(manager, *i, worldsToSupport))
//				continue;
//
//			DdNode *tmp = Cudd_bddOr(manager, *i, worldsToSupport);
//			Cudd_Ref(tmp);
//			sample_cost = costOfGoal(graph_levels,tmp);
//			Cudd_RecursiveDeref(manager, tmp);
//
//			if(sample_cost < min_sample_cost || !min_sample){
//				min_sample_cost = sample_cost;
//				min_sample = *i;
//			}
//		}
//
//		DdNode *newWorldsToSupport = Cudd_bddOr(manager, worldsToSupport,
//				min_sample);
//		Cudd_Ref(newWorldsToSupport);
//		newGoalCost = costOfGoal(graph_levels, newWorldsToSupport);
//
//		if(newGoalCost - goalCost < allowableIncrease ||
//				(double)numSupporters/(double)samples.size() < tau){
//			Cudd_RecursiveDeref(manager, worldsToSupport);
//			worldsToSupport = newWorldsToSupport;
//			Cudd_Ref(worldsToSupport);
//			numWorldsAdded++;
//			numSupporters++;
//		}
//		Cudd_RecursiveDeref(manager, newWorldsToSupport);
//	}
//
//
//
//	//   cout << "Added worlds: " << numWorldsAdded
//	//        << " Cost: " << goalCost
//	//        << " PR: " << (double)numSupporters/(double)samples.size()
//	//        << endl;
//
//	return worldsToSupport;
//
//
//}

//DdNode* removeWorldsToImproveCost(double tau, int graph_levels){
//	//want the set of worlds that gives enough probability of
//	//satisfaction but with minimal cost
//
//	//algorithm is to greedily remove worlds based on
//	//propagated cost.  If can't remove a group to decrease cost,
//	//then stop
//
//	DdNode *worldsToSupport = goal_samples;
//	Cudd_Ref(worldsToSupport);
//	double goalCost = costOfGoal(graph_levels, worldsToSupport);
//	double newGoalCost = goalCost;
//	double sample_cost, max_sample_cost;
//	DdNode* max_sample;
//	int numWorldsRemoved = 0;
//	int numSupporters = countParticles(worldsToSupport);
//
//
//	while (newGoalCost != goalCost &&
//			(double)numSupporters/(double)NUMBER_OF_MGS >= tau){
//		goalCost = newGoalCost;
//		//find most costly sample
//		max_sample_cost = 0;
//		for(list<DdNode*>::iterator i = samples.begin();
//		i != samples.end(); i++){
//			if(!bdd_entailed(manager, *i, worldsToSupport))
//				continue;
//			sample_cost = goal_sample_costs[*i];
//			if(sample_cost > max_sample_cost || !max_sample){
//				max_sample_cost = sample_cost;
//				max_sample = *i;
//			}
//		}
//		DdNode *newWorldsToSupport = Cudd_bddAnd(manager, worldsToSupport,
//				Cudd_Not(max_sample));
//		Cudd_Ref(newWorldsToSupport);
//		newGoalCost = costOfGoal(graph_levels, newWorldsToSupport);
//
//		if(newGoalCost < goalCost){
//
//			Cudd_RecursiveDeref(manager, worldsToSupport);
//			worldsToSupport = newWorldsToSupport;
//			Cudd_Ref(worldsToSupport);
//			numWorldsRemoved++;
//			numSupporters--;
//		}
//		Cudd_RecursiveDeref(manager, newWorldsToSupport);
//	}
//	cout << "Removed worlds: " << numWorldsRemoved
//	<< " Cost: " << goalCost
//	<< " PR: " << (double)numSupporters/(double)NUMBER_OF_MGS
//	<< endl;
//
//	return worldsToSupport;
//
//
//}
//
//DdNode* removeWorlds(double tau, double MaxParticles){
//	int particlesNeeded = ceil(tau*MaxParticles);
//	int particlesHave=countParticles(goal_samples);
//	int particlesToRemove = particlesHave-particlesNeeded;
//	DdNode* keepers = goal_samples;
//	Cudd_Ref(keepers);
//
//	//   //lexically remove samples
//	//   for(list<DdNode*>::iterator i = samples.begin();
//	//       i != samples.end() && particlesToRemove > 0; i++){
//	//     DdNode *tmp = Cudd_bddAnd(manager, keepers, Cudd_Not(*i));
//	//     Cudd_Ref(tmp);
//	//     Cudd_RecursiveDeref(manager, keepers);
//	//     keepers= tmp;
//	//     Cudd_Ref(keepers);
//	//     Cudd_RecursiveDeref(manager, tmp);
//
//	//     particlesToRemove--;
//	//   }
//
//	//remove samples based on cost
//	while(particlesToRemove > 0){
//		double max_sample_cost = -1, sample_cost;
//		DdNode* max_sample = NULL;
//
//		for(list<DdNode*>::iterator i = samples.begin();
//		i != samples.end(); i++){
//			if(!bdd_entailed(manager, *i, keepers))
//				continue;
//			sample_cost = goal_sample_costs[*i];
//			if(sample_cost > max_sample_cost || !max_sample){
//				max_sample_cost = sample_cost;
//				max_sample = *i;
//			}
//		}
//		//     cout << "removing "<<endl;
//		//     printBDD(max_sample);
//
//		DdNode *tmp = Cudd_bddAnd(manager, keepers, Cudd_Not(max_sample));
//		Cudd_Ref(tmp);
//		Cudd_RecursiveDeref(manager, keepers);
//		keepers= tmp;
//		Cudd_Ref(keepers);
//		Cudd_RecursiveDeref(manager, tmp);
//
//
//		particlesToRemove--;
//	}
//
//
//	//Cudd_RecursiveDeref(manager, keepers);
//	return keepers;
//}

//double RelaxedPlan::removeActsToMeetTau(double tau, double MaxParticles){
//	//find a set of particles such that their cost is
//	//minimal and the ratio of their size to the total number of particles
//	//is as close to tau as possible
//	double value = 0.0;
//
//	DdNode *keepers = removeWorlds(tau, MaxParticles);
//	Cudd_Ref(keepers);
//
//	//recompute costs
//	for(int j = 0; j < plan_length; j++){
//		std::set<LabelledEffect*>::iterator i = effect_levels[j]->begin();
//		for(;i!= effect_levels[j]->end(); ++i){
//			DdNode* tmp = Cudd_bddAnd(manager, (*i)->label, keepers);
//			Cudd_Ref(tmp);
//			if(!((EfNode*)(*i)->elt)->op->is_noop &&
//					tmp != Cudd_ReadLogicZero(manager)){
//				value += -1*((EfNode*)(*i)->elt)->effect->reward *
//				((EfNode*)(*i)->elt)->effect->probability_sum;
//			}
//			Cudd_RecursiveDeref(manager, tmp);
//		}
//	}
//	Cudd_RecursiveDeref(manager, keepers);
//
//
//	return value;
//}


extern int num_new_labels;
/**
 * 计算后继状态变量，不确定的var个数
 */
int num_particles_in_minterm(DdNode* dd){
	int num = 1;
	for(int i = 0; i < num_new_labels; i++){
		if(!(Cudd_bddIsVarEssential(manager, dd, i*2+1, 0) ||
				Cudd_bddIsVarEssential(manager, dd, i*2+1, 1)))
			num *= 2;
	}
	return num;
}
/**
 * 
 */
double get_particle_sum(DdNode* dd){
	list<double> values;
	list<DdNode*> cubes;
	double rew = 0.0;
	if(dd != Cudd_ReadZero(manager)){
		get_cubes(dd, &cubes, &values);
		list<DdNode*>::iterator c = cubes.begin();
		for(list<double>::iterator r = values.begin(); r != values.end() && c!= cubes.end(); r++,c++){
			rew += *r * num_particles_in_minterm(*c);
			//printBDD(*c);
			//  cout << "num_states = " << num_states_in_minterm(*c)<<endl;
			Cudd_RecursiveDeref(manager, *c);
		}
	}
	return rew;
}

DdNode* particle_normalize(DdNode *dd){
	double total = get_particle_sum(dd);
	DdNode* fr = Cudd_addConst(manager, total);
	Cudd_Ref(fr);
	DdNode *fr1 = Cudd_addApply(manager, Cudd_addDivide, dd, fr);
	Cudd_Ref(fr1);
	Cudd_RecursiveDeref(manager, fr);
	return fr1;
}


//takes belief state and asseses the probability of each particle,
//given the path it takes in the relaxed plan, returns an ADD with
//the probability associated with each accepting particle.
DdNode* RelaxedPlan::particleProbabilityMass(DdNode *belief,
		DdNode* stateParticleMap,
		DdNode* goal_samples){

	//     printBDD(belief);


	DdNode *goalParticles = Cudd_bddAnd(manager, goal_samples, stateParticleMap);
	Cudd_Ref(goalParticles);


	//  initialMass = belief * stateParticleMap , then existabstr the states
	DdNode *addStateParticleMap = Cudd_BddToAdd(manager, goalParticles);
	Cudd_Ref(addStateParticleMap);

	//This is the probabilityMass of each particle before the relaxed plan
	DdNode *initialMass = Cudd_addApply(manager, Cudd_addTimes, belief, addStateParticleMap);
	Cudd_Ref(initialMass);

	//   cout << "initialMass: " << endl;
	//   printBDD(initialMass);


	DdNode* ab = Cudd_addExistAbstract(manager, initialMass, current_state_cube);
	Cudd_Ref(ab);
	//This will hold resulting mass of each particle
	DdNode *particleMass = particle_normalize(ab);
	Cudd_Ref(particleMass);

	//printBDD(particleMass);

	DdNode* stepMass;
	for(int i = 0; i < plan_length; i++){
		stepMass = initialMass;//Cudd_ReadOne(manager);
		Cudd_Ref(stepMass);
		for(std::set<LabelledEffect*>::iterator j = effect_levels[i]->begin();
		j != effect_levels[i]->end(); j++){

			if((*j)->elt->op->is_noop){
				continue;
			}
			//       cout << "stepMass:"<<endl;
			//       printBDD(stepMass);

			DdNode* effPr = Cudd_addConst(manager, (*j)->elt->effect->probability_sum);
			Cudd_Ref(effPr);
			DdNode *effLabel = Cudd_BddToAdd(manager, (*j)->label);
			Cudd_Ref(effLabel);
			DdNode *effMass = Cudd_addApply(manager, Cudd_addTimes, effLabel, effPr);
			Cudd_Ref(effMass);

			//       cout << "effMass:"<<endl;
			//       printBDD(effMass);


			DdNode* commonMass = Cudd_addApply(manager, Cudd_addTimes, stepMass, effMass);
			Cudd_Ref(commonMass);



			DdNode *a = Cudd_addBddStrictThreshold(manager, stepMass, 0.0);
			Cudd_Ref(a);
			DdNode *b = Cudd_addBddStrictThreshold(manager, effMass, 0.0);
			Cudd_Ref(b);
			DdNode *c = Cudd_bddAnd(manager, a, Cudd_Not(b));
			Cudd_Ref(c);
			DdNode *d = Cudd_BddToAdd(manager, c);
			Cudd_Ref(d);

			DdNode* uncommonMass = Cudd_addApply(manager, Cudd_addTimes, d, stepMass);
			Cudd_Ref(uncommonMass);




			DdNode* tmp = Cudd_addApply(manager, Cudd_addPlus, commonMass, uncommonMass);
			Cudd_Ref(tmp);
			Cudd_RecursiveDeref(manager, stepMass);
			stepMass = tmp;
			Cudd_Ref(stepMass);
			Cudd_RecursiveDeref(manager, tmp);
			Cudd_RecursiveDeref(manager, effPr);
			Cudd_RecursiveDeref(manager, effLabel);
			Cudd_RecursiveDeref(manager, effMass);
			Cudd_RecursiveDeref(manager, commonMass);
			Cudd_RecursiveDeref(manager, uncommonMass);
			Cudd_RecursiveDeref(manager, a);
			Cudd_RecursiveDeref(manager, b);
			Cudd_RecursiveDeref(manager, c);
			Cudd_RecursiveDeref(manager, d);
		}


		DdNode *tmp1 = Cudd_addApply(manager, Cudd_addTimes, particleMass, stepMass);
		Cudd_Ref(tmp1);
		Cudd_RecursiveDeref(manager, particleMass);
		particleMass = tmp1;
		Cudd_Ref(particleMass);
		Cudd_RecursiveDeref(manager, tmp1);
		Cudd_RecursiveDeref(manager, stepMass);

		//    cout << "particle mass:"<<endl;
		//    printBDD(particleMass);
	}




	return particleMass;

}


//adjust the heuristic cost of a relaxed plan to measure the cost needed
//to actually meet tau
double RelaxedPlan::extrapolateCost(DdNode* particleMass, double tau){

	DdNode* pr = Cudd_addExistAbstract(manager, particleMass, particle_cube);
	Cudd_Ref(pr);

	int* x = new int[2*num_alt_facts];
	CUDD_VALUE_TYPE value;
	DdGen* gen = Cudd_FirstCube(manager, pr, &x, &value);
	delete [] x;


	double h = getRelaxedConformantCost();

	double adjustment = (h*tau)/(value);

	//    cout << "total Mass = " << value << endl;
	//    cout << "h = " << h << endl;
	//    cout << "adjustment = " << adjustment << endl;
	//    cout << "new h = " << tau/adjustment << endl;

	if(value > 1)
		exit(0);

	return adjustment;

}


void getRPStateFormulasToEvaluate(list<list<DdNode*>* >* state_formulas,
		int level,
		RelaxedPlan *rp,
		int worldCheck){


	list<DdNode*>* tmpl;
	DdNode* subgoal, *subgoal1, *tmp;

	if(worldCheck){
		for(int i = 0; i < num_alt_facts; i++){// 考虑每个fact
			tmpl = new list<DdNode*>();
			// 考虑level所在的目标
			for(list<LabelledFormula*>::iterator s = rp->b_goal_levels[level]->begin();
			s != rp->b_goal_levels[level]->end(); s++){
				subgoal = ((DdNode*)(*s)->elt);
				//      Cudd_Ref(subgoal);
				//	worlds = (*s)->label;
				//	Cudd_Ref(worlds);
				if(subgoal == Cudd_bddIthVar(manager, 2*i))
					tmpl->push_back(Cudd_bddIthVar(manager, 2*i));
				else if (subgoal == Cudd_Not(Cudd_bddIthVar(manager, 2*i)))
					tmpl->push_back(Cudd_Not(Cudd_bddIthVar(manager, 2*i)));

			}
			state_formulas->push_back(tmpl);// 获取每个fact相关BDD
		}
	}
	else{
		//this needs work because the subgoals are not single literals, rather
		//they are conjunctive sets of literals
		for(list<LabelledFormula*>::iterator s = rp->b_goal_levels[level]->begin();
		s != rp->b_goal_levels[level]->end(); s++){
			list<LabelledFormula*>::iterator r = s;
			r++;// 考虑下一个
			for(;r != rp->b_goal_levels[level]->end(); r++){
				subgoal = ((DdNode*)(*s)->elt);
				subgoal1 = ((DdNode*)(*r)->elt);
				printBDD(subgoal);
				printBDD(subgoal1);
				if(subgoal == Cudd_Not(subgoal1))// 两个互斥，忽略
					continue;
				if(Cudd_ReadLogicZero(manager) == Cudd_bddAnd(manager,
						((DdNode*)(*s)->label),
						((DdNode*)(*r)->label))){// momo007 2022.09.21 这里应该有个bug，但没调用过
					tmp = Cudd_bddAnd(manager, subgoal, subgoal1);
					Cudd_Ref(tmp);
					tmpl->push_back(tmp);
				}
			}
		}
	}


}

// 这里最终结果应该是到达plan任一层的所有state的集合，用BDD表示。
//The labelled subgoals define the states reached at a level.
//For a state to be reached in the relaxed plan, it must be the
//case that all of its literals can be reached in the same worlds
DdNode* RelaxedPlan::getStatesEntered(int levels){
	// cout << "get entered"<<endl;
	DdNode* result = Cudd_ReadLogicZero(manager), *lresult, *tres, *tmp, *tmp1;

	list<list<DdNode*>* > state_formulas;

	for (int k = 0; k < plan_length //&& k <= levels
	; k++){
		lresult = Cudd_ReadOne(manager);
		Cudd_Ref(lresult);


		state_formulas.clear();
		getRPStateFormulasToEvaluate(&state_formulas, k, this, 1);
		// 将每个fact的BDD析取，随后所有fact的BDD合取到lresult
		// cout << "got k = " << k <<endl;
		for(list<list<DdNode*>* >::iterator i = state_formulas.begin();
		i != state_formulas.end(); i++){
			tres = Cudd_ReadLogicZero(manager);
			Cudd_Ref(tres);
			for(list<DdNode*>::iterator j = (*i)->begin();
			j != (*i)->end(); j++){
				//tmp = labelBDD(source, *j, level);
				//Cudd_Ref(tmp);

				//if(tmp != Cudd_ReadLogicZero(manager)){
				//there exists some state in source that reaches
				//every state described by *j
				tmp1 = Cudd_bddOr(manager, tres, *j);
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, tres);
				tres = tmp1;
				Cudd_Ref(tres);
				Cudd_RecursiveDeref(manager, tmp1);
				//}
				//Cudd_RecursiveDeref(manager, tmp);
			}

			if(tres != Cudd_ReadLogicZero(manager)){
				//	printBDD(tres);
				tmp1 = Cudd_bddAnd(manager, lresult, tres);
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, lresult);
				lresult = tmp1;
				Cudd_Ref(lresult);
				Cudd_RecursiveDeref(manager, tmp1);
				Cudd_RecursiveDeref(manager, tres);
			}
			delete *i;
		}


		//    state_formulas.clear();
		//     getRPStateFormulasToEvaluate(&state_formulas, k, this, 0);
		//     for(list<list<DdNode*>* >::iterator i = state_formulas.begin();
		// 	i != state_formulas.end(); i++){
		//       for(list<DdNode*>::iterator j = (*i)->begin();
		// 	  j != (*i)->end(); j++){
		// 	tmp1 = Cudd_bddAnd(manager, Cudd_Not(*j), lresult);
		// 	Cudd_Ref(tmp1);
		// 	Cudd_RecursiveDeref(manager, lresult);
		// 	lresult = tmp1;
		// 	Cudd_Ref(lresult);
		// 	Cudd_RecursiveDeref(manager, tmp1);
		// 	Cudd_RecursiveDeref(manager, *j);
		//       }
		//       delete *i;
		//     }


		// 所有层的fact的BDD进行析取
		if(lresult != Cudd_ReadOne(manager)){
			//printBDD(lresult);

			tmp1 = Cudd_bddOr(manager, result, lresult);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, result);
			result = tmp1;
			Cudd_Ref(result);
			Cudd_RecursiveDeref(manager, tmp1);
			Cudd_RecursiveDeref(manager, tres);
		}


		//     b_goal_levels[i] = new std::list<LabelledFormula*>();
		//     for(list<LabelledFormula*>::iterator s = b_goal_levels[i]->begin();
		// 	s != b_goal_levels[i]->end(); s++){
		//       subgoal = ((DdNode*)(*s)->elt);
		//       Cudd_Ref(subgoal);
		//       worlds = (*s)->label;
		//       Cudd_Ref(worlds);
		//     }



	}
	return result;

}

RelaxedPlan::RelaxedPlan(RelaxedPlan* rp){
	plan_length = rp->plan_length;
	for (int i = 0; i < plan_length; i++){
		action_levels[i] = new std::set<LabelledAction*>();
		for(std::set<LabelledAction*>::iterator j = rp->action_levels[i]->begin();
			j != rp->action_levels[i]->end(); j++)
			action_levels[i]->insert(new LabelledAction(*j));


		b_goal_levels[i] = new std::list<LabelledFormula*>();
		for(std::list<LabelledFormula*>::iterator j = rp->b_goal_levels[i]->begin();
			j != rp->b_goal_levels[i]->end(); j++)
			b_goal_levels[i]->push_back(new LabelledFormula(*j));

		effect_levels[i] = new std::set<LabelledEffect*>();
		for(std::set<LabelledEffect*>::iterator j = rp->effect_levels[i]->begin();
			j != rp->effect_levels[i]->end(); j++)
			effect_levels[i]->insert(new LabelledEffect(*j));


		ef_mutexes_a[i] = NULL;
		ef_mutexes_b[i] = NULL;
		partitions[i]=NULL;
	}

}

/**
 * 计算每层动作Label和当前状态有交集的动作个数
 */
double RelaxedPlan::restrictRP(DdNode* dd){
	double h =0.0;
	DdNode* fr;
	//  DdNode* d = overApprox(manager, dd);
	//  Cudd_Ref(d);
	// cout << "HI restrict"<<endl;
	for(int i = 0; i < plan_length; i++){
		//cout << "i = " << i << flush << endl;
		int foundInLev = 0;
		for(set<LabelledAction*>::iterator j = action_levels[i]->begin();
		j != action_levels[i]->end(); j++){

			if((*j)->elt->is_noop)
				continue;

			//      if(0)
			//	fr = Cudd_bddAnd(manager, (*j)->label, dd);
			//      else{
			//	fr = Cudd_underApproxAnd(manager, dd, (*j)->label);
			//      }

			//      fr = Cudd_bddClippingAnd(manager, dd, (*j)->label, 1, 1);
			fr = Cudd_bddIntersect(manager, dd, (*j)->label);
			Cudd_Ref(fr);
			if(Cudd_ReadLogicZero(manager) != fr){
				//if(Cudd_bddLeq(manager, dd, (*j)->label)){
				//	cout << (*j)->elt->name << endl;
				//cout << "+"<<endl;
				foundInLev++;
				//h+=1.0;
			}
			Cudd_RecursiveDeref(manager, fr);

		}
		h+=foundInLev;
		if(foundInLev == 0)
			break;
		//cout << " h = " << h << flush << endl;
	}
	//cout << "BYE restrict h = " << h<<endl;
	return h;
}

/**
 * 考虑4种不同的mutex情况
 */
void RelaxedPlan::computeStaticMutexes(){
	set<LabelledEffect*>::iterator eff1, eff2;

	int num_mux=0;

	//  cout << "computing RP Mutexes"<< endl;
	for(int i = 0; i < plan_length; i++){
		ef_mutexes_a[i] = new list<LabelledEffect*>();
		ef_mutexes_b[i] = new list<LabelledEffect*>();

		//    cout << "TIME = "<< i<<endl;
		//two effects are static mutex if:
		//1: actions have inconsistent enabling preconditions
		//2: effects have inconsistent pre/eff
		//3: effects have inconsistent eff/eff
		//4: effects have inconsistent pre/pre
		for(eff1 = effect_levels[i]->begin(); eff1 != effect_levels[i]->end();eff1++){
			//      cout << "HI"<<endl;
			eff2 = eff1;
			eff2++;
			// 考虑同层不同的effect
			for(; eff2 != effect_levels[i]->end();eff2++){

				if(//1
						bdd_is_one(manager,
								Cudd_Not(bdd_and(manager,
										((EfNode*)(*eff1)->elt)->op->b_pre,
										((EfNode*)(*eff2)->elt)->op->b_pre))) ||
										//2
										bdd_is_one(manager,
												Cudd_Not(bdd_and(manager,
														((EfNode*)(*eff1)->elt)->effect->ant->b,
														((EfNode*)(*eff2)->elt)->effect->cons->b))) ||
														//2
														bdd_is_one(manager,
																Cudd_Not(bdd_and(manager,
																		((EfNode*)(*eff2)->elt)->effect->ant->b,
																		((EfNode*)(*eff1)->elt)->effect->cons->b))) ||
																		//3
																		bdd_is_one(manager,
																				Cudd_Not(bdd_and(manager,
																						((EfNode*)(*eff1)->elt)->effect->cons->b,
																						((EfNode*)(*eff2)->elt)->effect->cons->b))) ||
																						//4
																						bdd_is_one(manager,
																								Cudd_Not(bdd_and(manager,
																										((EfNode*)(*eff1)->elt)->effect->ant->b,
																										((EfNode*)(*eff2)->elt)->effect->ant->b)))
				){
					// 	  cout << "marking mutex " << ((EfNode*)(*eff1)->elt)->op->name
					// 	       << " and " << ((EfNode*)(*eff2)->elt)->op->name <<endl;
					ef_mutexes_a[i]->push_back(*eff1);
					ef_mutexes_b[i]->push_back(*eff2);
					num_mux++;
				}
				// 	cout << isMutex(((EfNode*)(*eff1)->elt), ((EfNode*)(*eff2)->elt), i) << " "
				// 	     << isMutex(((EfNode*)(*eff2)->elt), ((EfNode*)(*eff1)->elt), i) <<endl;
			}
		}
	}
	//  cout << "got "<<num_mux << " mutexes" <<endl;
}

int RelaxedPlan::isMutex(EfNode* eff1, EfNode* eff2, int time){
	list<LabelledEffect*>::iterator e1, e2;

	e1 = ef_mutexes_a[time]->begin();
	e2 = ef_mutexes_b[time]->begin();
	// 考虑互斥信息
	for(;(e1 != ef_mutexes_a[time]->end() &&
			e2 != ef_mutexes_b[time]->end()) ; e1++){
		if((((EfNode*)(*e1)->elt)->alt_index == eff1->alt_index &&
				((EfNode*)(*e2)->elt)->alt_index == eff2->alt_index) ||
				(((EfNode*)(*e1)->elt)->alt_index == eff2->alt_index &&
						((EfNode*)(*e2)->elt)->alt_index == eff1->alt_index))
			return TRUE;
		e2++;
	}
	return FALSE;

}

/**
 * 通过partiton对第time层的mutex effect进行简化
 */
int RelaxedPlan::partitionBreaksMutex(list<DdNode*>* parts, int time){
	//check if any of the mutexes at time or later are between effects that
	//fall into different elements of parts.
	list<LabelledEffect*>::iterator e1, e2, t1, t2;
	list<DdNode*>::iterator p1, p2;
	int gotOne = 0, broke = 0;
	//  cout << "enter does part mutex?, time = " << time << " parts size = " << parts->size()<<endl;

	for(int i = time; i < plan_length; i++){
		//   cout << "i = " << i<<endl;
		//  cout << " mutex size = " << ef_mutexes_a[i]->size() << " " <<ef_mutexes_b[i]->size() <<endl;
		//   if(ef_mutexes_a[i]->size() ==0 )
		//       continue;

		e1 = ef_mutexes_a[i]->begin();
		e2 = ef_mutexes_b[i]->begin();
		for(;(e1 != ef_mutexes_a[i]->end() &&
				e2 != ef_mutexes_b[i]->end()) ; ){


			broke = 0;
			// 考虑两个不同的partition
			for(p1 = parts->begin(); p1 != parts->end(); p1++){
				for(p2 = parts->begin(); p2 != parts->end(); p2++){
					// cout << "HI"<<endl;
					if(p1 == p2)
						continue;
					// 存在一对part将将个mutex effect分开
					if(
						!bdd_is_one(manager,// e1 label和p1有交集
								Cudd_Not(bdd_and(manager, (*e1)->label, (*p1)))) &&
								!bdd_is_one(manager,// e2 laebl和p2有交集
										Cudd_Not(bdd_and(manager, (*e2)->label, (*p2)))) &&
										bdd_is_one(manager,// e1 label和p2没交集
												Cudd_Not(bdd_and(manager, (*e1)->label, (*p2)))) &&
												bdd_is_one(manager,// e2 label和p1没交集
														Cudd_Not(bdd_and(manager, (*e2)->label, (*p1))))
					){
						//	       	    cout << "T"<<endl;
						//return TRUE;
						broke = 1;
						gotOne = 1;
						break;
					}
					//   else
					// 	    cout << "N"<<endl;
				}
				if(broke)
					break;
			}
			if(broke){
				//	cout << "Broke Mutex"<<endl;
				t1 = e1; t1++;//考虑处理下一对effect
				t2 = e2; t2++;
				ef_mutexes_a[i]->erase(e1);// 通过part不再mutex，移除
				ef_mutexes_b[i]->erase(e2);
				e1 = t1;
				e2 = t2;
				//     cout << " mutex size = " << ef_mutexes_a[i]->size() << " " <<ef_mutexes_b[i]->size() <<endl;
			}
			else{
				e1++;
				e2++;
			}
		}
	}
	// cout << "F"<<endl;

	return gotOne;

}

double RelaxedPlan::getExpectedCost(){
	double exp_cost = 0.0;
	double part_cost = 0.0;
	std::set<LabelledAction*>::iterator j;
	std::list<DdNode*>::iterator p;

	EfNode* tmp_ef;
	printf("alt_act_cost is not defined\n");
	assert(0);
	//   cout << "PLan length = " << plan_length<<endl;
	for(int i = 0; (i < plan_length ); i++){
		if(partitions[i]){
			for(p = partitions[i]->begin(); p != partitions[i]->end(); p++){
				part_cost = 0.0;
				for(j = action_levels[i]->begin();j!=action_levels[i]->end();++j){
					if(!((OpNode*)(*j)->elt)->is_noop &&
							!bdd_is_one(manager, Cudd_Not(bdd_and(manager, (*j)->label, *p)))){
						part_cost += alt_act_costs[((OpNode*)(*j)->elt)->alt_index];
					}
				}
				exp_cost += part_cost/partitions[i]->size();// 计算partition后的均值
			}
		}
		else{
			//       cout << "No Parts"<<endl;
			//       exit(0);
			for(j = action_levels[i]->begin();j!=action_levels[i]->end();++j){
				if(!((OpNode*)(*j)->elt)->is_noop ){
					exp_cost += alt_act_costs[((OpNode*)(*j)->elt)->alt_index];
				}
			}
		}
	}
	//   cout << "Num Effs in RP: " << num_effs << endl;
	return exp_cost;

}

void RelaxedPlan::push_up_actions(){
	printf("RelaxedPlan::push_up_actions() without implement");
	assert(0);
	//some actions are counted extra because they are done
	//in different worlds at different levels
	//so we push actions later in the RP so that there is more
	//overlap of the same action in different worlds

	// cout << "Pushing up:" <<endl;
	//   display();

	//  for(int i = 0; i < plan_length; i++){
	//     for(set<LabelledElement*>::iterator a = action_levels[i]->begin();
	// 	a != action_levels[i]->end();
	// 	a++){

	//       //find highest level to push up action before invalidating causal links
	//       for(int j = i+1; j < plan_length; j++){

	//       }
	//     }
	//   }



}


int RelaxedPlan::getRelaxedNumEffects(){
	int num_effs = 0;
	std::set<LabelledAction*>::iterator j;
	EfNode* tmp_ef;
	  cout << "PLan graph level = " << plan_length<<endl;
	for(int i = 0; (i < plan_length ); i++){
		j = action_levels[i]->begin();
		for(;j!=action_levels[i]->end();++j){
			if(!((OpNode*)(*j)->elt)->is_noop){
				if(((OpNode*)(*j)->elt)->unconditional->in_rp){// momo007 2022.09.21 unconditional只有一个？
					num_effs++;
					//(*j)->unconditional->in_rp = FALSE;
				}
				tmp_ef = ((OpNode*)(*j)->elt)->conditionals;
				while(tmp_ef) {
					if(tmp_ef->in_rp){
						num_effs++;
						//	   tmp_ef->in_rp = FALSE;
					}
					tmp_ef = tmp_ef->next;
				}
			}
		}
	}
	cout << "Num Effs in RP: " << num_effs << endl;
	return num_effs;
}
void RelaxedPlan::unsetEffectRPFlags(){

	std::set<LabelledAction*>::iterator j;
	EfNode* tmp_ef;

	//     cout << "unset " <<endl;
	//     exit(0);

	for(int i = 0; (i < plan_length); i++){
		j = action_levels[i]->begin();
		for(;j!=action_levels[i]->end();++j){
			if(((OpNode*)(*j)->elt)->unconditional->in_rp){
				((OpNode*)(*j)->elt)->unconditional->effect->original->in_rp = FALSE;
			}
			tmp_ef = ((OpNode*)(*j)->elt)->conditionals;
			while(tmp_ef) {
				if(tmp_ef->effect->original->in_rp){
					tmp_ef->effect->original->in_rp = FALSE;
				}
				tmp_ef = tmp_ef->next;
			}
		}
	}
}




//  void RelaxedPlan::insertEfs(set<LabelledElement*>* efs, int k){
//     //     cout << "Inserting effs in rp" <<endl;
//     DdNode* b = Cudd_ReadOne(manager);
//     DdNode* p = Cudd_ReadOne(manager);
//     EfNode* tmp_eff = NULL;
//     DdNode* tmp;
//     //put all associated acts in action_list[k];
//     if(!efs){
//       b_goal_levels[k] =  b;
//       Cudd_Ref(b_goal_levels[k]);

//       return;
//     }

//     std::set<LabelledElement*>::iterator j = efs->begin();
//     for(;j!=efs->end();++j){
//       //cout << "j = " << (*j)->op->name << endl;
//       if(!((EfNode*)(*j)->elt)->op->is_noop){
// 	//	cout << "added act to rp " << (*j)->op->name << endl;
// 	action_levels[k]->insert(new LabelledElement((int*)((EfNode*)(*j)->elt)->op, (*j)->label, 0));
//       }
//       else{
// 	if(((EfNode*)(*j)->elt)->op->preconds->ft->positive)
// 	  tmp = bdd_and(manager, b, Cudd_bddIthVar(manager, ((EfNode*)(*j)->elt)->op->preconds->ft->index));
// 	else
// 	  tmp = bdd_and(manager, b, Cudd_Not(Cudd_bddIthVar(manager, ((EfNode*)(*j)->elt)->op->preconds->ft->index)));

// 	if(bdd_isnot_one(manager, Cudd_Not(tmp))){
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, b);
// 	  b = tmp;
// 	  Cudd_Ref(b);
// 	  //	printBDD(b);
// 	}
//       }
//     }



//     //disjoin all facts from (1) effect ants, and (2) exec preconds into b_goal_level

//     std::set<LabelledElement*>::iterator tmp_op = action_levels[k]->begin();
//     for(;(tmp_op != action_levels[k]->end() && k > 0); ++tmp_op){
//       //      cout<< (*tmp_op)->name<<endl;
//      FtEdge* tmp_pre;
//       for(int i = 0; i < 3; i++){
// 	//cout << "i = " << i << endl;


// 	switch(i) {
// 	case 0: { tmp_pre = ((OpNode*)(*tmp_op)->elt)->preconds; break;}
// 	case 1: { tmp_pre = ((OpNode*)(*tmp_op)->elt)->unconditional->conditions; break;}
// 	case 2: { tmp_eff = ((OpNode*)(*tmp_op)->elt)->conditionals; break;}
// 	}

// 	while(tmp_eff || i != 2){

// 	  if(i == 2  && tmp_eff)
// 	    tmp_pre = tmp_eff->conditions;

// 	  p = Cudd_ReadOne(manager);
// 	  while(tmp_pre){
// 	    //  printFact(tmp_pre->ft->index);
// 	    if(tmp_pre->ft->positive){
// 	      tmp = bdd_and(manager, p, Cudd_bddIthVar(manager, tmp_pre->ft->index));
// 	      Cudd_Ref(tmp);
// 	      Cudd_RecursiveDeref(manager, p);
// 	      p = tmp;
// 	      Cudd_Ref(p);
// 	    }
// 	    else{
// 	      tmp = bdd_and(manager, p, Cudd_bddIthVar(manager, tmp_pre->ft->index));
// 	      Cudd_Ref(tmp);
// 	      Cudd_RecursiveDeref(manager, p);
// 	      p = tmp;
// 	      Cudd_Ref(p);
// 	    }
// 	    //printBDD(p);
// 	    tmp_pre = tmp_pre->next;
// 	  }

// 	  if(bdd_isnot_one(manager, p)){

// 	    tmp = bdd_and(manager, b, p);
// 	    if(bdd_isnot_one(manager, Cudd_Not(tmp))){
// 	      Cudd_Ref(tmp);
// 	      Cudd_RecursiveDeref(manager, b);
// 	      b = tmp;
// 	      Cudd_Ref(b);
// 	    }
// 	  }
// 	  //  printBDD(b);

// 	  if(tmp_eff)
// 	    tmp_eff = tmp_eff->next;
// 	  else
// 	    break;

// 	}
//       }
//     }
//     //    cout << "Must now support "<<endl;
//     //    printBDD(b);
//     b_goal_levels[k] = b;
//     Cudd_Ref(b_goal_levels[k]);
//   }

int RelaxedPlan::getEffectInteractionFactor(){

	int return_val = 0;
	int max = -1;
	std::set<LabelledAction*>::iterator j;
	EfNode* tmp_ef, *tmp_ef1;

	//  cout << "RP: Getting effect interaction factor"<< endl;
	//    display();
	for(int i = 0; (i < plan_length); i++){
		j = action_levels[i]->begin();
		//      cout << "LOOKIN AT LEVEL " << i <<endl;
		for(;j!=action_levels[i]->end();++j){
			//	cout << "CHECK Uncond "<< endl;
			if(((OpNode*)(*j)->elt)->unconditional->in_rp){// unconditional effect和effect mutex计算
				//	  	  cout << "Uncond in rp" <<endl;
				//count mutexes with effects of same action
				tmp_ef = ((OpNode*)(*j)->elt)->conditionals;
				while(tmp_ef){
					//    printBDD((*j)->unconditional->info_at[i]->label->label);
					if(//tmp_ef != (*j)->unconditional &&
							tmp_ef->in_rp &&
							!tmp_ef->info_at[i]->is_dummy &&
							ARE_MUTEX_EFS(i, ((OpNode*)(*j)->elt)->unconditional, tmp_ef, ((OpNode*)(*j)->elt)->unconditional->info_at[i]->label->label, tmp_ef->info_at[i]->label->label)){
						// exit(0);
						return_val++;

					}
				}
			}
			tmp_ef = ((OpNode*)(*j)->elt)->conditionals;// 所有effect的mutex计算
			while(tmp_ef) {
				if(tmp_ef->in_rp && !tmp_ef->info_at[i]->is_dummy){
					//    cout << "COnd in rp"<<endl;
					//count mutexes with effects of same action
					tmp_ef1 = ((OpNode*)(*j)->elt)->conditionals;
					while(tmp_ef1){
						if(//tmp_ef != tmp_ef1 &&
								tmp_ef1->in_rp &&
								!tmp_ef1->info_at[i]->is_dummy &&
								ARE_MUTEX_EFS(i, tmp_ef, tmp_ef1, tmp_ef->info_at[i]->label->label, tmp_ef1->info_at[i]->label->label)){
							return_val++;
							//exit(0);
						}
						tmp_ef1 = tmp_ef1->next;
					}

				}
				tmp_ef = tmp_ef->next;
			}
		}
		if(max < return_val) {// 记录一层中最大mutex数
			max = return_val;
		}
		return_val = 0;
	}
	//   cout << "exit factor" <<endl;
	return max;//return_val;
}


int numNonNoopsCombineWorlds(set<LabelledAction*>* ops){
	int count = 0;

	std::set<LabelledAction*>::iterator i = ops->begin();
	// use the set to get the union operator
	std::set<OpNode*>* unlab_ops = new std::set<OpNode*>();


	for(;i!= ops->end(); ++i){
		if(!((OpNode*)(*i)->elt)->is_noop){
			unlab_ops->insert(((OpNode*)(*i)->elt));
		}
	}

	return unlab_ops->size();

}
double costNonNoopsCombineWorlds(set<LabelledAction*>* ops){
	double cost = 0;

	std::set<LabelledAction*>::iterator i = ops->begin();

	std::set<OpNode*>* unlab_ops = new std::set<OpNode*>();


	for(;i!= ops->end(); ++i){
	//	if(!((OpNode*)(*i)->elt)->is_noop && (*i)->label != Cudd_ReadLogicZero(manager)){
	      if(!((OpNode*)(*i)->elt)->is_noop){
		
#ifdef PPDDL_PARSER
		if(1 || strcmp((*i)->elt->name,"(noop)")!=0){
		   //(1||!my_problem->domain().requirements.rewards) && strcmp((*i)->elt->name,"(noop_action)")!=0){
		  ///		  	cout << "Adding cost of :" << ((OpNode*)(*i)->elt)->name << endl;
				cost += 1;// alt_act_costs[((OpNode*)(*i)->elt)->alt_index-1];
		}
			else
				cost += 0;
#else
			cost += alt_act_costs[((OpNode*)(*i)->elt)->alt_index];
#endif
		}
	}

	return cost;

}
int RelaxedPlan::getRelaxedConformantNumActions() {
	int num_acts = 0;
	   cout << "PLan graph level = " << plan_length<<endl;
	//    this->display();
	for(int i = 0; i < plan_length; i++)

		num_acts += numNonNoopsCombineWorlds(action_levels[i]);// 统计该层不重复noop action个数

	cout << "Num Acts in RP: " << num_acts << endl;
	return num_acts;
}

double EffRewardNonNoopsCombineWorlds(set<LabelledEffect*>* efs){
	double reward = 0;

	std::set<LabelledEffect*>::iterator i = efs->begin();

	for(;i!= efs->end(); ++i){
		if(!((EfNode*)(*i)->elt)->op->is_noop){
			//	cout << " " << (*i)->elt->alt_index << flush;
			// 	cout << "Adding cost of :"
			// 	     << ((EfNode*)(*i)->elt)->op->name
			// 	     << " " << ((EfNode*)(*i)->elt)->effect->reward
			// 	     << endl;
			reward += ((EfNode*)(*i)->elt)->effect->reward;// * ((EfNode*)(*i)->elt)->effect->probability_sum;
		}
	}
	//cout << endl;

	return -1.0*reward;

}

extern double gDiscount;
double RelaxedPlan::getRelaxedConformantCost() {
	double cost = 0;
	//   cout << "PLan length = " << plan_length<<endl;
	for(int i = 0; i < plan_length; i++){
	  //cout << i << " of " << plan_length << endl;
	  if(1|| !my_problem->domain().requirements.rewards)
	    cost += //pow(gDiscount, i)*
	      costNonNoopsCombineWorlds(action_levels[i]);
	  else
	    cost += EffRewardNonNoopsCombineWorlds(effect_levels[i]);
	}

	if(my_problem->domain().requirements.rewards){
		//      cout << cost << endl;
		if(PF_LUG){
			cost += //pow(gDiscount, plan_length) *
				((double)countParticles(goal_samples)/(double)NUMBER_OF_MGS)*total_goal_reward;
		}
		else
			cost += total_goal_reward;
		//cout << cost << endl;
	}


	/*     cout << "Num Acts in RP: " << num_acts << endl; */
	return cost;
}

double RelaxedPlan::getAverageNumActions(){
	double result = 0.0;

	DdNode* worldCounts = Cudd_addConst(manager, 0.0), *tmp, *tmp1;
	Cudd_Ref(worldCounts);
	// 考虑plan
	for(int i = 0; i < plan_length; i++){
		// 计算plan的所有action可达的world count
		for(std::set<LabelledAction*>::iterator j = action_levels[i]->begin();
		j!= action_levels[i]->end(); ++j){
			if(!((OpNode*)(*j)->elt)->is_noop){// 不是 noop
				tmp1 = Cudd_BddToAdd(manager, (*j)->label);
				Cudd_Ref(tmp1);
				tmp = Cudd_addApply(manager, Cudd_addPlus,// 累计world count
						worldCounts,
						tmp1);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, tmp1);
				Cudd_RecursiveDeref(manager, worldCounts);
				worldCounts = tmp;
				Cudd_Ref(worldCounts);
				Cudd_RecursiveDeref(manager, tmp);
			}
		}
	}


	list<double> values;
	list<DdNode*> cubes;
	get_cubes(worldCounts, &cubes, &values);
	// 获取全部的value
	for(list<double>::iterator i = values.begin();
	i != values.end(); i++){
		result += *i;
	}
	result /= (double)values.size();// 计算平均值

	for(list<DdNode*>::iterator i = cubes.begin();
	i != cubes.end(); i++){
		Cudd_RecursiveDeref(manager, *i);
	}

	return result;
}




void RelaxedPlan::display() {
	cout << "RP has #levels: " << plan_length << endl;

	//std::list<LabelledElement*>::iterator k;
	std::set<LabelledAction*>::iterator j;
	std::set<LabelledEffect*>::iterator e;
	list<DdNode*>::iterator p;
	for(int i = 0; i< plan_length; i++) {
		cout << "Level " << i << endl;
		//    for(k = goal_levels[i]->begin(); k!=goal_levels[i]->end(); k++){
		//  	print_BitVector( ((FactInfoPair*)(*k)->elt)->positive->vector, gft_vector_length  );
		//  	print_BitVector( ((FactInfoPair*)(*k)->elt)->negative->vector,gft_vector_length  );
		//  	cout << endl;
		// // 	printBDD((*k)->label);

		//        }
		//cout << endl;

		//       for(e = effect_levels[i]->begin(); e != effect_levels[i]->end(); e++)
		// 	cout << "ef: " << ((EfNode*)(*e)->elt)->effect->in_rp << endl;


		if(action_levels[i]){
			//	cout << "|act_lev| = " <<  action_levels[i]->size()<<endl;
			j = action_levels[i]->begin();
			for(;j != action_levels[i]->end(); ++j){
				//cout << "index: "<<((*j)->elt)->alt_index <<endl;
#ifdef PPDDL_PARSER
				if(!((OpNode*)(*j)->elt)->is_noop &&
						((*j)->elt)->alt_index-1 < num_alt_acts){
					if(COMPUTE_LABELS){

						// 	  DdNode* tmp1 = Cudd_BddToAdd(manager, (*j)->label);
						// 	  Cudd_Ref(tmp1);
						// 	  DdNode* tmp = Cudd_addApply(manager, Cudd_addTimes,
						// 				      tmp1,
						// 				      probabilisticLabels);
						// 	  Cudd_Ref(tmp);
						// 	  Cudd_RecursiveDeref(manager, tmp1);


						cout << ((*j)->elt)->name //<< "\t cost = "
						// 	       << alt_act_costs[((*j)->elt)->alt_index-1]
						// 	    //<< ", pr = " << exAbstractAllLabels(tmp, plan_length-1)
						<< endl<<flush;
						printBDD((*j)->label);
						// 	  //Cudd_RecursiveDeref(manager, tmp);
					}
					else{
						printf("the alt_act_cost is not defined");
						assert(0);
						cout << ((*j)->elt)->name << "\t cost = "
							 << alt_act_costs[((*j)->elt)->alt_index - 1]
							 << endl
							 << flush;
					}

				}
//				else{
//					cout << "Noop: "  << ((OpNode*)(*j)->elt)->alt_index << endl;
//					printBDD((*j)->label);
//				}
#else
				if(((*j)->elt)->alt_index < num_alt_acts){
					cout << ((*j)->elt)->name << alt_act_costs[((*j)->elt)->alt_index] << endl<<flush;

				}
				else
					cout << ((OpNode*)(*j)->elt)->name << " 0" << endl;
#endif


				//	printBDD((*j)->label);
			}
		}
		//  p = partitions[i]->begin();
		//       for(;p!=partitions[i]->end();p++){
		// 	printBDD((*p));
		//       }
		cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" <<endl;
	}
}




RelaxedPlan::~RelaxedPlan() {
	std::set<LabelledAction*>::iterator j;
	std::set<LabelledEffect*>::iterator r;
	std::list<LabelledEffect*>::iterator q;
	std::list<LabelledFormula*>::iterator k;
	std::list<DdNode*>::iterator p;
	    //cout << "del rp" << plan_length << endl;
	if(this != NULL) {
		for (int i = 0; i < plan_length; i++){
			if(action_levels && action_levels[i]){
				// cout << "a = " << i << endl;
				for(j = action_levels[i]->begin();
				j!=action_levels[i]->end();++j){
					//   cout << "HI"<<endl<<flush;
					delete *j;
				}
				// cout <<"HI"<<endl<<flush;
				action_levels[i]->clear();
				delete action_levels[i];
			}

			if(effect_levels && effect_levels[i]){
				// cout << "e = " << i << endl;
				for(r = effect_levels[i]->begin();
				r!=effect_levels[i]->end();++r){
					delete *r;
				}
				effect_levels[i]->clear();
				delete effect_levels[i];
			}

			if(b_goal_levels && b_goal_levels[i]) {
				// cout << "g = " << i << endl;
				k = b_goal_levels[i]->begin();
				for(;k!=b_goal_levels[i]->end();++k) {
					delete *k;
				}
				b_goal_levels[i]->clear();
				delete b_goal_levels[i];
			}
			/**
			 * momo007 2022.09.16
			 * why not release the memory of ef_mutexed_a and ef_mutexed_b
			 */
			//	if(ef_mutexes_a && ef_mutexes_a[i] != NULL){
			// 	  cout << "xa = " << i << endl;
			// 	  for(q = ef_mutexes_a[i]->begin();
			// 	      q!= ef_mutexes_a[i]->end();++q){
			// 	    delete *q;
			// 	  }
			// 	  ef_mutexes_a[i]->clear();
			// 	  delete ef_mutexes_a[i];
			//         }

			// 	if(ef_mutexes_b && ef_mutexes_b[i]){
			// 	cout << "xb = " << i << endl;
			// 	  for(q = ef_mutexes_b[i]->begin();
			// 	      q!= ef_mutexes_b[i]->end();++q){
			// 	    delete *q;
			// 	  }
			// 	  ef_mutexes_b[i]->clear();
			// 	  delete ef_mutexes_b[i];
			//         }

			if(partitions && partitions[i]){
				// cout << "p = " << i << endl;
				for(p = partitions[i]->begin();
				p!= partitions[i]->end();++p){
					Cudd_RecursiveDeref(manager, *p);
				}
				partitions[i]->clear();
				delete partitions[i];
			}
		}
	}
	// cout << "exit del rp"<<endl;
}

double RelaxedPlanLite::computeInteraction(RelaxedPlanLite* rp){
	//interaction is # common actions / # possible common

	double common = 0.0;
	double possibleCommon = 0.0;
	int level_common = 0;
	double interaction;

	if(rp == NULL)
	  return 0.0;


	int min_len = (rp->actions.size() < actions.size() ? rp->actions.size() : actions.size());


	bool actInteraction = //	  
	  true;//	 false;

	for(int i = 0; i < 1// min_len //actions.size()
	; i++){
	  //std::cout << "lev = " << i << std::endl;
	  level_common = 0;

	  if(actInteraction){
	    for(set<int>::iterator k = actions[i]->begin(); k != actions[i]->end(); k++){
	      if(rp->actions[i]->find(*k) != rp->actions[i]->end()){
		
		level_common++;
	      }
	    }
	    if(actions[i] && actions[i]->size() > 0){
	      common += level_common; //pow(level_common/rp->actions[i]->size(),  1/(i+1));
	      possibleCommon += (rp->actions[i]->size() > actions[i]->size() ? 
				 rp->actions[i]->size() :
				 actions[i]->size());
		//actions[i]->size(); //pow(1, 1/(i+1));
	    }
	  }
	  else { //proopsition interaction
	    for(set<int>::iterator k = propositions[i]->begin(); k != propositions[i]->end(); k++){
	      if(rp->propositions[i]->find(*k) != rp->propositions[i]->end()){
		
		level_common++;
	      }
	    }
	    if(propositions[i] && propositions[i]->size() > 0){
	      common += level_common; //pow(level_common/rp->actions[i]->size(),  1/(i+1));
	      possibleCommon += (rp->propositions[i]->size() > propositions[i]->size() ? 
				 rp->propositions[i]->size() :
				 propositions[i]->size()); //pow(1, 1/(i+1));
	    }
	    
	  }

	}
	//std::cout << "done" << std::endl;
	interaction =  (possibleCommon > 0 ?  common/possibleCommon : 0);
	//std::cout << "int = " << interaction << std::endl;

	if(0 && interaction == 1){
		cout << "RPS: " << endl;
		int maxLen = max(actions.size(), rp->actions.size());
		for(int i = 0; i < maxLen; i++){
			if(actions[i]->size() == 0 && rp->actions[i]->size() == 0)
				break;
			cout << i << ": ";
			if(i < actions.size()){
				for(set<int>::iterator k = actions[i]->begin(); k != actions[i]->end(); k++){
					cout << *k << " ";
				}
			}
			cout << "\t\t|\t";
			if(i < rp->actions.size()){
				for(set<int>::iterator k = rp->actions[i]->begin(); k != rp->actions[i]->end(); k++){
					cout << *k << " ";
				}
			}
			cout << endl << endl;
		}
	}

	//return common/possibleCommon;
	return interaction;
}


void RelaxedPlanLite::unionRP(RelaxedPlan* rp){
  for(int i = 0; i < rp->plan_length; i++){
    if(!rp->action_levels[i])
      continue;
	// add noop action
    for(set<LabelledAction*>::iterator j = rp->action_levels[i]->begin();
	j != rp->action_levels[i]->end(); j++){
      if(!(*j)->elt->is_noop)
	actions[i]->insert((*j)->elt->index);
    }
    if(!rp->b_goal_levels[i])
      continue;
	// add fact
    for(list<LabelledFormula*>::iterator j = rp->b_goal_levels[i]->begin();
	j != rp->b_goal_levels[i]->end(); j++){
      DdNode *p  = (DdNode*)(*j)->elt;
      for(int i = 0; i < num_alt_facts; i++){
	DdNode* support;
	
	support = Cudd_Support(manager, p);
	Cudd_Ref(support);
	
	if(bdd_entailed(manager, support, Cudd_bddIthVar(manager, 2*i))){
	  if(Cudd_bddIsVarEssential(manager, p, 2*i, 1)){
	    propositions[i]->insert(i);
	  }
	  else if(Cudd_bddIsVarEssential(manager, p, 2*i, 0)){
	    propositions[i]->insert(2*i);
	  }
	  
	}
	Cudd_RecursiveDeref(manager,support);
      }
    }
  }
}

double RelaxedPlanLite::h_value(){
	double h = 0.0;
	for(int i = 0; i < IPP_MAX_PLAN; i++){
		h += actions[i]->size();
	}
	return h;
}
