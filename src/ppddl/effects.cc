/*
 * Copyright (C) 2003 Carnegie Mellon University and Rutgers University
 *
 * Permission is hereby granted to distribute this software for
 * non-commercial research purposes, provided that this copyright
 * notice is included with any such distribution.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
 * SOFTWARE IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU
 * ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 *
 * $Id: effects.cc,v 1.2 2006/09/29 23:13:55 dan Exp $
 */
#include "effects.h"
#include "problems.h"
#include "formulas.h"
#include "expressions.h"
#include <stack>
#include <stdexcept>
#include "exceptions.h"
#include "math.h"
#include <float.h>

int PRINT_ND=0;

/* ====================================================================== */
/* Assignment */

/* Constructs an assignment. */
Assignment::Assignment(AssignOp oper, const Application& application,
		const Expression& expr)
: operator_(oper), application_(&application), expr_(&expr) {
	Expression::register_use(application_);
	Expression::register_use(expr_);
}

/* Deletes this assignment. */
Assignment::~Assignment() {
	Expression::unregister_use(application_);
	Expression::unregister_use(expr_);
}


/* Changes the given state according to this assignment. */
void Assignment::affect(ValueMap& values) const {
	if (operator_ == ASSIGN_OP) {//赋值运算符号
		values[application_] = expr_->value(values);
	} else {
		//获取application的值
		ValueMap::const_iterator vi = values.find(application_);
		if (vi == values.end()) {
			throw std::logic_error("changing undefined value");
			//根据具体的操作，计算相应的结果
		} else if (operator_ == SCALE_UP_OP) {
			values[application_] = (*vi).second * expr_->value(values);
		} else if (operator_ == SCALE_DOWN_OP) {
			values[application_] = (*vi).second / expr_->value(values);
		} else if (operator_ == INCREASE_OP) {
			values[application_] = (*vi).second + expr_->value(values);
		} else { /* operator_ == DECREASE_OP */
			values[application_] = (*vi).second - expr_->value(values);
		}
	}
}


/* Returns an instantiaion of this assignment. */
const Assignment& Assignment::instantiation(const SubstitutionMap& subst,
		const Problem& problem) const {
	return *new Assignment(operator_,
			application_->substitution(subst),
			expr_->instantiation(subst, problem));
}

void AssignmentEffect::getMinMaxRewards(double *a_min_reward, double *a_max_reward, int* num_aux) const{
	const Application& application = assignment().application();

	ValueMap values;
	values[&application] = 0.0;
	assignment().affect(values);
	Rational v = (*values.begin()).second;
	double value = v.double_value();




	if(*a_min_reward == DBL_MAX){
	  // min not set
	  *a_min_reward = value;
	}
	else{
	  //min set
	  if(*a_min_reward <= 0.0){
	    if(value <= 0.0){
	      //sum negative valuw wiht negative min
	      *a_min_reward += value;
	    }
	    //else value positive and can only increase min
	  }
	  else {//min is positive
	    if(value < *a_min_reward){	    
	      //set to lower min
	      *a_min_reward = value;
	    }
	  }
	}


	if(*a_max_reward == -1*DBL_MAX){
	  // max not set
	  *a_max_reward = value;
	}
	else{
	  //max set
	  if(*a_max_reward >= 0.0){
	    if(value >= 0.0){
	      //sum pos valuw wiht pos max
	      *a_max_reward += value;
	    }
	    //else value neg and can only decrease max
	  }
	  else {//max is neg
	    if(value > *a_min_reward){	    
	      //set to higher max
	      *a_max_reward = value;
	    }
	  }
	}

	//	std::cout << value << " " << *a_min_reward << " " << *a_max_reward << std::endl;

	*num_aux += 2;

}


