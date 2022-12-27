/* -*-C++-*- */
/*
 * Parser.
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
 * $Id: parser.yy,v 1.6 2006/11/09 07:43:43 dan Exp $
 */
%{
#include <config.h>
#include "problems.h"
#include "domains.h"
#include "actions.h"
#include "effects.h"
#include "formulas.h"
#include "expressions.h"
#include "functions.h"
#include "predicates.h"
#include "terms.h"
#include "types.h"
#include "rational.h"
#include "exceptions.h"
#include "observations.h"
#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

  //dan
#include "globals.h"
  //#include <FlexLexer.h>


/* Workaround for bug in Bison 1.35 that disables stack growth. */
#define YYLTYPE_IS_TRIVIAL 1


/*
 * Context of free variables.
 */
struct Context {
    void push_frame() {
        frames_.push_back(VariableMap());
    }

    void pop_frame() {
        frames_.pop_back();
    }

    void insert(const std::string& name, Variable v) {
        frames_.back()[name] = v;
    }

    std::pair<Variable, bool> shallow_find(const std::string& name) const {
        VariableMap::const_iterator vi = frames_.back().find(name);
        if (vi != frames_.back().end()) {
            return std::make_pair((*vi).second, true);
        } else {
            return std::make_pair(0, false);
        }
    }

    std::pair<Variable, bool> find(const std::string& name) const {
        for (std::vector<VariableMap>::const_reverse_iterator fi = frames_.rbegin(); fi != frames_.rend(); fi++) {
            VariableMap::const_iterator vi = (*fi).find(name);
            if (vi != (*fi).end()) {
                return std::make_pair((*vi).second, true);
            }
        }
        return std::make_pair(0, false);
    }

private:
    struct VariableMap : public std::map<std::string, Variable> {};
    
    std::vector<VariableMap> frames_;// 存储每一个变量名对应的index即variable
};

//DAN
 extern int yyrestart(FILE*);
 // extern int yyrestart();
 // extern int yy_scan_string(char*);


/* The lexer. */
extern int yylex();
/* Current line number. */
extern size_t line_number;
/* Name of current file. */
extern std::string current_file;
/* Level of warnings. */
extern int warning_level;

/* Whether the last parsing attempt succeeded. */
static bool success = true;
/* Current domain. */
static Domain* domain;
/* Domains. */
static std::map<std::string, Domain*> domains;
/* Problem being parsed, or NULL if no problem is being parsed. */
static Problem* problem;
/* Current requirements. */
static Requirements* requirements;
/* The goal probability function, if probabilistic effects are required. */
static Function goal_prob_function; 
/* The reward function, if rewards are required. */
static Function reward_function;
/* Predicate being parsed. */
static Predicate predicate;
/* Whether a predicate is being parsed. */
static bool parsing_predicate;
/* Whether predicate declaration is repeated. */
static bool repeated_predicate;
/* Function being parsed. */
static Function function;
/* Whether a function is being parsed. */
static bool parsing_function;
/* Whether function declaration is repeated. */
static bool repeated_function;
/* Action being parsed, or NULL if no action is being parsed. */
static ActionSchema* action;
/* Current variable context. */
static Context context;
/* Predicate for atomic state formula being parsed. */
static Predicate atom_predicate;
/* Whether the predicate of the currently parsed atom was undeclared. */
static bool undeclared_atom_predicate;
/* Whether parsing effect fluents. */
static bool effect_fluent;
/* Whether parsing metric fluent. */
static bool metric_fluent;
/* Function for function application being parsed. */
static Function appl_function;
/* Whether the function of the currently parsed application was undeclared. */
static bool undeclared_appl_function;
/* Paramerers for atomic state formula or function application being parsed. 
  状态变量表，一个domain/problem中使用的状态变量
*/
static TermList term_parameters;
/* Whether parsing an atom. */
static bool parsing_atom;
/* Whether parsing a function application. */
static bool parsing_application;
/* Whether parsing a obs token. */
static bool parsing_obs_token = false;
/* Quantified variables for effect or formula being parsed. */
static VariableList quantified;
/* Most recently parsed term for equality formula. */
static Term eq_term;
/* Most recently parsed expression for equality formula. */
static const Expression* eq_expr;
/* The first term for equality formula. */
static Term first_eq_term;
/* The first expression for equality formula. */
static const Expression* first_eq_expr;
/* Kind of name map being parsed. */
static enum { TYPE_KIND, CONSTANT_KIND, OBJECT_KIND, VOID_KIND } name_kind;

/* Outputs an error message. */
static void yyerror(const std::string& s); 
/* Outputs a warning message. */
static void yywarning(const std::string& s);
/* Creates an empty domain with the given name. */
static void make_domain(const std::string* name);
/* Creates an empty problem with the given name. */
static void make_problem(const std::string* name,
			 const std::string* domain_name);
/* Adds :typing to the requirements. */
static void require_typing();
/* Adds :fluents to the requirements. */
static void require_fluents();
/* Adds :disjunctive-preconditions to the requirements. */
static void require_disjunction();
/* Adds :conditional-effects to the requirements. */ 
static void require_conditional_effects();
/* Returns a simple type with the given name. */
static Type make_type(const std::string* name);
/* Returns the union of the given types. */
static Type make_type(const TypeSet& types);
/* Returns a simple term with the given name. */
static Term make_term(const std::string* name);
/* Creates a predicate with the given name. */
static void make_predicate(const std::string* name);
/* Creates an observation token with the given name. */
static void make_observable(const std::string* name);
/* Creates a function with the given name. */
static void make_function(const std::string* name);
/* Creates an action with the given name. */
 static ObservationEntry* make_observation(const StateFormula&, const StateFormula&, const Rational&);
 static ObservationEntry* make_observation(const StateFormula&, const Rational&, const Rational &);
 static ObservationEntry* make_observation(const StateFormula&, const ProbabilisticEffect&);
/* Creates an action with the given name. */
static void make_action(const std::string* name);
/* Adds the current action to the current domain. */
static void add_action();
/* Adds the current event to the current domain. */
static void add_event();
/* Prepares for the parsing of a universally quantified effect. */ 
static void prepare_forall_effect();
/* Creates a universally quantified effect. */
static const pEffect* make_forall_effect(const pEffect& effect);
/* Adds an outcome to the given probabilistic effect. */
static void add_effect_outcome(ProbabilisticEffect& peffect,
			        const Rational* p, 
			       const pEffect& effect);
//dan
static void add_feffect_outcome(ProbabilisticEffect& peffect,
			        const Expression* p, 
			       const pEffect& effect);
/* Creates an add effect. */
static const pEffect* make_add_effect(const Atom& atom);
/* Creates a delete effect. */
static const pEffect* make_delete_effect(const Atom& atom);
/* Creates an assignment effect. */
static const pEffect* make_assignment_effect(Assignment::AssignOp oper,
					    const Application& application,
					    const Expression& expr);
/* Adds types, constants, or objects to the current domain or problem. */
static void add_names(const std::vector<const std::string*>* names, Type type);
/* Adds variables to the current variable list. */
static void add_variables(const std::vector<const std::string*>* names,
			  Type type);
/* Prepares for the parsing of an atomic state formula. */ 
static void prepare_atom(const std::string* name);
/* Prepares for the parsing of a function application. */ 
static void prepare_application(const std::string* name);
/* Adds a term with the given name to the current atomic state formula. */
static void add_term(const std::string* name);
/* Creates the atomic formula just parsed. */
static const Atom* make_atom();
/* Creates the function application just parsed. */
static const Application* make_application();
/* Creates a subtraction. */
static const Expression* make_subtraction(const Expression& term,
					  const Expression* opt_term);
/* Creates an atom or fluent for the given name to be used in an
   equality formula. */
static void make_eq_name(const std::string* name);
/* Creates an equality formula. */
static const StateFormula* make_equality();
/* Creates a negated formula. */
static const StateFormula* make_negation(const StateFormula& negand);
/* Creates an implication. */
static const StateFormula* make_implication(const StateFormula& f1,
					    const StateFormula& f2);
/* Prepares for the parsing of an existentially quantified formula. */
static void prepare_exists();
/* Prepares for the parsing of a universally quantified formula. */
static void prepare_forall();
/* Creates an existentially quantified formula. */
static const StateFormula* make_exists(const StateFormula& body);
/* Creates a universally quantified formula. */
static const StateFormula* make_forall(const StateFormula& body);
/* Sets the goal reward for the current problem. */
static void set_goal_reward(const Expression& goal_reward);
/* Sets the default metric for the current problem. */
static void set_default_metric();
/* if we use logical formula for init, then need to get atoms holding for later grounding 
   purposes*/
 static void get_init_elts();
 static void set_discount(const Rational& discount);
/*DAN */


%}
// 声明 terminal symbol
%token DEFINE DOMAIN_TOKEN PROBLEM
%token REQUIREMENTS TYPES CONSTANTS PREDICATES FUNCTIONS OBSERVABLES
%token STRIPS TYPING NEGATIVE_PRECONDITIONS DISJUNCTIVE_PRECONDITIONS EQUALITY
%token EXISTENTIAL_PRECONDITIONS UNIVERSAL_PRECONDITIONS
%token QUANTIFIED_PRECONDITIONS CONDITIONAL_EFFECTS FLUENTS ADL
%token DURATIVE_ACTIONS DURATION_INEQUALITIES CONTINUOUS_EFFECTS
%token PROBABILISTIC_EFFECTS REWARDS MDP
%token ONEOF UNKNOWN NON_DETERMINISTIC_DYNAMICS PROBABILISTIC_DYNAMICS
%token ACTION EVENT PARAMETERS PRECONDITION EFFECT OBSERVATION
%token PDOMAIN OBJECTS INIT GOAL GOAL_REWARD METRIC GOAL_PROBABILITY
%token WHEN NOT AND OR IMPLY EXISTS FORALL PROBABILISTIC
%token ASSIGN SCALE_UP SCALE_DOWN INCREASE DECREASE MINIMIZE MAXIMIZE
%token NUMBER_TOKEN OBJECT_TOKEN EITHER
%token LE GE NAME VARIABLE NUMBER HORIZON DISC
%token ILLEGAL_TOKEN PLANTIME PLAN FORPROBLEM IF THEN ELSE CASE GOTO DONE
%token ANTI_COMMENT
%token OBSERVE
// data types that semantic value may have
%union {
    Assignment::AssignOp setop;
    const pEffect* effect;
    ConjunctiveEffect* ceffect;
    ProbabilisticEffect* peffect; 
    Observation* observation_defs;
    ObservationEntry* observation;
    const StateFormula* formula;
    const Atom* atom;
    Conjunction* conj;
    Disjunction* disj;
    OneOfDisjunction* odisj;
    const Expression* expr;
    const Application* appl;
    Comparison::CmpPredicate comp;
    Type type;
    TypeSet* types;
    const std::string* str;
    std::vector<const std::string*>* strs;
    const Rational* num;
    //DAN
    plan* t_plan;
    Instruction * t_instr;
    Guards *t_guards;
    label_symbol* t_label_symbol;
    //NAD
}
// types of semantic value for nonterminal symbol
%type <setop> assign_op
%type <effect> eff_formula p_effect simple_init one_init
%type <ceffect> eff_formulas one_inits
%type <peffect> prob_effs prob_inits oneof_effs or_effs oneof_inits unknown_effs unknown_inits
%type <formula> formula
%type <atom> atomic_name_formula atomic_term_formula
%type <conj> conjuncts
%type <disj> disjuncts
%type <odisj> oneof_disjuncts
%type <expr> value f_exp opt_f_exp ground_f_exp opt_ground_f_exp
%type <appl> ground_f_head f_head
%type <comp> binary_comp
%type <strs> name_seq variable_seq c_symbols
%type <type> type_spec type
%type <types> types
%type <str> type_name predicate function name variable 
%type <str> DEFINE DOMAIN_TOKEN PROBLEM GOAL_PROBABILITY
%type <str> WHEN NOT AND OR IMPLY EXISTS FORALL PROBABILISTIC
%type <str> ASSIGN SCALE_UP SCALE_DOWN INCREASE DECREASE MINIMIZE MAXIMIZE
%type <str> NUMBER_TOKEN OBJECT_TOKEN EITHER PLANTIME
%type <str> NAME VARIABLE 
%type <num> probability NUMBER 
%type <t_plan> c_plan c_plan_body
%type <t_instr> c_steps c_step 
%type <t_instr> c_instruction c_action c_if c_case c_goto
%type <t_instr> c_then c_else
%type <t_label_symbol> c_label
%type <t_guards> c_guards
%type <observation> observation
%type <observation_defs> observation_defs
%%
/* 文件类型 */
file : { success = true; line_number = 1; } domains_and_problems {}
//{ if (!success) YYERROR; }
|  { success = true; line_number = 1; } c_plan {}// {if (!success) YYERROR;  }
| { success = true; line_number = 1; } observation {} //{if (!success) YYERROR; }  
;

