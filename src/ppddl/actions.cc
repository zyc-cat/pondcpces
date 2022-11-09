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
 * $Id: actions.cc,v 1.1 2006/07/11 17:53:06 dan Exp $
 */

#include "actions.h"
#include "problems.h"
#include "domains.h"
#include "formulas.h"
#include <stack>
#include <list>

extern Problem *my_problem;

/* ====================================================================== */
/* ActionSchema */

/* Constructs an action schema with the given name. */
ActionSchema::ActionSchema(const std::string &name)
    : name_(name), precondition_(&StateFormula::TRUE),
      effect_(new ConjunctiveEffect()), observation_(NULL), observation_cpt_(NULL), hasObservation_(false)
{
  StateFormula::register_use(precondition_);
  pEffect::register_use(effect_);
}

/* Deletes this action schema. */
ActionSchema::~ActionSchema()
{
  StateFormula::unregister_use(precondition_);
  pEffect::unregister_use(effect_);
}

/* Sets the precondition of this action schema. */
void ActionSchema::set_precondition(const StateFormula &precondition)
{
  if (&precondition != precondition_)
  {
    StateFormula::unregister_use(precondition_);
    precondition_ = &precondition;
    StateFormula::register_use(precondition_);
  }
}

/* Sets the effect of this action schema. */
void ActionSchema::set_effect(const pEffect &effect)
{
  if (&effect != effect_)
  {
    const pEffect *tmp = effect_;
    effect_ = &effect;
    pEffect::register_use(effect_);
    pEffect::unregister_use(tmp);
  }
}

/* Sets the observation of this action schema. */
void ActionSchema::set_observation(const Observation &observation)
{
  if (&observation != observation_)
  {
    const Observation *tmp = observation_;
    observation_ = &observation;
    hasObservation_ = true;
    Observation::register_use(observation_);
    Observation::unregister_use(tmp);
  }
}

void ActionSchema::set_observation_cpt(const ObservationCpt &observation_cpt)
{
  if (&observation_cpt != observation_cpt_)
  {
    const ObservationCpt *tmp = observation_cpt_;
    observation_cpt_ = &observation_cpt;
    hasObservation_ = true;

    //     Observation::register_use(observation_);
    // Observation::unregister_use(tmp);
  }
}

/* Fills the provided list with instantiations of this action
   schema. 
*/
void ActionSchema::instantiations(ActionList &actions,
                                  const Problem &problem) const
{
  size_t n = arity();// 获取action schema的参数的个数

  if (n == 0)
  {
    SubstitutionMap subst;// 创建一个空的subst
    const StateFormula &precond = precondition().instantiation(subst, problem);//考虑对前提进行实例化
    if (!precond.contradiction())//前提条件正确不矛盾
    {
      actions.push_back(&instantiation(subst, problem, precond));//添加该实例化动作
    }
  }
  else// 含有参数的情况
  {
    SubstitutionMap args;// 替换的参数常量映射关系
    std::vector<ObjectList> arguments(n, ObjectList());//考虑n个参数列表，每个参数位置有多个常数可以实例化
    std::vector<ObjectList::const_iterator> next_arg;
    // 该循环完成所有参数适合的常量查询，存放在arguments的n个列表中
    for (size_t i = 0; i < n; i++)//迭代每个参数
    {
      // 考虑第i个参数，查找合适的实例化常量，存放在arguments[i]中
      problem.compatible_objects(arguments[i],
                                 problem.domain().terms().type(parameter(i)));
      // 有一个参数没有没有合适的常数，无法实例化，直接返回
      if (arguments[i].empty())
      {
        return;
      }
      // 将该合适的参数列表加入到next-arg中
      next_arg.push_back(arguments[i].begin());
    }
    // 前提条件的栈，为了能够实现各种参数的组合，因此需要借助栈
    std::stack<const StateFormula *> preconds;
    // 添加该动作的前提条件
    preconds.push(precondition_);
    // 使用顶部，进行注册
    StateFormula::register_use(preconds.top());
    // 考虑剩余的n个参数
    for (size_t i = 0; i < n;)
    {
      // 使用第i个参数中的参量
      args.insert(std::make_pair(parameter(i), *next_arg[i]));
      SubstitutionMap pargs;
      // pargs和args一样，为什么实现两份？
      pargs.insert(std::make_pair(parameter(i), *next_arg[i]));
      // 当前仅完成前[0,i]的参数实例化
      const StateFormula &precond =
          preconds.top()->instantiation(pargs, problem);
      // 加入到栈中
      preconds.push(&precond);
      StateFormula::register_use(preconds.top());
      // 如果当前是最后一个参数或者前提条件不幸
      if (i + 1 == n || precond.contradiction())
      {
        // 前提条件满足，那么该动作已经实例化完成
        if (!precond.contradiction())
        {
          // 添加该动作的实例化
          actions.push_back(&instantiation(args, problem, precond));
        }
        // 清空之前的args列表
        for (int j = i; j >= 0; j--)
        {
          StateFormula::unregister_use(preconds.top());
          preconds.pop();
          args.erase(parameter(j));
          next_arg[j]++;// 考虑该参数的下一个常数
          if (next_arg[j] == arguments[j].end())
          {
            if (j == 0)
            {
              i = n;
              break;
            }
            else
            {
              next_arg[j] = arguments[j].begin();
            }
          }
          else
          {
            i = j;
            break;
          }
        }
      }
      else
      {
        i++;
      }
    }
    // 完成后，清空栈，取消引用计数 
    while (!preconds.empty())
    {
      StateFormula::unregister_use(preconds.top());
      preconds.pop();
    }
  }
}