/* Prints this object on the given stream. */
void Assignment::print(std::ostream& os, const FunctionTable& functions,
		const TermTable& terms) const {

	if(!PRINT_ND){//DAN
		os << '(';
		if (operator_ == ASSIGN_OP) {
			os << "assign ";
		} else if (operator_ == SCALE_UP_OP) {
			os << "scale-up ";
		} else if (operator_ == SCALE_DOWN_OP) {
			os << "scale-down ";
		} else if (operator_ == INCREASE_OP) {
			os << "increase ";
		} else { /* operator_ == DECREASE_OP */
			os << "decrease ";
		}
		application_->print(os, functions, terms);
		os << ' ';
		expr_->print(os, functions, terms);
		os << ')';
	}
}

bool Assignment::operator == (const Assignment& a) const{
	//todo
	return false;
}
bool Assignment::operator<(const Assignment& a) const{
	//todo
	return false;
}

/* ====================================================================== */
/* SimpleEffect */

/* Constructs a simple effect. */
SimpleEffect::SimpleEffect(const Atom& atom)
: atom_(&atom) {
	StateFormula::register_use(atom_);
}


/* Deletes this simple effect. */
SimpleEffect::~SimpleEffect() {
	StateFormula::unregister_use(atom_);
}



/* ====================================================================== */
/* AddEffect */

/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
void AddEffect::state_change(AtomList& adds, AtomList& deletes,
		AssignmentList& assignments,
		const AtomSet& atoms,
		const ValueMap& values) const {
	adds.push_back(&atom());
}


/* Returns an instantiation of this effect. 
	momo007 005 2022.02.28
*/
const pEffect& AddEffect::instantiation(const SubstitutionMap& subst,
		const Problem& problem) const {
	// 根据给定的subst进行替换部分变量，然后返回的地址
	const Atom* inst_atom = &atom().substitution(subst);
	// 如果实例化前后一样，直接返回Effect
	if (inst_atom == &atom()) {
		return *this;
	} else {
		// 这是一个新的effect，根据实例化后的atom创建一个新的addEffect返回
		return *new AddEffect(*inst_atom);
	}
}


/* Prints this object on the given stream. */
void AddEffect::print(std::ostream& os, const PredicateTable& predicates,
		const FunctionTable& functions,
		const TermTable& terms) const {
	atom().print(os, predicates, functions, terms);
}

void AddEffect::getMinMaxRewards(double *a_min_reward, double *a_max_reward, int* num_aux) const{}


bool SimpleEffect::operator == (const pEffect& effect) const{
	const  SimpleEffect *ae = dynamic_cast<const SimpleEffect*>(&effect);
	if(ae){
		std::cout << "sEqual" << std::endl;

		const Atom *a1 = &atom();
		const Atom *a2 = &ae->atom();

		const  AddEffect *ae1 = dynamic_cast<const AddEffect*>(&effect);
		const  AddEffect *ae2 = dynamic_cast<const AddEffect*>(this);

		return ((ae1 == NULL && ae2 == NULL) || (ae1 != NULL && ae2 != NULL) ) && a1==a2;
	}
	return false;
}
bool SimpleEffect::operator<(const pEffect& effect) const{
	if(this->operator==(effect))
		return false;
	else
		return this < &effect;
}

bool AddEffect::operator == (const pEffect& effect) const{
	const  AddEffect *ae = dynamic_cast<const AddEffect*>(&effect);
	//std::cout << "aEqual" << std::endl;
	if(ae){

		const Atom *a1 = &atom();
		const Atom *a2 = &ae->atom();

		//std::cout << a1 << " " << a2 << " " <<  (a1==a2) << std::endl;

		if (a1==a2 )
			return true;
	}
	return false;
}
bool AddEffect::operator<(const pEffect& effect) const{
	// std::cout << "a<" << std::endl;

	if(this->operator==(effect))//  if(this == effect)
		return false;
	else{
		const  AddEffect *ae = dynamic_cast<const AddEffect*>(&effect);
		if(ae) return &atom() < &ae->atom();
		else
			return true;
	}
}

/* ====================================================================== */
/* DeleteEffect */