//====DAN============OBSERVATION FROM ENVIRONMENT=========//

e_observation : formula { the_observation = $1; }; 

//=====DAN===========PLANS=========//


c_plan : '('  DEFINE 
              '(' PLAN name ')'
              '(' FORPROBLEM name ')'
              c_plan_body
          ')'
            {the_plan=$11; $$->name = (char*)$5->c_str(); $$->problem_name = (char*)$9->c_str();}
;

c_plan_body : '(' c_steps ')'
{
    //cout << "Plan" << endl;
    $$ = new plan(); 
    $$->begin = $2;
}
;

// should be altered to be left-recursive
// but that requires keeping a tail pointer around
c_steps : c_step c_steps
        {
	  //cout << "Steps" << endl;
	  $$= $1;
	  $$->next = $2;
	}
|   /* empty */ { $$= 0; }
;

c_step : c_label c_instruction
{
  //cout << "Label Step" << endl;
  // mark label as associated with this instruction
  $1->instr = $2;
  $$ = $2; 
}
      | c_instruction 
{ 
  //cout << "Step" << endl;
  $$ = $1; 
}
;

c_instruction : '(' c_action ')' { $$ = $2; }
	| '(' c_if ')' { $$ = $2; }
	| '(' c_case ')' { $$ = $2; }
	| '(' c_goto ')' { $$ = $2; }
        | '(' DONE ')'  
{ 
  //cout << "Done" << endl;
  $$ = new Done(); 
}
| '(' DONE NUMBER NUMBER ')' 
{
  //cout << "LCPP"<<endl;
  $$ = new Lcp_Done($3->double_value(), $4->double_value());
}

| error {std::cout <<"trouble parsing instruction"<<std::endl;}
;

c_action : name c_symbols
  {
    //cout << "Action" << endl;
    std::string tmp;
    tmp += "(";
    tmp += (std::string)*$1;
    std::vector<std::string*>::iterator c;
    for(c=((std::vector<std::string*>*)($2))->begin(); c!=((std::vector<std::string*>*)($2))->end(); c++)
      {
	tmp += " ";
	tmp += (**c);
      }
    tmp += ")";

    $$ = new mAction(findAct(tmp));
  }
;

c_symbols :
   c_symbols name {$$=$1; $1->push_back($2);}
| /* Empty */ {$$=new   std::vector<const std::string*>;}
;

c_then : '(' THEN c_steps ')'
{
  //cout << "Then" << endl;
  $$ = $3;
}
;

c_else : '(' ELSE c_steps ')'
{
  //cout << "Else" << endl;
  $$ = $3;
}
;

c_if : IF formula c_then 
{
  //cout << "IfThen" << endl;
  $$ = new IfThen(formula_bdd(*$2, false),$3);
}
| IF formula c_then c_else
{
  //cout << "IfThenElse" << endl;
  $$ = new IfThenElse(formula_bdd(*$2, false),$3,$4);
}
;

c_case : CASE c_guards 
{
  //cout << "Case" << endl;
  $$ = new Case($2);
}
;

c_guards : c_guards '(' formula c_steps ')'
{
  //cout << "Guards" << endl;
  $$ = $1;
  $$->push_back(std::make_pair(formula_bdd(*$3, false),$4));
}
| /* empty */ { $$ = new Guards(); }
;


c_goto : GOTO c_label
{
  //cout << "Goto" << endl;
  $$ = new Goto($2);
}
;

c_label : name
{
  // make a label object
  // put/find in a hash table
  $$ = new label_symbol(*$1);//current_analysis->label_tab.symbol_ref($1);
}
;

domains_and_problems : /* empty */
                     |  domains_and_problems domain_def {std::cout << "finish definition of domain" << std::endl;}
                     |  domains_and_problems problem_def {std::cout << "finish definition of problem" << std::endl;}
                     ;


/* ====================================================================== */
/* Domain definitions. */

domain_def : '(' define '(' domain name ')' { make_domain($5); }
               domain_body ')'
           ;
// requirement define
domain_body : /* empty */
            | require_def
            | require_def domain_body2
            | domain_body2
            ;
// type define
domain_body2 : types_def
             | types_def domain_body3
             | domain_body3
             ;
/*
 from body3 to body 9 show that constant, predicate, function order is arbitary
  but the last one must be structure_defs
*/
domain_body3 : constants_def
             | predicates_def
			 | predicates_def observables_def
             | functions_def
             | constants_def domain_body4
             | predicates_def domain_body5
             | predicates_def observables_def domain_body5
             | functions_def domain_body6
             | structure_defs
             ;

domain_body4 : predicates_def
			 | predicates_def observables_def
             | functions_def
             | predicates_def domain_body7
             | predicates_def observables_def domain_body7
             | functions_def domain_body8
             | structure_defs
             ;

domain_body5 : constants_def
             | functions_def
             | constants_def domain_body7
             | functions_def domain_body9
             | structure_defs
             ;

domain_body6 : constants_def
             | predicates_def
   | predicates_def observables_def
             | constants_def domain_body8
             | predicates_def domain_body9
             | structure_defs
             ;

domain_body7 : functions_def 
             | functions_def structure_defs
             | structure_defs
             ;