/* Returns an instantiation of this action schema. 
*/
const Action &ActionSchema::instantiation(const SubstitutionMap &subst,
                                          const Problem &problem,
                                          const StateFormula &precond) const
{
  Action *action = new Action(name());// 创建一个action
  size_t n = arity();
  for (size_t i = 0; i < n; i++)// 考虑每个参数
  {
    // 查看该参数需要实例化的常数
    SubstitutionMap::const_iterator si = subst.find(parameter(i));
    action->add_argument((*si).second);
  }
  // 设置前提条件和observation的数据
  action->set_precondition(precond);
  action->set_effect(effect().instantiation(subst, problem));
  if (observation_ != NULL)
    action->set_observation(observation().instantiation(subst, problem));
  if (observation_cpt_ != NULL)
    action->set_observation_cpt(observation_cpt().instantiation(subst, problem));
  //  cout << "done with " << name()<<endl;
  return *action;
}

/* Prints this action schema on the given stream. */
void ActionSchema::print(std::ostream &os, const PredicateTable &predicates,
                         const FunctionTable &functions,
                         const TermTable &terms) const
{
  os << "  " << name();
  os << std::endl
     << "    parameters:";
  for (VariableList::const_iterator vi = parameters_.begin();
       vi != parameters_.end(); vi++)
  {
    os << ' ';
    terms.print_term(os, *vi);
  }
  os << std::endl
     << "    precondition: ";
  precondition().print(os, predicates, functions, terms);
  os << std::endl
     << "    effect: ";
  effect().print(os, predicates, functions, terms);
}

/* ====================================================================== */
/* Action */

/* Next action id. */
size_t Action::next_id = 1;

/* Constructs an action with the given name. */
Action::Action(const std::string &name)
    : id_(next_id++), name_(name), precondition_(&StateFormula::TRUE),
      effect_(new ConjunctiveEffect()), observation_(NULL), hasObservation_(false)
{
  StateFormula::register_use(precondition_);
  pEffect::register_use(effect_);
}

/* Deletes this action. */
Action::~Action()
{
  StateFormula::unregister_use(precondition_);
  pEffect::unregister_use(effect_);
}

/* Sets the precondition of this action. */
void Action::set_precondition(const StateFormula &precondition)
{
  if (&precondition != precondition_)
  {
    StateFormula::unregister_use(precondition_);
    precondition_ = &precondition;
    StateFormula::register_use(precondition_);
  }
}