/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
void DeleteEffect::state_change(AtomList& adds, AtomList& deletes,
		AssignmentList& assignments,
		const AtomSet& atoms,
		const ValueMap& values) const {
	deletes.push_back(&atom());//在deleteList中添加删减的atom
}


/* Returns an instantiation of this effect.
	实现的结构和AddEffect完全形同，参考拿一块代码注释
*/
const pEffect& DeleteEffect::instantiation(const SubstitutionMap& subst,
		const Problem& problem) const {
	const Atom* inst_atom = &atom().substitution(subst);
	if (inst_atom == &atom()) {
		return *this;
	} else {
		return *new DeleteEffect(*inst_atom);
	}
}


/* Prints this object on the given stream. */
void DeleteEffect::print(std::ostream& os, const PredicateTable& predicates,
		const FunctionTable& functions,
		const TermTable& terms) const {
	os << "(not ";
	atom().print(os, predicates, functions, terms);
	os << ")";
}

void DeleteEffect::getMinMaxRewards(double *a_min_reward, double *a_max_reward, int* num_aux) const {}



bool DeleteEffect::operator == (const pEffect& effect) const{
	const  DeleteEffect *ae = dynamic_cast<const DeleteEffect*>(&effect);
	// std::cout << "dEqual" << std::endl;
	if(ae){

		const Atom *a1 = &atom();
		const Atom *a2 = &ae->atom();
		return a1==a2;
	}
	return false;
}

bool DeleteEffect::operator<(const pEffect& effect) const{
	//std::cout << "d<" << std::endl;
	if(this->operator==(effect))//  if(this == effect)
		return false;
	else{
		const  DeleteEffect *ae = dynamic_cast<const DeleteEffect*>(&effect);
		if(ae) return &atom() < &ae->atom();
		else
			return false;
	}
}
bool QuantifiedEffect::operator<(const pEffect& effect) const{
	if(this->operator==(effect))//  if(this == effect)
		return false;
	else
		return this < &effect;
}
bool ConditionalEffect::operator<(const pEffect& effect) const{
	if(this->operator==(effect))//  if(this == effect)
		return false;
	else
		return this < &effect;
}
bool ProbabilisticEffect::operator<(const pEffect& effect) const{
	if(this->operator==(effect))//  if(this == effect)
		return false;
	else
		return this < &effect;
}
bool ConjunctiveEffect::operator<(const pEffect& effect) const{
	if(this->operator==(effect))//  if(this == effect)
		return false;
	else
		return this < &effect;
}
bool AssignmentEffect::operator<(const pEffect& effect) const{
	if(this->operator==(effect))//  if(this == effect)
		return false;
	else
		return this < &effect;
}
/* ====================================================================== */
/* AssignmentEffect */

/* Constructs an assignment effect. */
AssignmentEffect::AssignmentEffect(const Assignment& assignment)
: assignment_(&assignment) {
}


/* Deletes this assignment effect. */
AssignmentEffect::~AssignmentEffect() {
	delete assignment_;
}


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
void AssignmentEffect::state_change(AtomList& adds, AtomList& deletes,
		AssignmentList& assignments,
		const AtomSet& atoms,
		const ValueMap& values) const {
	assignments.push_back(assignment_);//更新AssignmentList
}


/* Returns an instantiation of this effect. */
const pEffect& AssignmentEffect::instantiation(const SubstitutionMap& subst,
		const Problem& problem) const {
	return *new AssignmentEffect(assignment().instantiation(subst, problem));
}


/* Prints this object on the given stream. */
void AssignmentEffect::print(std::ostream& os,
		const PredicateTable& predicates,
		const FunctionTable& functions,
		const TermTable& terms) const {
	assignment().print(os, functions, terms);
}

bool AssignmentEffect::operator == (const pEffect& effect) const{
	const  AssignmentEffect *ae = dynamic_cast<const AssignmentEffect*>(&effect);
	if(ae){
		const Assignment *a1 = &assignment();
		const Assignment *a2 = &ae->assignment();
		return a1==a2;
	}
	return false;
}