domain_body8 : predicates_def
			 | predicates_def observables_def
             | predicates_def structure_defs
             | predicates_def observables_def structure_defs
             | structure_defs
             ;

domain_body9 : constants_def
             | constants_def structure_defs
             | structure_defs
             ;

/**
  * here show that using { c++ language } to print message
  */
observables_def : '('  OBSERVABLES {// cout << "ob tokens"<<endl;
}  observable_decls ')'{// cout << "done ob tokens"<<endl;
}
                ;
// 结构递归定义
structure_defs : structure_def
               | structure_defs structure_def
               ;
// 结构即action
structure_def : action_def
              ;

// requirement定义即 (requirent k1 k2 k3 k4)
require_def : '(' REQUIREMENTS require_keys ')'
            ;

require_keys : require_key
             | require_keys require_key
             ;
// 这块的大写用到了tokenizer.ll的定义的正则表达式，随后用{调用c++操作}
require_key : STRIPS { requirements->strips = true; }
            | TYPING { requirements->typing = true; }
            | NEGATIVE_PRECONDITIONS
                { requirements->negative_preconditions = true; }// 否定条件 :negative-preconditions
            | DISJUNCTIVE_PRECONDITIONS
                { requirements->disjunctive_preconditions = true; }// 析取条件 :disjunctive-preconditions
            | EQUALITY { requirements->equality = true; }
            | EXISTENTIAL_PRECONDITIONS
                { requirements->existential_preconditions = true; }
            | UNIVERSAL_PRECONDITIONS
                { requirements->universal_preconditions = true; }
            | QUANTIFIED_PRECONDITIONS
                { requirements->quantified_preconditions(); }
            | CONDITIONAL_EFFECTS { requirements->conditional_effects = true; }//条件Effect :conditional-effects
            | PROBABILISTIC_DYNAMICS { requirements->probabilistic = true; }
            | NON_DETERMINISTIC_DYNAMICS { // 开启不确定性 :non-deterministic
				requirements->non_deterministic = true;
				requirements->probabilistic_effects = true;// 这里还是开启了概率Effect
				requirements->probabilistic = false;
				}
            | FLUENTS { requirements->fluents = true; }// :fluents
            | ADL { requirements->adl(); } // :adl
            | DURATIVE_ACTIONS // 这块是不支持的动作，遇到直接跑出异常
                { throw Exception("`:durative-actions' not supported"); }
            | DURATION_INEQUALITIES
                { throw Exception("`:duration-inequalities' not supported"); }
            | CONTINUOUS_EFFECTS
                { throw Exception("`:continuous-effects' not supported"); }
            | PROBABILISTIC_EFFECTS // 使用概率效果 :probabilistic-effects
                {
					requirements->probabilistic_effects = true;
					requirements->probabilistic = true;
					requirements->non_deterministic = false;// 关闭不确定性
					goal_prob_function = domain->functions().add_function("goal-probability");
				}
            | REWARDS// 开启reward
                {
					requirements->rewards = true;
					reward_function = domain->functions().add_function("reward");
				}
            | MDP // 支持Markov
                {
					requirements->probabilistic_effects = true;
					requirements->rewards = true;
					goal_prob_function = domain->functions().add_function("goal-probability");
					reward_function = domain->functions().add_function("reward");
				}
            ;

/* 类型定义： 
	遇到头部 :types 
		1. 调用require_typing()判断是否有requiremnt
		2. 设置名字类型
		3. typed_names可以识别多个类型名
		4. 类型至空
*/
types_def : '(' TYPES { require_typing(); name_kind = TYPE_KIND; }
              typed_names ')' { name_kind = VOID_KIND; }
          ;
/* 常量头部 :constants*/
constants_def : '(' CONSTANTS { name_kind = CONSTANT_KIND; } typed_names ')'
                  { name_kind = VOID_KIND; }
              ;
/* 谓词头部 :predicates 随后读取谓词*/
predicates_def : '(' PREDICATES predicate_decls ')'
               ;
/* Function头部 :function 随后读取函数*/
functions_def : '(' FUNCTIONS { require_fluents(); } function_decls ')'
              ;


/* ====================================================================== */
/* Predicate and function declarations. and observables */

predicate_decls : /* empty */
                | predicate_decls predicate_decl
                ;
/* 单个谓词定义 识别谓词和相应的参数*/
predicate_decl : '(' predicate { make_predicate($2); } variables ')'
                   { parsing_predicate = false; }
               ;

function_decls : /* empty */
               | function_decl_seq
               | function_decl_seq function_type_spec function_decls
               ;

function_decl_seq : function_decl
                  | function_decl_seq function_decl
                  ;

function_type_spec : '-' { require_typing(); } function_type
                   ;

function_decl : '(' function { make_function($2); } variables ')'
                  { parsing_function = false; }
              ;

observable_decls : 
				 | observable_decls observable_decl 
                 ;

observable_decl : '(' predicate  
					{ make_predicate($2);
					// make_observable($2); parsing_obs_token = false;
					}  variables ')' 
					{ //parsing_obs_token = false;
 					  parsing_predicate = false; 
					}
                ;


/* ====================================================================== */
/* Action definitions. 
*/
action_def : '(' ACTION name {// cout << *$3<<endl;
               make_action($3);  }
               parameters action_body ')' { add_action(); }
           | '(' EVENT name {
               make_action($3);  }
               parameters action_body ')' { add_event(); }
           ;
/*
  设置动作参数类型 :paremeter()
*/
parameters : /* empty */
           | PARAMETERS '(' variables ')'
           ;
/* 即一个action可以没有前提条件，如果为空*/
action_body : precondition action_body2
            | action_body2
            ;

action_body2 : action_body3
             | effect action_body3
             | effect              
             ;

action_body3 : /* empty */ { std::cout << "empty observations\n"; }
             | observations
             ;


precondition : PRECONDITION formula { if($2 != NULL) action->set_precondition(*$2); }
             ;

effect : EFFECT eff_formula { action->set_effect(*$2); }
       ;
/* :observation(
	and 
	) 
*/
observations : OBSERVE '(' observation_defs ')'{action->set_observation(*$3);}
             | OBSERVATION '(' and  observation_defs ')' {action->set_observation(*$4);}
             | OBSERVATION  '(' observation_defs ')' {action->set_observation(*$3); // cout << "parse ob"<<endl;
};

/* ====================================================================== */
/* Effect formulas. */

eff_formula : p_effect 
            | '(' and eff_formulas ')' { $$ = $3; }
            | '(' forall { prepare_forall_effect(); } '(' variables ')'
                eff_formula ')' { $$ = make_forall_effect(*$7); }
            | '(' when { require_conditional_effects(); } formula
                eff_formula ')' { $$ = &ConditionalEffect::make(*$4, *$5); }
            | '(' probabilistic prob_effs ')' { if($3->size() ==1 && $3->probability(0) == 1.0){ $$=&$3->effect(0);} else $$ = $3; }
            | '(' ONEOF oneof_effs ')' { $$ = $3; }
            | '(' OR or_effs ')' { $$ = $3; }
            | '(' UNKNOWN unknown_effs ')' { $$ = $3; }
            ;

eff_formulas : /* empty */ { $$ = new ConjunctiveEffect(); }
             | eff_formulas eff_formula { $$ = $1; $$->add_conjunct(*$2); }
             ;

prob_effs : probability eff_formula
               {
 		$$ = new ProbabilisticEffect();
 		add_effect_outcome(*$$, $1, *$2);
 	      }
           | prob_effs probability eff_formula
              { $$ = $1; add_effect_outcome(*$$, $2, *$3); }
           | f_exp eff_formula
              {
                //$1->print(cout,problem->domain().functions(),problem->terms()); 
		$$ = new ProbabilisticEffect();
		add_feffect_outcome(*$$, $1, *$2);
	      }
          |prob_effs f_exp eff_formula

          {
	    //$2->print(cout,problem->domain().functions(),problem->terms()); 
	   $$ = $1; add_feffect_outcome(*$$, $2, *$3); }
          ;

oneof_effs : oneof_effs eff_formula
             { $$ = $1; add_effect_outcome(*$$, new Rational(-1.0), *$2); }
           | eff_formula
{ $$ = new ProbabilisticEffect(); add_effect_outcome(*$$, new Rational(-1.0), *$1); }
           ;

or_effs : or_effs eff_formula
             { $$ = $1; add_effect_outcome(*$$, new Rational(-3.0), *$2); }
           | eff_formula
			 { $$ = new ProbabilisticEffect(); add_effect_outcome(*$$, new Rational(-3.0), *$1); }
;

unknown_effs : p_effect
               { $$ = new ProbabilisticEffect(); 
                 add_effect_outcome(*$$, new Rational(-2.0), *$1); }
             ;



probability : NUMBER 
            ;

p_effect : atomic_term_formula {$$ = make_add_effect(*$1); }
         | '(' not atomic_term_formula ')' { $$ = make_delete_effect(*$3); }
         | '(' assign_op { effect_fluent = true; } f_head f_exp ')'
             { $$ = make_assignment_effect($2, *$4, *$5); }
         ;

