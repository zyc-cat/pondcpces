/* -*-C++-*- */
/*
 * Actions.
 *
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
 * $Id: actions.h,v 1.2 2006/09/29 23:13:55 dan Exp $
 */
#ifndef ACTIONS_H
#define ACTIONS_H

#include <config.h>
#include "terms.h"
#include <iostream>
#include <map>
#include <string>
#include "observations.h"

struct Observation;
struct PredicateTable;
struct FunctionTable;
struct ValueMap;
struct StateFormula;
struct AtomSet;
struct pEffect;
struct Problem;
struct OutcomeSet; //dan

/* ====================================================================== */
/* ActionSchema */

struct Action;
struct ActionList;

/*
 * Action schema.
 */
struct ActionSchema {
  /* Constructs an action schema with the given name. */
  ActionSchema(const std::string& name);

  /* Deletes this action schema. */
  ~ActionSchema();

  /* Adds a parameter to this action schema. */
  void add_parameter(Variable parameter) { parameters_.push_back(parameter); }

  /* Sets the precondition of this action schema. */
  void set_precondition(const StateFormula& precondition);

  /* Sets the effect of this action schema. */
  void set_effect(const pEffect& effect);
 
  /* Sets the observation of this action schema. */
  void set_observation(const Observation& observation);
  /*momo007 002 2022.02.28
    这块observation_cpt的作用
  */
  void set_observation_cpt(const ObservationCpt& observation);

  
  /* Returns the name of this action schema. */
  const std::string& name() const { return name_; }

  /* Returns the arity of this action schema. */
  size_t arity() const { return parameters_.size(); }

  /* Returns the ith parameter of this action schema. */
  Variable parameter(size_t i) const { return parameters_[i]; }

  /* Returns the preconditions of this action schema. */
  const StateFormula& precondition() const {  return *precondition_; }

  /* Returns the effect of this action schema. */
  const pEffect& effect() const { return *effect_; }

  /* Returns the effect of this action schema. */
  const Observation& observation() const { return *observation_; }
  const ObservationCpt& observation_cpt() const { return *observation_cpt_; }

  bool hasObservation() const {return hasObservation_;}
  

  /* Fills the provided list with instantiations of this action
     schema.
     momo007 001 2022.2.28
    实现action schema的所有满足的action，存储在ActionList中
    借助栈来是实现所有参数的组合   
  */
  void instantiations(ActionList& actions, const Problem& problem) const;

  /* Returns an instantiation of this action schema. 
    实现一种action参数组合的情况，参数组合由subst给出，返回action的引用
  */
  const Action& instantiation(const SubstitutionMap& subst,
			      const Problem& problem,
			      const StateFormula& precond) const;

  /* Prints this action schema on the given stream. */
  void print(std::ostream& os, const PredicateTable& predicates,
	     const FunctionTable& functions, const TermTable& terms) const;

private:
  /* Action name. */
  std::string name_;
  /* Action parameters. */
  VariableList parameters_;
  /* Action precondition. */
  /* Action effect. */
  const StateFormula* precondition_;
  const pEffect* effect_;
  /* Action effect. */
  const Observation* observation_;
  const ObservationCpt* observation_cpt_;
  bool hasObservation_;

};


/* ====================================================================== */
/* ActionSchemaMap */

/*
 * Table of action schemas.
 */
struct ActionSchemaMap : public std::map<std::string, const ActionSchema*> {
};


/* ====================================================================== */
/* Action */

/*
 * A fully instantiated action.
 */
struct Action {
  /* Constructs an action with the given name. */
  Action(const std::string& name);

  /* Deletes this action. */
  ~Action();

  /* Adds an argument to this action. */
  void add_argument(Object argument) { arguments_.push_back(argument); }

  /* Sets the precondition of this action. */
  void set_precondition(const StateFormula& precondition);

  /* Sets the effect of this action. */
  void set_effect(const pEffect& effect);

  /* Sets the observation of this action schema. */
  void set_observation(const Observation& observation);
  void set_observation_cpt(const ObservationCpt& observation);
 
  /* Returns the id for this action. */
  size_t id() const { return id_; }

  /* Returns the name of this action. */
  const std::string& name() const { return name_; }

  /* Returns the arity of this action. */
  size_t arity() const { return arguments_.size(); }

  /* Returns the ith argument of this action. */
  Object argument(size_t i) const { return arguments_[i]; }

  /* Returns the precondition of this action. */
  const StateFormula& precondition() const { return *precondition_; }

  /* Returns the effect of this action. */
  const pEffect& effect() const { return *effect_; }

  /* Returns the effect of this action. */
 const Observation& observation() { return *observation_; }
  const ObservationCpt& observation_cpt() const { return *observation_cpt_; }

  /* Tests if this action is enabled in the given state. */
  bool enabled(const AtomSet& atoms, const ValueMap& values) const;

  /* Changes the given state according to the effects of this action. 
    momo007 003 2022.02.28
    该动作对于atomset的影响
  */
  void affect(AtomSet& atoms, ValueMap& values) const;

  /* Prints this action on the given stream. */
  void print(std::ostream& os, const TermTable& terms) const;

  /* Prints this action on the given stream in XML. */
  void printXML(std::ostream& os, const TermTable& terms) const;

  //dan
  void setProbabilityFromExpressions(const Problem* problem) const;
  bool hasObservation() const {return hasObservation_;}  
 
private:
  /* Next action id. */
  static size_t next_id;

  /* Unique id for actions. */
  size_t id_;
  /* Action name. */
  std::string name_;
  /* Action arguments. */
  ObjectList arguments_;
  /* Action precondition. */
  const StateFormula* precondition_;
  /* Action effect. */
  const pEffect* effect_;
  const Observation* observation_;
  const ObservationCpt* observation_cpt_;//cpt的observation的作用？
  bool hasObservation_;
 
};

/*
 * Less than function object for action pointers.
 */
namespace std {
  template<> struct less<const Action*>
    : public binary_function<const Action*, const Action*, bool> {
    /* Comparison function operator. */
    bool operator()(const Action* a1, const Action* a2) const {
      if (a1 == NULL) {
	return a2 != NULL;
      } else if (a2 == NULL) {
	return false;
      } else {
	return a1->id() < a2->id();
      }
    }
  };
}


/* ====================================================================== */
/* ActionList */

/*
 * List of actions.
 */
struct ActionList : public std::vector<const Action*> {
};


#endif /* ACTIONS_H */