/* ====================================================================== */
/* ConjunctiveEffect */

/* Deletes this conjunctive effect. */
ConjunctiveEffect::~ConjunctiveEffect() {
	for (EffectList::const_iterator ei = conjuncts_.begin();
			ei != conjuncts_.end(); ei++) {
		unregister_use(*ei);
	}
}


/* Adds a conjunct to this conjunctive effect. */
void ConjunctiveEffect::add_conjunct(const pEffect& conjunct) {
	const ConjunctiveEffect* conj_effect =
			dynamic_cast<const ConjunctiveEffect*>(&conjunct);
	if (conj_effect != NULL) {
		for (EffectList::const_iterator ei = conj_effect->conjuncts_.begin();
				ei != conj_effect->conjuncts_.end(); ei++) {
			conjuncts_.push_back(*ei);
			register_use(*ei);
		}
		register_use(&conjunct);
		unregister_use(&conjunct);
	} else {
		conjuncts_.push_back(&conjunct);
		register_use(&conjunct);
	}
}


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
void ConjunctiveEffect::state_change(AtomList& adds, AtomList& deletes,
		AssignmentList& assignments,
		const AtomSet& atoms,
		const ValueMap& values) const {
	// 迭代每一个conjunct中的Effect，分别调用Effect自身的state-change
	for (EffectList::const_iterator ei = conjuncts_.begin();
			ei != conjuncts_.end(); ei++) {
		(*ei)->state_change(adds, deletes, assignments, atoms, values);
	}
}


/* Returns an instantiation of this effect. */
const pEffect& ConjunctiveEffect::instantiation(const SubstitutionMap& subst,
		const Problem& problem) const {
	ConjunctiveEffect& inst_effect = *new ConjunctiveEffect();
	for (EffectList::const_iterator ei = conjuncts_.begin();
			ei != conjuncts_.end(); ei++) {
		inst_effect.add_conjunct((*ei)->instantiation(subst, problem));
	}
	return inst_effect;
}


/* Prints this object on the given stream. */
void ConjunctiveEffect::print(std::ostream& os,
		const PredicateTable& predicates,
		const FunctionTable& functions,
		const TermTable& terms) const {
	if (size() == 1) {
		conjunct(0).print(os, predicates, functions, terms);
	} else {
		os << "(and";
		for (EffectList::const_iterator ei = conjuncts_.begin();
				ei != conjuncts_.end(); ei++) {
			os << ' ';
			(*ei)->print(os, predicates, functions, terms);
		}
		os << ")";
	}
}


void ConjunctiveEffect::getMinMaxRewards(double *a_min_reward, double *a_max_reward, int* num_aux) const {
	//std::cout << "CE " << conjuncts_.size() << std::endl;
	for (EffectList::const_iterator ei = conjuncts_.begin();
			ei != conjuncts_.end(); ei++) {
		(*ei)->getMinMaxRewards(a_min_reward, a_max_reward, num_aux);
	}
}

bool ConjunctiveEffect::operator == (const pEffect& effect) const{
	const  ConjunctiveEffect *ae = dynamic_cast<const ConjunctiveEffect*>(&effect);
	if(ae){

		if(size() != ae->size())
			return false;

		for(int i = 0; i < size(); i++){
			if(!(conjunct(i).operator==(ae->conjunct(i))))
				return false;
		}
		return true;
	}
	return false;
}



/* ====================================================================== */
/* ConditionalEffect */

/* Returns a conditional effect. */
const pEffect& ConditionalEffect::make(const StateFormula& condition,
		const pEffect& effect) {
	if (condition.tautology()) {
		return effect;
	} else if (condition.contradiction()) {
		return *new ConjunctiveEffect();
	} else {
		return *new ConditionalEffect(condition, effect);
	}
}


