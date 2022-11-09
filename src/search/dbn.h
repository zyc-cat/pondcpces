#ifndef DBN_H
#define DBN_H

#include <map>
#include <list>
#include <set>
#include "dd.h"
#include "expressions.h"
#include <ostream>
#include "effects.h"

class dbn_node;
class dbn;

extern DdManager* manager;


//typedef std::pair<std::map<int, int>*, std::map<double, double>* > dbn_row;
//typedef std::map<std::map<int, int>*, std::map<double, double>* > dbn_cpt;
//typedef std::map<std::map<int, int>*, bool> dbn_noops;
std::ostream& operator<<(std::ostream& os, dbn_node& d);
std::ostream& operator<<(std::ostream& os, dbn& d);
//std::ostream& operator<<(std::ostream& os, dbn_cpt& d);

class dbn_node{
 public:
  dbn_node(int v) : var(v) {
    dd_bits= NULL;
    cpt = NULL;
    goalOrSinkRelevant = false;
    //outcomes = NULL;
    //make_noop();
  }
    //  dbn_node(dbn_node* node, dbn* olddbn, dbn* newdbn);
  dbn_node(dbn_node* d){
    this->var = d->var;
    this->cpt = d->cpt;
    if(this->cpt){
      Cudd_Ref(this->cpt);
    }
    //    parents.insert(d->parents.begin(), d->parents.end());
    //    children.insert(d->children.begin(), d->children.end());
    if(d->dd_bits!=NULL){
      dd_bits = new std::set<int>();
      dd_bits->insert(d->dd_bits->begin(), d->dd_bits->end());
    }
    outcomes.insert(d->outcomes.begin(), d->outcomes.end());
    prs.insert(d->prs.begin(), d->prs.end());
    goalOrSinkRelevant = d->goalOrSinkRelevant;
    factored_cpts.insert(factored_cpts.begin(), d->factored_cpts.begin(), d->factored_cpts.end());
    probabilistic_parents.insert(d->probabilistic_parents.begin(),d->probabilistic_parents.end());
    parents.insert(d->parents.begin(),d->parents.end());
    
    //    for(  std::map<int, DdNode*>::iterator o = d->outcomes.begin(); o != d->outcomes.end(); o++){    
    //}

  }
  ~dbn_node() {
    Cudd_RecursiveDeref(manager, cpt);
    if(dd_bits)
      delete dd_bits;
    //delete_cpt(&(this->cpt));
  }

  
  //bool row_matches_aux(dbn_row* row, std::map<int, double>* aux_value);
  //static void delete_cpt(dbn_cpt *cpt);
 
  DdNode* sample_cpt();
  
  std::set<int> parents;
  std::set<int> children;

  std::map<DdNode*,int> probabilistic_parents;
  std::list<DdNode*> effects;
  std::list<DdNode*> factored_cpts;
  DdNode *cpt;
  std::map<int, DdNode*> outcomes;
  std::map<int, double> prs;
  std::set<int>* dd_bits;
  bool goalOrSinkRelevant;
  //std::map<std::map<int, int>*, std::map<int, double>* > 
  //  dbn_cpt cpt;
  //  dbn_noops cpt_noops;

  int var;

  void make_noop(){
    DdNode* x = Cudd_addIthVar(manager, var);
    Cudd_Ref(x);
    DdNode* y = Cudd_addIthVar(manager, var-1);
    Cudd_Ref(y);
    cpt = Cudd_addXeqy(manager, 1, &x, &y);
    Cudd_Ref(cpt);
    Cudd_RecursiveDeref(manager, x);
    Cudd_RecursiveDeref(manager, y);
  }

  bool conditional(int bits);
  void add_noops(int bits);
  
  //bool noop;

  //  int domain; 
/* 0:  constant
		 1:  boolean
		 >1: aux (int)
	      */

};

class dbn {
  public:
  dbn(int num_vars) {
    num_aux_vars = 0;
    a_varmap = NULL;
    variable_elimination_order = NULL;
    var_cubes = NULL;
    not_all_goal_and_sink = Cudd_ReadOne(manager);
    Cudd_Ref(not_all_goal_and_sink);
    all_goal_and_sink = Cudd_ReadZero(manager);
    Cudd_Ref(all_goal_and_sink);
    for(int i = 0; i < num_vars; i++){
      //vars[2*i] = NULL;//new dbn_node(2*i, 1);
      vars[2*i+1] = new dbn_node(2*i+1);
      //      vars[2*i+1]->make_noop();
	//NULL;//new dbn_node(2*i+1, 1);
      //make_noop(vars[2*i], vars[2*i+1]);
    }
    vars[-1] = new dbn_node(-1);
/*     reward_node = new dbn_node(-1, 1); */
    //    reward_node = new dbn_node(mdbn->reward_node, mdbn, this);
    //    std::map<double, double> *rew = new std::map<double, double>();
    //(*rew)[0.0]=1.0;
    //reward_node->cpt[new std::map<int,int>()] = rew;
    //reward_dd = NULL;
  }