assign_op : assign { $$ = Assignment::ASSIGN_OP; }
          | scale_up { $$ = Assignment::SCALE_UP_OP; }
          | scale_down { $$ = Assignment::SCALE_DOWN_OP; }
          | increase { $$ = Assignment::INCREASE_OP; }
          | decrease { $$ = Assignment::DECREASE_OP; }
          ;

/* ====================================================================== */
/* Observation formulas. */

observation_defs : observation_defs observation {$$->add_entry($2); }
                 | observation {$$ = new Observation(); $$->add_entry($1); }
                 ;
/* momo007: add the last case */
observation : '('  formula  probability probability ')' {$$ = make_observation(*$2, *$3, *$4);} 
| '('  formula  formula probability ')' {$$ = make_observation(*$2, *$3 , *$4); } 
|  '(' when formula '(' probabilistic prob_effs ')' ')' {$$ = make_observation(*$3, *$6); $6->setObservation();} 
|  formula { $$ = make_observation(*$1,Rational(0.5),Rational(0.5));}
            ;

//observation_cpt : observation_cpt observation_row {$$->add_cpt_entry($2); }
//                | observation_row {$$ = new ObservationCpt(); $$->add_cpt_entry($1); }
//                ;


//observation_row : '(' when formula '(' probabilistic prob_effs ')' ')' {$$ = make_observation(*$3, *$6);} 
//;



/* ====================================================================== */
/* Problem definitions. */

problem_def : '(' define '(' problem name ')' '(' PDOMAIN name ')'
                { make_problem($5, $9); } problem_body ')'
                { problem->instantiate_actions(); problem->instantiate_events(); delete requirements; }
            | '(' define '(' problem name ')' '(' PDOMAIN name ')'
               { make_problem($5, $9); } problem_body ')' '(' plantime NUMBER ')'
               { problem->instantiate_actions();  problem->instantiate_events(); problem->set_plan_time(*$16);
	         delete $16; delete requirements; }
             ;

/* (:requirements) is optional */
problem_body     : require_def problem_body_r
                 | problem_body_r
                 ;

/* 
    MoMo007 09.08 fix the bug
    init and goal/metric are mandatory 
	先定义object 随后定义 init 和 goal
*/
problem_body_r  : init  goal_spec
                | object_decl init  goal_spec   ;

/* 1st: horizon, object, or discount */
problem_body_ig  : /* empty */
                 | horizon_decl problem_body_h
                 | object_decl {std::cout << "finish object 1" << std::endl;} problem_body_o
                 | discount problem_body_d
                 ;

/* 2nd: horizon, followed by object or discount */
problem_body_h   : /* empty */
                 | object_decl {std::cout << "finish object 2" << std::endl;} problem_body_ho
                 | discount problem_body_hd
                 ;

problem_body_ho  : /* empty */
                 | discount
                 ;

problem_body_hd  : /* empty */
                 | object_decl {std::cout << "finish object 3" << std::endl;}
                 ;

/* 2nd: object, followed by horizon or discount */
problem_body_o   : /* empty */{std::cout << "empty problem_body_o" << std::endl;}
                 | horizon_decl problem_body_oh
                 | discount problem_body_od
                 ;

problem_body_oh  : /* empty */{std::cout << "empty problem_body_oh" << std::endl;}
                 | discount
                 ;

problem_body_od  : /* empty */
                 | horizon_decl
                 ;

/* 2nd: discount, followed by object or horizon */
problem_body_d   : /* empty */
                 | horizon_decl problem_body_dh
                 | object_decl {std::cout << "finish object" << std::endl;} problem_body_do
                 ;

problem_body_dh  : /* empty */
                 | object_decl{std::cout << "finish object" << std::endl;}
                 ;

problem_body_do  : /* empty */
                 | horizon_decl
                 ;

// :objects
object_decl  : /*empty object list*/
             | '(' OBJECTS { name_kind = OBJECT_KIND; std::cout << "start object:" << std::endl;} typed_names ')'
                { name_kind = VOID_KIND; std::cout << "end object" << std::endl;}
            ;

horizon_decl : '(' HORIZON NUMBER ')' {problem->set_plan_horizon(*$3);}
             | ANTI_COMMENT HORIZON NUMBER ')' {problem->set_plan_horizon(*$3);}
             ;

/**
    MoMo007 2022.09.9
    INIT init_element init_elements
    do not support the negative atom, or formula
    we only use the ( and conjuncts) case.
*/
init : '(' INIT init_element {std::cout << "init_element " << std::endl;} init_elements ')' 
     | '(' INIT '(' and conjuncts ')' 
       { std::cout << "init and conjuncts\n"; problem->set_init_formula(*$5); get_init_elts();}  ')'
     ;



init_elements : /* empty */ {std::cout << "empty init" << std::endl;}
              | init_elements init_element {std::cout << "init element" << std::endl;}
              ;

init_element :	atomic_name_formula 
				{
					std::cout << "atom formula" << std::endl; 
					problem->add_init_atom(*$1);  problem->add_init_effect(*(new AddEffect(*$1)));}
             | '(' '=' ground_f_head NUMBER ')'
                 { problem->add_init_value(*$3, *$4); delete $4; }
             | '(' probabilistic prob_inits ')'
                 { problem->add_init_effect(*$3); }
            | '(' ONEOF oneof_inits ')'
				{ std::cout << "oneof_init" << std::endl; problem->add_init_effect(*$3); }
            | '(' UNKNOWN unknown_inits ')' { problem->add_init_effect(*$3); }
| /* none */
             ;

prob_inits : /*f_exp simple_init
               {
		 $$ = new ProbabilisticEffect();
		 add_effect_outcome(*$$, &($1->value(problem->init_values())), *$2);
	       }
           | prob_inits f_exp simple_init
	   { $$ = $1; add_effect_outcome(*$$, &($2->value(problem->init_values())), *$3); }*/
         probability simple_init
               {
		 $$ = new ProbabilisticEffect();
		 add_effect_outcome(*$$, $1, *$2);
	       }
           | prob_inits probability simple_init
	   { $$ = $1; add_effect_outcome(*$$, $2, *$3); }
           ;

oneof_inits : oneof_inits simple_init
             { $$ = $1; add_effect_outcome(*$$, new Rational(-1.0), *$2); }
           | simple_init
{ $$ = new ProbabilisticEffect(); add_effect_outcome(*$$, new Rational(-1.0), *$1); }
           ;

unknown_inits : simple_init
               { $$ = new ProbabilisticEffect(); 
                 add_effect_outcome(*$$, new Rational(-2.0), *$1); }
             ;

simple_init : one_init {$$ = $1;}
            | '(' and one_inits ')' { $$ = $3; }
            ;

one_inits : /* empty */ { $$ = new ConjunctiveEffect(); }
          | one_inits one_init { $$ = $1; $$->add_conjunct(*$2); }
          ;

one_init : atomic_name_formula { $$ = make_add_effect(*$1); }
         | '(' '=' ground_f_head value ')'
             { $$ = make_assignment_effect(Assignment::ASSIGN_OP, *$3, *$4); }
         ;

value : NUMBER { $$ = new Value(*$1); delete $1; }
      ;
/*这块设置goal formula,还有可选的goal reward */
goal_spec : '(' GOAL formula ')' goal_reward { problem->set_goal(*$3, true); }
          | '(' GOAL formula NUMBER ')' goal_reward { problem->set_goal(*$3, *$4); delete $4; }
          | '(' GOAL formula ')' discount goal_reward { problem->set_goal(*$3, true); }
          | metric_spec 
          ;

discount : '(' DISC NUMBER ')' {set_discount(*$3);}
         | ANTI_COMMENT DISC NUMBER ')' {set_discount(*$3);}
         ;

goal_reward : metric_spec
            | '(' GOAL_REWARD ground_f_exp ')' metric_spec
                { set_goal_reward(*$3); }
            ;

metric_spec : /* empty */ { std::cout << "Set default metric()" << std::endl; set_default_metric(); }
            | '(' METRIC maximize { metric_fluent = true; } ground_f_exp ')'
                { problem->set_metric(*$5); metric_fluent = false; }
            | '(' METRIC minimize { metric_fluent = true; } ground_f_exp ')'
                { problem->set_metric(*$5, true); metric_fluent = false; }
            ;

/* ====================================================================== */
/* Formulas. */


formula : // empty 
		 '(' ')'{ $$ = new Conjunction(); std::cout << "empty()\n";}
		| atomic_term_formula { $$ = $1; }
        | '(' '=' term_or_f_exp
            { first_eq_term = eq_term; first_eq_expr = eq_expr; }
            term_or_f_exp ')' { $$ = make_equality(); }
        | '(' binary_comp { require_fluents(); } f_exp f_exp ')'
            { $$ = new Comparison($2, *$4, *$5); }
        | '(' not formula ')' { $$ = make_negation(*$3); }
        | '(' and conjuncts ')' { $$ = $3; }
        | '(' or { require_disjunction(); } disjuncts ')' { $$ = $4; }
        | '(' ONEOF { require_disjunction(); } oneof_disjuncts ')' { $$ = $4; }
        | '(' imply formula formula ')' { $$ = make_implication(*$3, *$4); }
        | '(' exists { prepare_exists(); } '(' variables ')' formula ')'
            { $$ = make_exists(*$7); }
        | '(' forall { prepare_forall(); } '(' variables ')' formula ')'
            { $$ = make_forall(*$7); }
        ;