/* Constructs a conditional effect. */
ConditionalEffect::ConditionalEffect(const StateFormula& condition,
		const pEffect& effect)
: condition_(&condition), effect_(&effect) {
	StateFormula::register_use(condition_);
	register_use(effect_);
}


/* Deletes this conditional effect. */
ConditionalEffect::~ConditionalEffect() {
	StateFormula::unregister_use(condition_);
	unregister_use(effect_);
}


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
void ConditionalEffect::state_change(AtomList& adds, AtomList& deletes,
		AssignmentList& assignments,
		const AtomSet& atoms,
		const ValueMap& values) const {
	// 判断当前condition是否成立
	if (condition().holds(atoms, values)) {
		/* Effect condition holds. */
		effect().state_change(adds, deletes, assignments, atoms, values);
	}
}


/* Returns an instantiation of this effect. */
const pEffect& ConditionalEffect::instantiation(const SubstitutionMap& subst,
		const Problem& problem) const {
	return make(condition().instantiation(subst, problem),
			effect().instantiation(subst, problem));
}


/* Prints this object on the given stream. */
void ConditionalEffect::print(std::ostream& os,
		const PredicateTable& predicates,
		const FunctionTable& functions,
		const TermTable& terms) const {
	os << "(when ";
	condition().print(os, predicates, functions, terms);
	os << ' ';
	effect().print(os, predicates, functions, terms);
	os << ")";
}

void ConditionalEffect::getMinMaxRewards(double *a_min_reward, double *a_max_reward, int* num_aux) const  {
	effect().getMinMaxRewards(a_min_reward, a_max_reward, num_aux);
}

bool ConditionalEffect::operator == (const pEffect& eff) const{
	const  ConditionalEffect *ae = dynamic_cast<const ConditionalEffect*>(&eff);
	if(ae){
		//todo check condition equality
		return effect()==ae->effect();
	}
	return false;
}

/* ====================================================================== */
/* ProbabilisticEffect */

/* Deletes this probabilistic effect. */
ProbabilisticEffect::~ProbabilisticEffect() {
	for (EffectList::const_iterator ei = effects_.begin();
			ei != effects_.end(); ei++) {
		unregister_use(*ei);
	}
}

/* Adds an outcome to this probabilistic effect. */
bool ProbabilisticEffect::add_foutcome(const Expression& p,
		const pEffect& effect) {
	//   const ProbabilisticEffect* prob_effect =
	//     dynamic_cast<const ProbabilisticEffect*>(&effect);
	//   if (prob_effect != NULL) {
	//     size_t n = prob_effect->size();
	//     for (size_t i = 0; i < n; i++) {
	//       if (!add_outcome(p*prob_effect->probability(i),
	// 		       prob_effect->effect(i))) {
	// 	return false;
	//       }
	//     }
	//     register_use(&effect);
	//     unregister_use(&effect);
	//   } else if (p != 0) {
	effects_.push_back(&effect);
	register_use(&effect);
	fweights_.push_back(&p);
	//    if (weight_sum_ == 0) {
	//       weights_.push_back(p.numerator());
	//       weight_sum_ = p.denominator();
	//       return true;
	//     } else {
	//       std::pair<int, int> m =
	// 	Rational::multipliers(weight_sum_, p.denominator());
	//       int sum = 0;
	//       size_t n = size();
	//       for (size_t i = 0; i < n; i++) {
	// 	sum += weights_[i] *= m.first;
	//       }
	//       weights_.push_back(p.numerator()*m.second);
	//       sum += p.numerator()*m.second;
	//       weight_sum_ *= m.first;
	//       return sum <= weight_sum_;
	//     }
	// }
	return true;
}