/* Sets the effect of this action. */
void Action::set_observation(const Observation &observation)
{

  if (&observation != observation_)
  {
    // cout << "HI"<<endl;
    if (observation_ != NULL)
      Observation::unregister_use(observation_);

    hasObservation_ = true;
    observation_ = &observation;

    Observation::register_use(observation_);
  }
  //  cout << "BYE"<<endl;
}
void Action::set_observation_cpt(const ObservationCpt &observation_cpt)
{

  hasObservation_ = true;
  observation_cpt_ = &observation_cpt;
}

/* Sets the effect of this action.
  momo007 004 2022.02.28
  这块使用很多次dynamic判断进行向下转换，看看能否优化
  这块作者貌似有严重的bug
*/
void Action::set_effect(const pEffect &effect)
{
  if (&effect != effect_)
  { //设置的effect是新的一个
    //判断是否为多个effect合取
    if (const ConjunctiveEffect *e = dynamic_cast<const ConjunctiveEffect *>(&effect))
    {
      ConjunctiveEffect *e1 = new ConjunctiveEffect(); //创建一个新的空的conjunct effect
      std::list<const StateFormula *> reward_conditions;//所需要的前提条件
      bool got_reward = false;
      // 迭代conjuncts中的每个effect
      for (int ei = 0; ei < e->size(); ei++)
      {
        e1->add_conjunct(e->conjunct(ei));
        //如果条件且赋值，则添加他的条件
        if (const ConditionalEffect *ce = dynamic_cast<const ConditionalEffect *>(&e->conjunct(ei)))
        {
          if (const AssignmentEffect *ae = dynamic_cast<const AssignmentEffect *>(&ce->effect()))
          {
            reward_conditions.push_back(&ce->condition());
          }
        }
        // 是普通的赋值但不是条件（没有条件），那么got_reward设置为true
        else if (const AssignmentEffect *ae = dynamic_cast<const AssignmentEffect *>(&e->conjunct(ei)))
        {
          got_reward = true;
        }
      }//end-for

      // 如果有多个条件
      if (reward_conditions.size() > 0)
      {
        // 创建Assignment一些必要的组件
        std::pair<Function, bool> reward_function = my_problem->domain().functions().find_function("reward");
        const Application &reward_appl =
            Application::make_application(reward_function.first, TermList());
        // 创建Assignment
        const Assignment *as = new Assignment(Assignment::ASSIGN_OP, reward_appl, *new Value(0.0));
        // 这块实现将条件的conjunct转化为条件的disjunct
        Disjunction *d = new Disjunction();
        for (std::list<const StateFormula *>::iterator i = reward_conditions.begin(); i != reward_conditions.end(); i++)
        {
          d->add_disjunct((const StateFormula &)**i);
        }
        // n此时存储该conjunctionEffect需要的前提条件
        const StateFormula &n = // Negation(*d);//
            Negation::make_negation((const StateFormula &)*d);
        //	n.print(std::cout, my_problem->domain().predicates(), my_problem->domain().functions(), my_problem->terms());
        const AssignmentEffect &ae = *new AssignmentEffect(*as);
        e1->add_conjunct(ConditionalEffect::make(n, ae));
      }
      // 该分支说明说有effect都是前提都是true
      else if (got_reward)
      {
        std::pair<Function, bool> reward_function = my_problem->domain().functions().find_function("reward");
        const Application &reward_appl =
            Application::make_application(reward_function.first, TermList());
        const Assignment *as = new Assignment(Assignment::ASSIGN_OP, reward_appl, *new Value(0.0));
        e1->add_conjunct(*new AssignmentEffect(*as));
      }
      // delete effect;
      effect_ = e1;
    }
    else
    {
      pEffect::unregister_use(effect_);
      effect_ = &effect;
      pEffect::register_use(effect_);
    }
  }
}

/* Tests if this action is enabled in the given state. */
bool Action::enabled(const AtomSet &atoms, const ValueMap &values) const
{
  return precondition_->holds(atoms, values);
}

/**  
 * Changes the given state according to the effects of this action. 
 */