conjuncts : /* empty */ { $$ = new Conjunction(); }
          | conjuncts formula { $$->add_conjunct(*$2); }
          ;

disjuncts : /* empty */ { $$ = new Disjunction(); }
          | disjuncts formula { $$->add_disjunct(*$2); }
          ;
oneof_disjuncts : /* empty */ { $$ = new OneOfDisjunction(); }
          | oneof_disjuncts formula { $$->add_oneof_disjunct(*$2); }
          ;

atomic_term_formula : '(' predicate { prepare_atom($2); } terms ')'
                        { $$ = make_atom(); }
                    | predicate { prepare_atom($1); $$ = make_atom(); }
                    ;

atomic_name_formula : '(' predicate { prepare_atom($2); } names ')'
                        { $$ = make_atom(); }
                    | predicate { prepare_atom($1); $$ = make_atom(); }
                    ;

binary_comp : '<' { $$ = Comparison::LT_CMP; }
            | LE { $$ = Comparison::LE_CMP; }
            | GE { $$ = Comparison::GE_CMP; }
            | '>' {$$ = Comparison::GT_CMP; }
            ;


/* ====================================================================== */
/* Function expressions. */

f_exp : NUMBER { $$ = new Value(*$1); delete $1; }
      | '(' '+' f_exp f_exp ')' { $$ = new Addition(*$3, *$4); }
      | '(' '-' f_exp opt_f_exp ')' { $$ = make_subtraction(*$3, $4); }
      | '(' '*' f_exp f_exp ')' { $$ = new Multiplication(*$3, *$4); }
      | '(' '/' f_exp f_exp ')' { $$ = new Division(*$3, *$4); }
      | f_head { $$ = $1; }
      ;

term_or_f_exp : NUMBER
                  { require_fluents(); eq_expr = new Value(*$1); delete $1; }
              | '(' '+' { require_fluents(); } f_exp f_exp ')'
                  { eq_expr = new Addition(*$4, *$5); }
              | '(' '-' { require_fluents(); } f_exp opt_f_exp ')'
                  { eq_expr = make_subtraction(*$4, $5); }
              | '(' '*' { require_fluents(); } f_exp f_exp ')'
                  { eq_expr = new Multiplication(*$4, *$5); }
              | '(' '/' { require_fluents(); } f_exp f_exp ')'
                  { eq_expr = new Division(*$4, *$5); }
              | '(' function { require_fluents(); prepare_application($2); }
                  terms ')' { eq_expr = make_application(); }
              | name { make_eq_name($1); }
              | variable { eq_term = make_term($1); eq_expr = NULL; }
              ;

opt_f_exp : /* empty */ { $$ = NULL; }
          | f_exp
          ;

f_head : '(' function { prepare_application($2); } terms ')'
           { $$ = make_application(); }
       | function { prepare_application($1); $$ = make_application(); }
       ;

ground_f_exp : NUMBER { $$ = new Value(*$1); delete $1; }
             | '(' '+' ground_f_exp ground_f_exp ')'
                 { $$ = new Addition(*$3, *$4); }
             | '(' '-' ground_f_exp opt_ground_f_exp ')'
                 { $$ = make_subtraction(*$3, $4); }
             | '(' '*' ground_f_exp ground_f_exp ')'
                 { $$ = new Multiplication(*$3, *$4); }
             | '(' '/' ground_f_exp ground_f_exp ')'
                 { $$ = new Division(*$3, *$4); }
             | ground_f_head { $$ = $1; }
             | GOAL_PROBABILITY
                 { prepare_application($1); $$ = make_application(); }
             ;

opt_ground_f_exp : /* empty */ { $$ = NULL; }
                 | ground_f_exp
                 ;

ground_f_head : '(' function { prepare_application($2); } names ')'
                  { $$ = make_application(); }
                | function
                    { prepare_application($1); $$ = make_application(); }
              ;


/* ====================================================================== */
/* Terms and types. */

terms : /* empty */
      | terms name { add_term($2); }
      | terms variable { add_term($2); }
      ;

names : /* empty */
      | names name { add_term($2); }
      ;

variables : /* empty */
          | variable_seq { add_variables($1, OBJECT_TYPE); }
          | variable_seq type_spec { add_variables($1, $2); } variables
          ;

variable_seq : variable { $$ = new std::vector<const std::string*>(1, $1); }
             | variable_seq variable { $$ = $1; $$->push_back($2); }
             ;

typed_names : /* empty */{std::cout << "empty types line " << line_number << std::endl;}
            | name_seq { add_names($1, OBJECT_TYPE); }
            | name_seq type_spec { add_names($1, $2); } typed_names
            ;

name_seq : name { $$ = new std::vector<const std::string*>(1, $1); }
         | name_seq name { $$ = $1; $$->push_back($2); }
         ;

type_spec : '-' { require_typing(); } type { $$ = $3; }
          ;

type : object { $$ = OBJECT_TYPE; }
     | type_name { $$ = make_type($1); }
     | '(' either types ')' { $$ = make_type(*$3); delete $3; }
     ;

types : object { $$ = new TypeSet(); }
      | type_name { $$ = new TypeSet(); $$->insert(make_type($1)); }
      | types object { $$ = $1; }
      | types type_name { $$ = $1; $$->insert(make_type($2)); }
      ;

function_type : number
              ;


/* ====================================================================== */
/* Tokens. */

define : DEFINE { delete $1; }
       ;

domain : DOMAIN_TOKEN { delete $1; }
       ;

problem : PROBLEM { delete $1; }
        ;

when : WHEN { delete $1; }
     ;

not : NOT { delete $1; }
    ;

and : AND { delete $1; }
    ;

or : OR { delete $1; }
   ;

imply : IMPLY { delete $1; }
      ;

exists : EXISTS { delete $1; }
       ;

forall : FORALL { delete $1; }
       ;

probabilistic : PROBABILISTIC { delete $1; }
              ;

assign : ASSIGN { delete $1; }
       ;

scale_up : SCALE_UP { delete $1; }
         ;

scale_down : SCALE_DOWN { delete $1; }
           ;

increase : INCREASE { delete $1; }
         ;

decrease : DECREASE { delete $1; }
         ;

minimize : MINIMIZE { delete $1; }
         ;

maximize : MAXIMIZE { delete $1; }
         ;

number : NUMBER_TOKEN { delete $1; }
       ;

object : OBJECT_TOKEN { delete $1; }
       ;

either : EITHER { delete $1; }
       ;

plantime : PLANTIME { delete $1; }
       ;

type_name : DEFINE | DOMAIN_TOKEN | PROBLEM
          | EITHER
          | MINIMIZE | MAXIMIZE
          | NAME
          ;

predicate : type_name
          | OBJECT_TOKEN | NUMBER_TOKEN
          ;



function : name
         ;

name : DEFINE | DOMAIN_TOKEN | PROBLEM
     | NUMBER_TOKEN | OBJECT_TOKEN | EITHER
     | WHEN | NOT | AND | OR | IMPLY | EXISTS | FORALL | PROBABILISTIC
     | ASSIGN | SCALE_UP | SCALE_DOWN | INCREASE | DECREASE
     | MINIMIZE | MAXIMIZE
     | NAME
     ;

variable : VARIABLE
         ;

%%

/* Outputs an error message. */
static void yyerror(const std::string& s) {
	std::cerr << PACKAGE ":" << current_file << ':' << line_number << ": " << s << std::endl;
	success = false;
}


/* Outputs a warning. */
static void yywarning(const std::string& s) {
	if (warning_level > 0) {
		std::cerr << PACKAGE ":" << current_file << ':' << line_number << ": " << s << std::endl;
		if (warning_level > 1) {
			success = false;
		}
	}
}


/* Creates an empty domain with the given name. */
static void make_domain(const std::string* name) {
	domain = new Domain(*name);
	domains[*name] = domain;// 添加映射关系
	requirements = &domain->requirements;
	problem = NULL;
	delete name;
}