/* Adds an outcome to this probabilistic effect. */
bool ProbabilisticEffect::add_outcome(const Rational& p,
		const pEffect& effect) {
	// std::cout << "Add out " << p <<std::endl;
	const ProbabilisticEffect* prob_effect =
			dynamic_cast<const ProbabilisticEffect*>(&effect);
	if (prob_effect != NULL) {

		size_t n = prob_effect->size();
		for (size_t i = 0; i < n; i++) {
			if (!add_outcome(p*prob_effect->probability(i),
					prob_effect->effect(i))) {
				return false;
			}
		}
		register_use(&effect);
		unregister_use(&effect);
	} else {//if (p != 0.0) {
		effects_.push_back(&effect);
		register_use(&effect);
#ifdef FRACT_RATIONALS  
		if (weight_sum_ == 0) {
			weights_.push_back(p.numerator());
			weight_sum_ = p.denominator();
			return true;
		} else {
			std::pair< unsigned long int,  unsigned long int> m =
					Rational::multipliers(weight_sum_, p.denominator());
			int sum = 0;
			size_t n = size();
			for (size_t i = 0; i < n; i++) {
				sum += weights_[i] *= m.first;
			}
			weights_.push_back(p.numerator()*m.second);
			sum += p.numerator()*m.second;
			weight_sum_ *= m.first;
			return sum <= weight_sum_;
		}
#else
	weights_.push_back(p.double_value());
	weight_sum_+= p.double_value();
#endif
	}
	return true;
}

void ProbabilisticEffect::setProbabilityFromExpressions(const Problem* problem) {
	size_t n = fweights_.size();// 获取表达式的个数
	std::cout << "|fweights_|= " << n << std::endl;
	for (size_t i = 0; i < n; i++) {
		const Expression* e = fweights_[i];
		Rational p = e->value(problem->init_values());// 获取值
		std::cout << "p = " << p.double_value() << std::endl;
#ifdef FRACT_RATIONALS  
		if (weight_sum_ == 0) {
			weights_.push_back(p.numerator());
			weight_sum_ = p.denominator();
		}
		else {
			std::pair<int, int> m =
					Rational::multipliers(weight_sum_, p.denominator());
			int sum = 0;
			size_t n = size();
			for (size_t i = 0; i < n; i++) {
				sum += weights_[i] *= m.first;
			}
			weights_.push_back(p.numerator()*m.second);
			sum += p.numerator()*m.second;
			weight_sum_ *= m.first;
			//	cout << "ws = " << weight_sum_ <<endl;
		}
#else
		weights_.push_back(p.double_value());//加入到值列表中
		weight_sum_+= p.double_value();// 类加总的值
#endif
	}
}

/* Returns the ith outcome's probability. */
Rational ProbabilisticEffect::probability(size_t i) const {
#ifdef FRACT_RATIONALS  
	return Rational(weights_[i], weight_sum_);
#else
	return Rational(weights_[i]);
#endif
}

/* Returns the ith outcome's probability. */ //dan
const Expression* ProbabilisticEffect::fprobability(size_t i) const {
	return fweights_[i];
}

/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
void ProbabilisticEffect::state_change(AtomList& adds, AtomList& deletes,
		AssignmentList& assignments,
		const AtomSet& atoms,
		const ValueMap& values) const {
	if (size() != 0) {
		int w = int(rand()/(RAND_MAX + 1.0)*weight_sum_);
		int wtot = 0;
		size_t n = size();
		for (size_t i = 0; i < n; i++) {
			wtot += weights_[i];
			if (w < wtot) {
				effect(i).state_change(adds, deletes, assignments, atoms, values);
				return;
			}
		}
	}
}


