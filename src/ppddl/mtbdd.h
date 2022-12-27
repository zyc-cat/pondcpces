/* -*-C++-*- */
#ifndef MTBDD_H
#define MTBDD_H

#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include "client.h"
#include <cudd.h>
#include <util.h>
#include <list>
#include "dbn.h"

double transform_reward_to_probability(double reward);
double transform_probability_to_reward(double pr);
DdNode* groundActionDD(const Action& action);
DdNode* getObservationDD(const Action &action);
dbn *action_dbn(const Action &action);
DdNode* groundEventDD(const Action& action);
void bdd_goal_cnf(std::list<DdNode*>* goal_cnf);

/* ====================================================================== */
/* MTBDDPlanner */

/*
 * An MTBDD planner.
 */
struct MTBDDPlanner : public Planner {
  /* Constructs an MTBDD planner. */
  MTBDDPlanner(const Problem& problem, double gamma, double epsilon)
    : Planner(problem), gamma_(gamma), epsilon_(epsilon), mapping_(NULL) {}

  /* Deletes this MTBDD planner. */
  virtual ~MTBDDPlanner();

  virtual void initRound();
  virtual const Action* decideAction(const State& state);
  virtual void endRound();

private:
  /* Discount factor. */
  double gamma_;
  /* Error tolerance. */
  double epsilon_;
  /* DD manager. */
  DdManager* dd_man_;
  /* Policy. */
  DdNode* mapping_;
  /* Actions. */
  std::map<size_t, const Action*> actions_;
  /* State variables. */
  std::map<int, const Atom*> dynamic_atoms_;
};


#endif /* MTBDD_H */