/* Creates an empty problem with the given name. */
static void make_problem(const std::string* name, const std::string* domain_name) {
	// 获取problem对应的domain是否存在
	std::map<std::string, Domain*>::const_iterator di = domains.find(*domain_name);
	if (di != domains.end()) {
		domain = (*di).second;
	}
	// domain不存在直接结束
	else {
		domain = new Domain(*domain_name);
		domains[*domain_name] = domain;
		yyerror("undeclared domain `" + *domain_name + "' used");
  	}
	// 获取domain的requirement
	requirements = new Requirements(domain->requirements);
	problem = new Problem(*name, *domain);
	my_problem = problem;
	// 判断是否需要reward
	if (requirements->rewards) {
		Action* noopAct = new Action("noop_action");
		const Application& reward_appl =
			Application::make_application(reward_function, TermList());
		const Assignment* reward_assignment =
			new Assignment(Assignment::ASSIGN_OP, reward_appl, *new Value(0.0));
		noopAct->set_effect(*new AssignmentEffect(*reward_assignment));
		//add_action(noopAct);
		problem->add_action(noopAct);
     
//     const Application& reward_appl =
//       Application::make_application(reward_function, TermList());
//     const Assignment* reward_assignment =
//       new Assignment(Assignment::ASSIGN_OP, reward_appl, *new Value(0.0));
//     problem->add_init_effect(*new AssignmentEffect(*reward_assignment));
	}
	delete name;
	delete domain_name;
}


/* Adds :typing to the requirements. */
static void require_typing() {
	if (!requirements->typing) {
		yywarning("assuming `:typing' requirement");
		requirements->typing = true;
	}
}


/* Adds :fluents to the requirements. */
static void require_fluents() {
	if (!requirements->fluents) {
		yywarning("assuming `:fluents' requirement");
		requirements->fluents = true;
	}
}


/* Adds :disjunctive-preconditions to the requirements. */
static void require_disjunction() {
	if (!requirements->disjunctive_preconditions) {
		yywarning("assuming `:disjunctive-preconditions' requirement");
		requirements->disjunctive_preconditions = true;
	}
}


/* Adds :conditional-effects to the requirements. */ 
static void require_conditional_effects() {
	if (!requirements->conditional_effects) {
		yywarning("assuming `:conditional-effects' requirement");
		requirements->conditional_effects = true;
	}
}


/* Returns a simple type with the given name. */
static Type make_type(const std::string* name) {
	std::pair<Type, bool> t = domain->types().find_type(*name);
	if (!t.second)
	{
		t.first = domain->types().add_type(*name);
		
		if (name_kind != TYPE_KIND)
			yywarning("implicit declaration of type `" + *name + "'");
	}

	delete name;
	return t.first;
}


/* Returns the union of the given types. */
static Type make_type(const TypeSet& types) {
	return domain->types().add_type(types);
}


/* Returns a simple term with the given name. */
static Term make_term(const std::string* name) {
	if ((*name)[0] == '?') {
		std::pair<Variable, bool> v = context.find(*name);
		if (!v.second) {
			if (problem != NULL)
				v.first = problem->terms().add_variable(OBJECT_TYPE);
			else
				v.first = domain->terms().add_variable(OBJECT_TYPE);

			context.insert(*name, v.first);
			yyerror("free variable `" + *name + "' used");
    	}
		delete name;
		return v.first;
	} 
	else {	
		TermTable& terms = (problem != NULL) ? problem->terms() : domain->terms();
		
		const PredicateTable& predicates = (parsing_obs_token ? domain->observables() : domain->predicates());
		
		std::pair<Object, bool> o = terms.find_object(*name);

		if (!o.second) {
			size_t n = term_parameters.size();
			if (parsing_atom && predicates.arity(atom_predicate) > n) {
				o.first = terms.add_object(*name, predicates.parameter(atom_predicate, n));
		} else {
			o.first = terms.add_object(*name, OBJECT_TYPE);
		}
			yywarning("implicit declaration of object `" + *name + "'");
		}
		delete name;
		return o.first;
	}
}


/* Creates a predicate with the given name. */
static void make_predicate(const std::string* name) {
	repeated_predicate = false;
	std::pair<Predicate, bool> p = domain->predicates().find_predicate(*name);
	if (!p.second) {
		//    cout << "Make pred " << *name << endl;
		p.first = domain->predicates().add_predicate(*name);
	} else {
		repeated_predicate = true;
		yywarning("ignoring repeated declaration of predicate `" + *name + "'");
	}
	predicate = p.first;
	parsing_predicate = true;
	delete name;
}

/* Creates a observation token with the given name. */
static void make_observable(const std::string* name) {
	repeated_predicate = false;
	std::cout << "observable: " << *name <<std::endl;
	std::pair<Predicate, bool> p = domain->observables().find_predicate(*name);
	if (!p.second) {
		p.first = domain->observables().add_predicate(*name);
	} else {
		repeated_predicate = true;
		yywarning("ignoring repeated declaration of observable `" + *name + "'");
	}
	predicate = p.first;
	//  parsing_obs_token = true;
	parsing_predicate = true;
	delete name;
}

//static ObservationCptRow* make_observation(
static ObservationEntry* make_observation(
					   const StateFormula& form,
					   const ProbabilisticEffect& eff){
	//  ObservationCptRow *row = new ObservationCptRow(form, eff);
	OBS_TYPE=OBS_CPT;
	//  const Rational& prob = eff.probability(0);
	// const Atom& symbol = ((SimpleEffect&)eff.effect(0)).atom();
	ObservationEntry *row = new ObservationEntry(form, eff);
	return row;
}

/* Appends new observation entry to list of observation entries */
static ObservationEntry* make_observation(
			       const StateFormula& form,
			       const Rational& posprob,
			       const Rational& negprob){
	//  cout << "make entry"<<endl;
	OBS_TYPE=OBS_TPFP;
	if (typeid(form) == typeid(Constant)) {
		std::cout << "token is cosnt"<<std::endl;
	}
/* token.print(cout, ((Domain)problem->domain()).observables(),
    	problem->domain().functions(),
    	problem->terms()); */
	ObservationEntry *ob = new ObservationEntry(form, posprob, negprob);
  	return ob;
}

/* Appends new observation entry to list of observation entries */
static ObservationEntry* make_observation(
			       const StateFormula& form,
			       const StateFormula& o,
			       const Rational& prob){
	//  cout << "make entry"<<endl;
	OBS_TYPE=OBS_TPFP;
	if (typeid(form) == typeid(Constant)) {
		std::cout << "token is cosnt"<<std::endl;
	}

	if (typeid(o) == typeid(Constant)) {
		std::cout << "token is cosnt"<<std::endl;
	}

    /* o.print(cout, ((Domain)problem->domain()).predicates(),
  	      problem->domain().functions(),
  	      problem->terms()); */

	/* token */
	ObservationEntry *ob = new ObservationEntry(form, &o, prob, prob);
  	return ob;
}

/* Creates a function with the given name. */
static void make_function(const std::string* name) {
	repeated_function = false;
	std::pair<Function, bool> f = domain->functions().find_function(*name);
	if (!f.second) {
		f.first = domain->functions().add_function(*name);
	} else {
    	repeated_function = true;
    	if (requirements->rewards && f.first == reward_function) {
    		yywarning("ignoring declaration of reserved function `reward'");
    	} else {
    		yywarning("ignoring repeated declaration of function `" + *name + "'");
    	}
	}
	function = f.first;
	parsing_function = true;
	delete name;
}


/* Creates an action with the given name. */
static void make_action(const std::string* name) {
	context.push_frame();
	action = new ActionSchema(*name);
	delete name;
}


/* Adds the current action to the current domain. */
static void add_action() {
	context.pop_frame();
	if (domain->find_action(action->name()) == NULL) {
		domain->add_action(*action);
	} else {
		yywarning("ignoring repeated declaration of action `" + action->name() + "'");
		delete action;
	}
	action = NULL;
}

/* Adds the current event to the current domain. */
static void add_event() {
	context.pop_frame();
	if (domain->find_event(action->name()) == NULL) {
		domain->add_event(*action);
	} else {
		yywarning("ignoring repeated declaration of event `" + action->name() + "'");
		delete action;
	}
	action = NULL;
}

/* Prepares for the parsing of a universally quantified effect. */ 
static void prepare_forall_effect() {
	if (!requirements->conditional_effects) {
		yywarning("assuming `:conditional-effects' requirement");
		requirements->conditional_effects = true;
	}
	context.push_frame();
	quantified.push_back(NULL_TERM);
}


/* Creates a universally quantified effect. */
static const pEffect* make_forall_effect(const pEffect& effect) {
	context.pop_frame();
	QuantifiedEffect* qeffect = new QuantifiedEffect(effect);
	size_t n = quantified.size() - 1;
	size_t m = n;

	while (is_variable(quantified[n])) n--;
	
	for (size_t i = n + 1; i <= m; i++) {
		qeffect->add_parameter(quantified[i]);
	}

	quantified.resize(n);
	return qeffect;
}