/* Returns an instantiation of this effect. */
const pEffect&
ProbabilisticEffect::instantiation(const SubstitutionMap& subst,
		const Problem& problem) const {
	ProbabilisticEffect& inst_effect = *new ProbabilisticEffect();
	size_t n = size();
	size_t fn = fsize();

	if(n > 0){
		//   cout << "instan" << endl;
		if(n == 1 && probability(0) == -2.0) {//unknown effect
			inst_effect.add_outcome(*(new Rational(0.5)),
					effect(0).instantiation(subst, problem));
			inst_effect.add_outcome(*(new Rational(0.5)),
					(*(new DeleteEffect(((SimpleEffect&)effect(0)).atom()))).instantiation(subst, problem));

		}
		else if(n > 1 && fabs(probability(0).double_value()) == 1.0){ //oneof effect
			for (size_t i = 0; i < n; i++) {
				inst_effect.add_outcome(*(new Rational(1.0/(double)n)),
						effect(i).instantiation(subst, problem));
			}
		}
		else if(n > 1 && fabs(probability(0).double_value()) == 3.0){ //or effect
			for (size_t i = 0; i < n; i++) {
				inst_effect.add_outcome(*(new Rational(1.0/(double)n)),
						effect(i).instantiation(subst, problem));
			}
		}
		else{
			for (size_t i = 0; i < n; i++) {
				if(isObservation() || probability(i) > 0.0 )
				inst_effect.add_outcome(probability(i),
						effect(i).instantiation(subst, problem));
			}
		}
	}
	else if(fn > 0){
		for (size_t fi = 0; fi < fn; fi++) {
			//only instantiate effect if instantiation of expression is
			//mentioned in the initial conditions
			//  cout << "instatiating pr ef " << endl;
			const Expression* ex;
			try {
				ex = &fprobability(fi)->instantiation(subst, problem);
				//cout << "got expr"<<endl;


				inst_effect.add_foutcome(*ex,
						effect(fi).instantiation(subst, problem));
			} catch (const Exception& exc) {// cout << "Caught "  << exc <<endl;
			}


		}
	}

	return inst_effect;
}



/* Prints this object on the given stream. */
void ProbabilisticEffect::print(std::ostream& os,
		const PredicateTable& predicates,
		const FunctionTable& functions,
		const TermTable& terms) const {
	if(!PRINT_ND){
		os << "(probabilsitic";
		//     if (weight_sum_ == 0) {
		//       os << "(and)";
		//     } else if (weight_sum_ == weights_.back()) {
		//       effect(0).print(os, predicates, functions, terms);
		//     } else {
		size_t n = size();
		for (size_t i = 0; i < n; i++) {
			os << ' ' << probability(i) << ' ';
			effect(i).print(os, predicates, functions, terms);
			//     }
			os << ")";
		}
	}else{
		if (weight_sum_ == 0) {
			//DAN //os << "(and)";
		} else if (weight_sum_ == weights_.back()) {
			effect(0).print(os, predicates, functions, terms);
		} else {
			if(0){

				int max = 0;
				int maxw = 0;
				size_t n = size();
				for (size_t i = 0; i < n; i++) {
					//	  cout << "i = " << i << endl;
					if(weights_[i] > maxw){
						max = i;
						maxw = weights_[i];

					}
				}
				//	effect(max).print(cout, predicates, functions, terms);cout<<endl;
				effect(max).print(os, predicates, functions, terms);
			}
			else{
				os << "(oneof ";
				size_t n = size();
				for (size_t i = 0; i < n; i++) {
					//      cout<<"HI"<<endl;
					//os << ' ' << probability(i) << ' ';
					effect(i).print(os, predicates, functions, terms);
				}
				os << ")";
			}
		}
	}
}

void ProbabilisticEffect::getMinMaxRewards(double *a_min_reward, double *a_max_reward, int* num_aux) const{
	Rational p_rest = 1;
			size_t n = size();
	for (size_t i = 0; i < n; i++) {
		effect(i).getMinMaxRewards(a_min_reward, a_max_reward, num_aux);
		Rational p = probability(i);
		p_rest = p_rest-p;
	}
	if(p_rest > 0.0)
		n++;
	int inc = (int)ceil(log2((double)n));
	//std::cout << n << " " << inc << std::endl;
	*num_aux += inc;
	// std::cout << *num_aux << std::endl;
}