  dbn(dbn* mdbn){
    
 /*    num_aux_vars = mdbn->num_aux_vars; */
/*     for(std::map<int, dbn_node*>::iterator i = mdbn->vars.begin(); */
/* 	i != mdbn->vars.end(); i++){ */
/*       vars[(*i).first] = new dbn_node((*i).second, mdbn, this); */
/*     } */
/*     reward_node = new dbn_node(mdbn->reward_node, mdbn, this); */
/*    /\*  reward_node = new dbn_node(mdbn->reward_node, mdbn, this); *\/ */
/* /\*     std::map<double, double> *rew = new std::map<double, double>(); *\/ */
/* /\*     (*rew)[0.0]=1.0; *\/ */
/* /\*     reward_node->cpt[new std::map<int,int>()] = rew; *\/ */
/*     reward_dd = NULL; */
  }


  ~dbn(){
   /*  for(std::map<int, dbn_node*>::iterator i = vars.begin(); */
/* 	i != vars.end(); i++){ */
/*       if((*i).second){ */
/* 	delete (*i).second; */
/*       } */
/*     } */
/*     for(std::map<int, DdNode*>::iterator d = dds.begin();  */
/* 	d != dds.end(); d++){ */
/*       if((*d).second){ */
/* 	Cudd_RecursiveDeref(manager, (*d).second); */
/*       } */
/*     } */

/*     if(reward_node){ */
/*       //std::cout << "del rn" << std::endl; */
/*       delete reward_node; */
/*     } */
/*     if(reward_dd){ */
/*       Cudd_RecursiveDeref(manager, reward_dd); */
/*     } */
  }

  std::map<int, dbn_node*> vars;

  int num_aux_vars;
  //  std::map<int, DdNode*> dds;

  std::map<int, std::map<int, DdNode*>*> aux_var_value_dds;
  //  std::map<int, std::set<int>*> aux_var_bits;

  std::list<std::set<int>*> abstraction_order;
  std::list<DdNode*> abstraction_order_cubes;
  std::list<std::pair<int, bool>*>* variable_elimination_order;
  std::map<int, DdNode*>* var_cubes;

  int* a_varmap;
    DdNode* reward_dd;
    DdNode* not_all_goal_and_sink;
    DdNode* all_goal_and_sink;
  //  dbn_node* reward_node;

    void make_noops(int num_alt_facts){
      for(int i = 0; i < num_alt_facts; i++){
	if(vars[2*i+1] == NULL){
	  vars[2*i+1] = new dbn_node(2*i+1);
	  vars[2*i+1]->make_noop();
	}
      }
    }

    void add_noops(int num_alt_facts){
      int aux_bits = 0;
      for(int i = 0; i < num_aux_vars; i++){
	aux_bits+=vars[2*num_alt_facts+i]->dd_bits->size();
      }
      
      for(int i = 0; i < num_alt_facts; i++){
	
	vars[2*i+1]->add_noops(aux_bits);
      }
    }

    std::list<std::pair<int, bool>*>* get_variable_elimination_order();
    std::map<int, DdNode*>* get_var_cubes();
    static std::map<const pEffect*, dbn_node*>* generate_probabilistic_nodes(const pEffect *Effect);
    static std::map<int, DdNode*>* gen_outcome_dds(std::set<int>* dd_bits, std::map<int, double>* prs, int start_index);
    static    void generate_probabilistic_nodes(const pEffect *effect, std::map<const pEffect*, dbn_node*> *result, int* start_index);
  static dbn* conjoin_dbns(std::list<dbn*> *dbns, 
			   std::map<dbn*, std::map<int, int>*> *dbn_var_map);
  static dbn* condition_dbn(DdNode* c, dbn* mdbn);
  static dbn* probabilistic_dbns(std::list<double>* prs, std::list<dbn*> *dbns, 
				 dbn_node* pr_node
				 
				 );
  static dbn* simple_effect(dbn* mdbn, int var, int value);
  static bool isNoop(dbn_node* node);
  //static dbn* negate_dbn(dbn* mdbn);
  //  void make_noop(dbn_node* parent, dbn_node* child);
  void   apply_rewards_and_sink();
  static dbn* reward(dbn *mdbn, ValueMap *r, dbn_node* prnode);
  //  static void combine_cpts(dbn_cpt* r, dbn_cpt *o, dbn_noops *r1, 
  //		   dbn_noops* o1,  bool t, bool ident, bool r);
  //static void combine_nodes(dbn_node* r, dbn_node *o, bool rb, std::map<int, int>* pd);
/*   static void combine_cpts(dbn_cpt* r, dbn_cpt *o, */
/* 			   dbn_node * n, std::set<int> *p, std::map<int, int>* pd, */
/* 			   bool r); */
  std::list<std::set<int>*>* get_abstraction_order();
  std::list<DdNode*>* get_abstraction_order_cubes();
  int* get_varmap();

  //  std::map<int, DdNode*>* get_dds();
  // double get_matching_reward(std::pair<std::map<int, int>*, std::map<double, double>* >* row);
     DdNode * get_reward_dd();
  // DdNode* nodeToDD(dbn_node* node, bool r);
     //   void gen_aux_var_dds();
  // static dbn_node* map_node(dbn_node* old, std::map<int, int> *var_map);
     DdNode* get_aux_var_cube();
};


#endif 