void Action::affect(AtomSet &atoms, ValueMap &values) const
{
  // 动作的affect记录三种列表，包括addList，delList，assignmentsList
  AtomList adds;
  AtomList deletes;
  AssignmentList assignments;
  // Effct类提供了接口，不同的类会根据自己的类型修改add、del、assignment的List
  effect().state_change(adds, deletes, assignments, atoms, values);
  // 随后根据每种List修改atoms
  // 将deleteList中的atom删除
  for (AtomList::const_iterator ai = deletes.begin();
       ai != deletes.end(); ai++)
  {
    atoms.erase(*ai);
  }
  // 添加AddList中的atoms
  atoms.insert(adds.begin(), adds.end());
  // 更新所有assignment
  for (AssignmentList::const_iterator ai = assignments.begin();
       ai != assignments.end(); ai++)
  {
    (*ai)->affect(values);
  }
}

/* Prints this action on the given stream. */
void Action::print(std::ostream &os, const TermTable &terms) const
{
  (PRINT_ND ? os << name() : os << '(' << name()); // DAN
  for (ObjectList::const_iterator oi = arguments_.begin();
       oi != arguments_.end(); oi++)
  {
    (PRINT_ND ? os << '_' : os << ' '); // DAN
    terms.print_term(os, *oi);
  }
  (PRINT_ND ? os << "" : os << ')'); // DAN
}

/* Prints this action on the given stream in XML. */
void Action::printXML(std::ostream &os, const TermTable &terms) const
{
  os << "<action><name>" << name() << "</name>";
  for (ObjectList::const_iterator oi = arguments_.begin();
       oi != arguments_.end(); oi++)
  {
    os << "<term>";
    terms.print_term(os, *oi);
    os << "</term>";
  }
  os << "</action>";
}

// we allowed some expressions to be used in place of probabilities so
// that they can have functions in the intial condition for effect probability
// this function puts the real numbers in the effects before instantiatioin from the expressions
// so that nothing breaks

void recurse_effects(const Problem *problem, const pEffect *effect)
{

  const ConjunctiveEffect *ce =
      dynamic_cast<const ConjunctiveEffect *>(effect);
  // 转化成功，说明是合并的effect，递归每个conjunct
  if (ce != NULL)
  {
    //  cout << "Messing with effect CE"<<endl;
    size_t n = ce->size();
    if (n > 0)
    {
      for (size_t i = 0; i < n; i++)
      {
        recurse_effects(problem, &ce->conjunct(i));
      }
    }
    //   cout << "done Messing with effect CE"<<endl;
    return;
  }

  const QuantifiedEffect *qe =
      dynamic_cast<const QuantifiedEffect *>(effect);
  if (ce != NULL)
  {
    //  cout << "Messing with effect QE"<<endl;
    recurse_effects(problem, &qe->effect());
    return;
  }

  const ConditionalEffect *we =
      dynamic_cast<const ConditionalEffect *>(effect);
  if (we != NULL)
  {
    // cout << "Messing with effect CDE"<<endl;
    recurse_effects(problem, &we->effect());
    // cout << "done Messing with effect CDE"<<endl;
    return;
  }

  const ProbabilisticEffect *pe =
      dynamic_cast<const ProbabilisticEffect *>(effect);
  if (pe != NULL && pe->size() == 0 && pe->fsize() > 0)
  {
    // cout << "Messing with effect PR"<<endl;
    double sum = 0.0;
    // 调用probability effect的实现，设置effect的每个Expression的概率值和该effect总的概率
    ((ProbabilisticEffect *)pe)->setProbabilityFromExpressions(problem);
    size_t n = pe->size();// 表达式的个数
    for (size_t i = 0; i < n; i++)
    {
      recurse_effects(problem, &pe->effect(i));
    }
    //  cout << "done Messing with effect PR"<<endl;
    return;
  }
}

void Action::setProbabilityFromExpressions(const Problem *problem) const
{
  //  cout << "working on " << name_ <<endl;
  recurse_effects(problem, effect_);
  //  cout << "done working on " << name_ <<endl;
}