bool ProbabilisticEffect::operator == (const pEffect& eff) const{
	const  ProbabilisticEffect *ae = dynamic_cast<const ProbabilisticEffect*>(&eff);
	if(ae){

		if(size() != ae->size())
			return false;

		for(int i = 0; i < size(); i++){
			if(!(effect(i) == ae->effect(i)) || probability(i) != probability(i))
				return false;
		}
		return true;
	}
	return false;
}

/* ====================================================================== */
/* QuantifiedEffect */

/* Constructs a universally quantified effect. */
QuantifiedEffect::QuantifiedEffect(const pEffect& effect)
: effect_(&effect) {
	register_use(effect_);
}


/* Deletes this universally quantifed effect. */
QuantifiedEffect::~QuantifiedEffect() {
	unregister_use(effect_);
}


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
void QuantifiedEffect::state_change(AtomList& adds, AtomList& deletes,
		AssignmentList& assignments,
		const AtomSet& atoms,
		const ValueMap& values) const {
	effect().state_change(adds, deletes, assignments, atoms, values);
}


/* Returns an instantiation of this effect. */
const pEffect& QuantifiedEffect::instantiation(const SubstitutionMap& subst,
		const Problem& problem) const {
	int n = arity();
	if (n == 0) {
		return effect().instantiation(subst, problem);
	} else {
		SubstitutionMap args(subst);
		std::vector<ObjectList> arguments(n, ObjectList());
		std::vector<ObjectList::const_iterator> next_arg;
		for (int i = 0; i < n; i++) {
			problem.compatible_objects(arguments[i],
					problem.terms().type(parameter(i)));
			if (arguments[i].empty()) {
				return *new ConjunctiveEffect();
			}
			next_arg.push_back(arguments[i].begin());
		}
		ConjunctiveEffect* conj = new ConjunctiveEffect();
		std::stack<const pEffect*> conjuncts;
		conjuncts.push(&effect().instantiation(args, problem));
		register_use(conjuncts.top());
		for (int i = 0; i < n; ) {
			SubstitutionMap pargs;
			pargs.insert(std::make_pair(parameter(i), *next_arg[i]));
			const pEffect& conjunct = conjuncts.top()->instantiation(pargs, problem);
			conjuncts.push(&conjunct);
			if (i + 1 == n) {
				conj->add_conjunct(conjunct);
				for (int j = i; j >= 0; j--) {
					if (j < i) {
						unregister_use(conjuncts.top());
					}
					conjuncts.pop();
					next_arg[j]++;
					if (next_arg[j] == arguments[j].end()) {
						if (j == 0) {
							i = n;
							break;
						} else {
							next_arg[j] = arguments[j].begin();
						}
					} else {
						i = j;
						break;
					}
				}
			} else {
				register_use(conjuncts.top());
				i++;
			}
		}
		while (!conjuncts.empty()) {
			unregister_use(conjuncts.top());
			conjuncts.pop();
		}
		return *conj;
	}
}


/* Prints this object on the given stream. */
void QuantifiedEffect::print(std::ostream& os,
		const PredicateTable& predicates,
		const FunctionTable& functions,
		const TermTable& terms) const {
	if (parameters_.empty()) {
		effect().print(os, predicates, functions, terms);
	} else {
		os << "(forall (";
		VariableList::const_iterator vi = parameters_.begin();
		terms.print_term(os, *vi);
		for (vi++; vi != parameters_.end(); vi++) {
			os << ' ';
			terms.print_term(os, *vi);
		}
		os << ") ";
		effect().print(os, predicates, functions, terms);
		os << ")";
	}
}

void QuantifiedEffect::getMinMaxRewards(double *a_min_reward, double *a_max_reward, int* num_aux) const  {
	effect().getMinMaxRewards(a_min_reward, a_max_reward, num_aux);
}


bool QuantifiedEffect::operator == (const pEffect& eff) const{
	const  QuantifiedEffect *ae = dynamic_cast<const QuantifiedEffect*>(&eff);
	if(ae){
		//todo check quantifier equality
		return effect()==ae->effect();
	}
	return false;
}