/* Adds an outcome to the given probabilistic effect.*/
static void add_effect_outcome(ProbabilisticEffect& peffect,
			        const Rational* p, const pEffect& effect) {
    std::cout << "*p = " << *p << std::endl; 
    // oneof涉及到non-deterministic效果
    std::cout << "requirements->non_deterministic:" << requirements->non_deterministic;
	if((*p == -1.0 || *p == -2.0 || *p == -3.0) && !requirements->non_deterministic){
		yywarning("assuming `:non-deterministic' requirement");
		requirements->non_deterministic = true;    
		/* requirements->probabilistic_effects = true; */
	}
    // 一般的probability effect
	else if ((*p != -1.0 && *p != -2.0 || *p != -3.0) && !requirements->probabilistic_effects) {
    	yywarning("assuming `:probabilistic-effects' requirement1");
		requirements->probabilistic_effects = true;
	} 
    
	if(*p == -1.0){ // okay, its an oneof nd-effect
	}
	else if(*p == -2.0){ // okay, its an unknown nd-effect
	}
	else if(*p == -3.0){ // okay, its an or nd-effect
	}
	else if (*p < 0.0 || *p > 1.0) {
		yyerror("outcome probability needs to be in the interval [0,1]");
	}
	if (!peffect.add_outcome(*p, effect)) {
		yyerror("effect outcome probabilities add up to more than 1");
	}
	delete p;
}
static void add_feffect_outcome(ProbabilisticEffect& peffect,
			        const Expression* p, const pEffect& effect) {
   
	if (!requirements->probabilistic_effects) {
		yywarning("assuming `:probabilistic-effects' requirement2");
		requirements->probabilistic_effects = true;
	}
	/* if (*p < 0 || *p > 1) {
		yyerror("outcome probability needs to be in the interval [0,1]");
	} */
	if (!peffect.add_foutcome(*p, effect)) {
		yyerror("effect outcome probabilities add up to more than 1");
	}
	/* cout << "done adding feffect" <<endl; */
	/* delete p; */
}

/* Creates an add effect. */
static const pEffect* make_add_effect(const Atom& atom) {
	domain->predicates().make_dynamic(atom.predicate());
	return new AddEffect(atom);
}


/* Creates a delete effect. */
static const pEffect* make_delete_effect(const Atom& atom) {
	domain->predicates().make_dynamic(atom.predicate());
	return new DeleteEffect(atom);
}


/* Creates an assignment effect. */
static const pEffect* make_assignment_effect(Assignment::AssignOp oper,
					    const Application& application,
					    const Expression& expr) {
	if (requirements->rewards && application.function() == reward_function) {
		if ((oper != Assignment::INCREASE_OP && oper != Assignment::DECREASE_OP) 
			|| typeid(expr) != typeid(Value)) {
				yyerror("only constant reward increments/decrements allowed");
		}
	} else {
		require_fluents();
	}
	effect_fluent = false;
	domain->functions().make_dynamic(application.function());
	const Assignment& assignment = *new Assignment(oper, application, expr);
	return new AssignmentEffect(assignment);
}


/* Adds types, constants, or objects to the current domain or problem. */
static void add_names(const std::vector<const std::string*>* names, Type type) {
	for (std::vector<const std::string*>::const_iterator si = names->begin(); si != names->end(); si++) {
		const std::string* s = *si;
		if (name_kind == TYPE_KIND) {// 1-if
			if (*s == OBJECT_NAME) {// 2-if
				yywarning("ignoring declaration of reserved type `object'");
			} else if (*s == NUMBER_NAME) {
				yywarning("ignoring declaration of reserved type `number'");
			} else {
				std::pair<Type, bool> t = domain->types().find_type(*s);
				if (!t.second) {
				  t.first = domain->types().add_type(*s);
				}
				if (!domain->types().add_supertype(t.first, type)) {
				  yyerror("cyclic type hierarchy");
				}
			}// end 2-if

		} else if (name_kind == CONSTANT_KIND) {// 1-if
			std::pair<Object, bool> o = domain->terms().find_object(*s);
			if (!o.second) {
				domain->terms().add_object(*s, type);
			} else {
				TypeSet components;
				domain->types().components(components, domain->terms().type(o.first));
				components.insert(type);
				domain->terms().set_type(o.first, make_type(components));
			}

		} else { /* name_kind == OBJECT_KIND */
			if (domain->terms().find_object(*s).second) {
				yywarning("ignoring declaration of object `" + *s + "' previously declared as constant");
			} else {
				std::pair<Object, bool> o = problem->terms().find_object(*s);
				if (!o.second) {
					problem->terms().add_object(*s, type);
				} else {
					TypeSet components;
					domain->types().components(components, problem->terms().type(o.first));
					components.insert(type);
					problem->terms().set_type(o.first, make_type(components));
				}
			}
		}// end 1-if
		delete s;
	}// end-for

	delete names;
}


/* Adds variables to the current variable list. */
static void add_variables(const std::vector<const std::string*>* names, Type type) {
	for (std::vector<const std::string*>::const_iterator si = names->begin(); si != names->end(); si++) {
    	const std::string* s = *si;// 获取名字
		// 是普通谓词
		if (parsing_predicate && !parsing_obs_token) {
			if (!repeated_predicate)
				domain->predicates().add_parameter(predicate, type);
		}
		// 函数
		else if (parsing_function && !parsing_obs_token ) {
			if (!repeated_function) {
				domain->functions().add_parameter(function, type);
			}
		}
		// 可观察谓词
		else if(parsing_predicate && parsing_obs_token){
			if (!repeated_function) {
				domain->observables().add_parameter(function, type);
			}
		}
		// term
		else {
			if (context.shallow_find(*s).second) {
				yyerror("repetition of parameter `" + *s + "'");
			} else if (context.find(*s).second) {
				yywarning("shadowing parameter `" + *s + "'");
			}

			Variable var;
			if (problem != NULL) {
				var = problem->terms().add_variable(type);
			} else {
				var = domain->terms().add_variable(type);
			}
			context.insert(*s, var);
			if (!quantified.empty()) {
				quantified.push_back(var);
			} else { /* action != NULL */
				action->add_parameter(var);
			}
		}
		delete s;
	}// end-for
	delete names;
}


/* Prepares for the parsing of an atomic state formula. */ 
static void prepare_atom(const std::string* name) {
	std::pair<Predicate, bool> p;
	if(parsing_obs_token)
		p = domain->observables().find_predicate(*name);
	else
		p = domain->predicates().find_predicate(*name);
	if (!p.second) {
		if(parsing_obs_token)
			atom_predicate = domain->observables().add_predicate(*name);      
		else
			atom_predicate = domain->predicates().add_predicate(*name);
	
		undeclared_atom_predicate = true;
	
		if (problem != NULL)
			yywarning("undeclared predicate `" + *name + "' used");
		else
			yywarning("implicit declaration of predicate `" + *name + "'");
	} 
	else {
		atom_predicate = p.first;
		undeclared_atom_predicate = false;
	}
	term_parameters.clear();
	parsing_atom = true;
	delete name;
}


/* Prepares for the parsing of a function application. */ 
static void prepare_application(const std::string* name) {
	std::pair<Function, bool> f = domain->functions().find_function(*name);
	if (!f.second) {
		appl_function = domain->functions().add_function(*name);
		undeclared_appl_function = true;
		if (problem != NULL)
			yywarning("undeclared function `" + *name + "' used");
		else 
			yywarning("implicit declaration of function `" + *name + "'");
	} else {
		appl_function = f.first;
		undeclared_appl_function = false;
	}
	
	if (requirements->rewards && f.first == reward_function) {
		if (!effect_fluent && !metric_fluent) {
			yyerror("reserved function `reward' not allowed here");
		}
	} else {
		require_fluents();
	}
	term_parameters.clear();
	parsing_application = true;
	delete name;
}


/* Adds a term with the given name to the current atomic state formula. */
static void add_term(const std::string* name) {
    Term term = make_term(name);// 根据名字创建term
    // 获取term table
    const TermTable& terms = (problem != NULL) ? problem->terms() : domain->terms();
    // 当前正在解析atom
    if (parsing_atom) {
        // 根据是否解析obs判断是普通谓词还是可观察变量
        PredicateTable& predicates = (!parsing_obs_token ? domain->predicates() : domain->observables());
        size_t n = term_parameters.size();
        if (undeclared_atom_predicate) {
            predicates.add_parameter(atom_predicate, terms.type(term));
        } else if (predicates.arity(atom_predicate) > n 
                && !domain->types().subtype(terms.type(term),predicates.parameter(atom_predicate,n))) {
            yyerror("type mismatch");
        }
    // 当前在解析application
    } else if (parsing_application) {
        FunctionTable& functions = domain->functions();
        size_t n = term_parameters.size();
        if (undeclared_appl_function) {
            functions.add_parameter(appl_function, terms.type(term));
        } else if (functions.arity(appl_function) > n 
                && !domain->types().subtype(terms.type(term), functions.parameter(appl_function, n))) {
            yyerror("type mismatch");
        }
    }
  term_parameters.push_back(term);
}


/* Creates the atomic formula just parsed. */
static const Atom* make_atom() {
    size_t n = term_parameters.size();
    if(parsing_obs_token){// 当前正在解析observation
        // 参数个数情况判断
        if (domain->observables().arity(atom_predicate) < n) {
            yyerror("too many parameters passed to obs `"
                + domain->observables().name(atom_predicate) + "'");
        } else if (domain->observables().arity(atom_predicate) > n) {
            yyerror("too few parameters passed to obs `"
                + domain->observables().name(atom_predicate) + "'");
        }
    }
    // 当前解析普通atom
    else{
        // 参数个数情况判断
        if (domain->predicates().arity(atom_predicate) < n) {
            yyerror("too many parameters passed to predicate `"
                + domain->predicates().name(atom_predicate) + "'");
        } else if (domain->predicates().arity(atom_predicate) > n) {
            std::cout << "need " << domain->predicates().arity(atom_predicate) << " but " << n << std::endl;
            yyerror("too few parameters passed to predicate `"
                + domain->predicates().name(atom_predicate) + "'");
        }
    }
    parsing_atom = false;
    // 创建atom
    return &Atom::make_atom(atom_predicate, term_parameters);
}


/* Creates the function application just parsed. */
static const Application* make_application() {

    size_t n = term_parameters.size();
    if (domain->functions().arity(appl_function) < n) {
        yyerror("too many parameters passed to function `"
            + domain->functions().name(appl_function) + "'");
    } else if (domain->functions().arity(appl_function) > n) {
        yyerror("too few parameters passed to function `"
            + domain->functions().name(appl_function) + "'");
    }
    parsing_application = false;
    return &Application::make_application(appl_function, term_parameters);
}


/* Creates a subtraction.  term - opt_term */
static const Expression* make_subtraction(const Expression& term,
					  const Expression* opt_term) {
    if (opt_term != NULL) {// 两个数实现减法
        return new Subtraction(term, *opt_term);
    } else {// 一个数实现负数
        return new Subtraction(*new Value(0.0), term);
    }
}


/* Creates an atom or fluent for the given name to be used in an
   equality formula. */
static void make_eq_name(const std::string* name) {
    std::pair<Function, bool> f = domain->functions().find_function(*name);
    if (f.second) {// domain中定义的function
        prepare_application(name);
        eq_expr = make_application();
    } else {
        /* Assume this is a term. */
        eq_term = make_term(name);
        eq_expr = NULL;
    }
}


/* Creates an equality formula. 使用到了 =的前提条件 */
static const StateFormula* make_equality() {
    if (!requirements->equality) {
        yywarning("assuming `:equality' requirement");
        requirements->equality = true;
    }
    // 等号两侧表达式不为空
    if (first_eq_expr != NULL && eq_expr != NULL) {
        return new Comparison(Comparison::EQ_CMP, *first_eq_expr, *eq_expr);
    } else if (first_eq_expr == NULL && eq_expr == NULL) {// 两者均为空
        // 获取term表格
        const TermTable& terms = (problem != NULL) ? problem->terms() : domain->terms();
        // 判断类型是否相同
        if (domain->types().subtype(terms.type(first_eq_term), terms.type(eq_term))
            || domain->types().subtype(terms.type(eq_term),  terms.type(first_eq_term)))
        {
            return new Equality(first_eq_term, eq_term);
        } else {
            return 0;//&StateFormula::FALSE;
        }
    } else {
        yyerror("comparison of term and numeric expression");
        return 0;//&StateFormula::FALSE;
    }
}


/* Creates a negated formula. */
static const StateFormula* make_negation(const StateFormula& negand) {
    if (typeid(negand) == typeid(Atom)) {
        if (!requirements->negative_preconditions) {
            yywarning("assuming `:negative-preconditions' requirement");
            requirements->negative_preconditions = true;
        }
        // 需要否定的formual不是赋值或者比较，则需要支持析取式
        } else if (typeid(negand) != typeid(Equality) && typeid(negand) != typeid(Comparison)) {
            require_disjunction();
        }
    //创建并返回指针
    return &Negation::make_negation(negand);
}


/* Creates an implication. f1 -> f2 */
static const StateFormula* make_implication(const StateFormula& f1, const StateFormula& f2) {
    require_disjunction();// 判断是否有disjunction，没有则warning并开启
    Disjunction* disj = new Disjunction();//创建析取式
    disj->add_disjunct(Negation::make_negation(f1));//创建 !f1 \/ f2 
    disj->add_disjunct(f2);
    return disj;
}


/* Prepares for the parsing of an existentially quantified formula. */
static void prepare_exists() {
	if (!requirements->existential_preconditions) {
		yywarning("assuming `:existential-preconditions' requirement");
		requirements->existential_preconditions = true;
	}
	context.push_frame();
	quantified.push_back(NULL_TERM);
}


/* Prepares for the parsing of a universally quantified formula. */
static void prepare_forall() {
	if (!requirements->universal_preconditions) {
		yywarning("assuming `:universal-preconditions' requirement");
		requirements->universal_preconditions = true;
	}
	context.push_frame();
	quantified.push_back(NULL_TERM);
}


/* Creates an existentially quantified formula. */
static const StateFormula* make_exists(const StateFormula& body) {
	context.pop_frame();
	size_t m = quantified.size() - 1;
	size_t n = m;
	
	while (is_variable(quantified[n])) n--;

	if (n < m) {
		Exists* exists = new Exists();
		
		for (size_t i = n + 1; i <= m; i++) {
			exists->add_parameter(quantified[i]);
		}

		exists->set_body(body);
		quantified.resize(n);
		return exists;
	} else {
		quantified.pop_back();
		return &body;
	}
}


/* Creates a universally quantified formula. */
static const StateFormula* make_forall(const StateFormula& body) {
	context.pop_frame();
	size_t m = quantified.size() - 1;
	size_t n = m;
	while (is_variable(quantified[n])) {
		n--;
	}
	if (n < m) {
		Forall* forall = new Forall();
		for (size_t i = n + 1; i <= m; i++) {
			forall->add_parameter(quantified[i]);
		}
		forall->set_body(body);
		quantified.resize(n);
		return forall;
	} else {
		quantified.pop_back();
		return &body;
	}
}

void set_discount(const Rational& discount){
	problem->set_discount(discount);
}


/* Sets the goal reward for the current problem. */
void set_goal_reward(const Expression& goal_reward) {
    if (!requirements->rewards) {// goal中含有reward判断是否使用reward关键字
        yyerror("goal reward only allowed with the `:rewards' requirement");
    } else {
        const Application& reward_appl =
            Application::make_application(reward_function, TermList());// 创建一个application存储reward的操作
        const Assignment* reward_assignment =
            new Assignment(Assignment::INCREASE_OP, reward_appl, goal_reward);// 对该aplication赋值
        problem->set_goal_reward(*reward_assignment);//设置reward目标
    }
}


/* Sets the default metric for the current problem. */
static void set_default_metric() {
    if (requirements->rewards) {
        const Application& reward_appl =
            Application::make_application(reward_function, TermList());
        problem->set_metric(reward_appl);//设置评估函数
    }
}

/* make all atoms in formula or subformula dynamic */
void make_all_dynamic(const StateFormula &formula){
   if (typeid(formula) == typeid(Constant)) {
    /*
     * The formula is either TRUE or FALSE, so it contains no state
     * variables.
     */
    return;
  }

  const Atom* af = dynamic_cast<const Atom*>(&formula);
  if (af != NULL) {
    /*
     * The formula is an atom representing a single state variable.
     */
    domain->predicates().make_dynamic(af->predicate());
    return;
  }

  const Negation* nf = dynamic_cast<const Negation*>(&formula);
  if (nf != NULL) {
    /*
     * The state variables of a negation are the state variables of the
     * negand.
     */
    make_all_dynamic(nf->negand());
    return;
  }

  const Conjunction* cf = dynamic_cast<const Conjunction*>(&formula);
  if (cf != NULL) {
    /*
     * The state variables of a conjunction are the state variables of
     * the conjuncts.
     */
    for (size_t i = 0; i < cf->size(); i++) {
      make_all_dynamic(cf->conjunct(i));
    }
    return;
  }

  const Disjunction* df = dynamic_cast<const Disjunction*>(&formula);
  if (df != NULL) {
    /*
     * The state variables of a disjunction are the state variables of
     * the disjuncts.
     */
    for (size_t i = 0; i < df->size(); i++) {
      make_all_dynamic(df->disjunct(i));
    }
    return;
  }

  const OneOfDisjunction* odf = dynamic_cast<const OneOfDisjunction*>(&formula);
  if (odf != NULL) {
    /*
     * The state variables of a disjunction are the state variables of
     * the disjuncts.
     */
    for (size_t i = 0; i < odf->size(); i++) {
      make_all_dynamic(odf->disjunct(i));
    }
    return;
  }
  /*
   * No other types of formulas should appear in fully instantiated
   * action preconditions and effect conditions.
   */

}


/* if we use logical formula for init, then need to get atoms holding for later grounding 
   purposes*/
static void get_init_elts(){
  
    const StateFormula &init = problem->init_formula();// 得到初始状态公式
    if(!&init)
        return;
    // 判断是否为和取式
    const Conjunction *c = dynamic_cast<const Conjunction*>(&init);
    if(c != NULL){
        // 将初始状态的每一个atom添加
        for (size_t i = 0; i < c->size(); i++) {
            const Atom *a = dynamic_cast<const Atom*>(&c->conjunct(i));
            if(a != NULL){// 普通atom直接添加
                problem->add_init_atom(*a);	
            }
            else{// oneof等形式的atom，递归处理
                make_all_dynamic(c->conjunct(i));
            }      
        }
    }
    else
        make_all_dynamic(init);
}
