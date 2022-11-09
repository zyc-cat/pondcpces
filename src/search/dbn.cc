#include "globals.h"
#include "dbn.h"
#include "graph_wrapper.h"
#include "solve.h"
#include <ostream>
#include <algorithm>
using namespace std;

using namespace std;


DdNode* dbn_node::sample_cpt(){
  double pr = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
  if(var < 2*num_alt_facts || dd_bits == NULL || !RBPF_PROGRESSION 
     || 
     (goalOrSinkRelevant //&& pr < 0.10
       )
	   ){
		return cpt;
	}
	else{
		//		std::cout << "[" << std::endl;
		//			Cudd_CheckKeys(manager);
		//			std::cout << "|" << std::endl;
		DdNode *sample = draw_cpt_samples(cpt, RBPF_SAMPLES, dd_bits);
		//printBDD(cpt);
		//printBDD(sample);


		//			Cudd_CheckKeys(manager);
		//			std::cout << "]" << std::endl;

		return sample;
	}
}

bool dbn_node::conditional(int aux_bits){
	bool isPar = false;

  if(factored_cpts.size() > 0){
    for(std::list<DdNode*>::iterator f= factored_cpts.begin(); f!=factored_cpts.end();f++){
	DdNode* support = Cudd_Support(manager, *f);
	Cudd_Ref(support);

	for(int p = 0; p < 2*num_alt_facts+aux_bits; p++){
		if(p != var && //p != var-1 &&
				Cudd_bddIsVarEssential(manager, support, p, 1)
		){
			isPar = true;
			break;
		}
	}

	Cudd_RecursiveDeref(manager, support);
	if(isPar)
	  break;
    }

  }
  else{
	DdNode* support = Cudd_Support(manager, cpt);
	Cudd_Ref(support);

	for(int p = 0; p < 2*num_alt_facts+aux_bits; p++){
		if(p != var && //p != var-1 &&
				Cudd_bddIsVarEssential(manager, support, p, 1)
		){
			isPar = true;
			break;
		}
	}

	Cudd_RecursiveDeref(manager, support);
	//    cout << isPar << endl;
  }
	return isPar;
}

DdNode* dbn::get_aux_var_cube(){
	set<int> aux_bits;

	for(int i = 0; i < num_aux_vars; i++){
		//printBDD(vars[2*num_alt_facts+i]->cpt);
		aux_bits.insert(vars[2*num_alt_facts+i]->dd_bits->begin(),
				vars[2*num_alt_facts+i]->dd_bits->end());
	}


	DdNode** variables = new DdNode*[aux_bits.size()];
	int k = 0;
	for(set<int>::iterator i = aux_bits.begin(); i != aux_bits.end(); i++){
		variables[k++] = Cudd_addIthVar(manager, *i);
	}

	DdNode* cube = Cudd_addComputeCube(manager, variables, NULL, k);
	Cudd_Ref(cube);
	//  printBDD(cube);
	delete [] variables;

	return cube;
}

void dbn_node::add_noops(int aux_bits){
  //	cout << "add noops " << var << endl;
	//	printBDD(cpt);

	if(cpt == NULL || cpt == Cudd_ReadZero(manager)){
		make_noop();
	}
	else if(conditional(aux_bits)){
	  //  cout << "conditional" << endl;
	  DdNode* tmp = cpt;
	  Cudd_Ref(tmp);
	  make_noop();
	  DdNode* noop = cpt;
	  Cudd_Ref(noop);
		
	  DdNode** variables = new DdNode*[1];
	  variables[0] = Cudd_bddIthVar(manager, var);
	  //    variables[1] = Cudd_bddIthVar(manager, var-1);
	  DdNode* cube = Cudd_bddComputeCube(manager, variables, NULL, 1);// (*i)->size());
	  Cudd_Ref(cube);
	  delete [] variables;

	  DdNode* ncpt;
	  DdNode* ncptNoop;
	  if(0 && (var == (2*(num_alt_facts-2))+1 ||
		   var == (2*(num_alt_facts-1))+1)){
	    DdNode *ncptBAll = Cudd_ReadOne(manager);
	    Cudd_Ref(ncptBAll);
	    for(list<DdNode*>::iterator f = factored_cpts.begin(); f!=factored_cpts.end(); f++){
		DdNode* cptB = Cudd_addBddThreshold(manager, *f, 1.0);
		Cudd_Ref(cptB);	    
		DdNode *cptBNoVar = Cudd_bddExistAbstract(manager, cptB, cube);
		Cudd_Ref(cptBNoVar);  
		DdNode* ncptB = Cudd_Not(cptBNoVar);
		Cudd_Ref(ncptB);
		DdNode* tmp1 = Cudd_bddAnd(manager, ncptBAll, ncptB);
		Cudd_Ref(tmp1);
		Cudd_RecursiveDeref(manager, ncptBAll);
		ncptBAll = tmp1;
		Cudd_Ref(ncptBAll);
		Cudd_RecursiveDeref(manager, tmp1);
		Cudd_RecursiveDeref(manager, cptBNoVar);
		Cudd_RecursiveDeref(manager, cptB);
		Cudd_RecursiveDeref(manager, ncptB);
		//		printBDD(ncptBAll);
	    }
	    ncpt = Cudd_BddToAdd(manager, ncptBAll);
	    Cudd_Ref(ncpt);
	    ncptNoop = Cudd_addApply(manager, Cudd_addTimes, ncpt, noop);
	    Cudd_Ref(ncptNoop);

	    std::cout << "NOOP" << std::endl;
	    printBDD(ncptNoop);
	    for(list<DdNode*>::iterator f = factored_cpts.begin(); f!=factored_cpts.end(); f++){
	      DdNode* t = Cudd_addApply(manager, Cudd_addPlus, *f, ncptNoop);
	      Cudd_Ref(t);
	      Cudd_RecursiveDeref(manager, *f);
	      *f = t;
	      Cudd_Ref(*f);
	      Cudd_RecursiveDeref(manager, t);
	    }
	  }
	  else{
		DdNode* cptB = Cudd_addBddThreshold(manager, tmp, 1.0);
		Cudd_Ref(cptB);
		DdNode *cptBNoVar = Cudd_bddExistAbstract(manager, cptB, cube);
		Cudd_Ref(cptBNoVar);
		DdNode* ncptB = Cudd_Not(cptBNoVar);
		Cudd_Ref(ncptB);
		ncpt = Cudd_BddToAdd(manager, ncptB);
		Cudd_Ref(ncpt);
		ncptNoop = Cudd_addApply(manager, Cudd_addTimes, ncpt, noop);
		Cudd_Ref(ncptNoop);
		Cudd_RecursiveDeref(manager, cpt);
		cpt = Cudd_addApply(manager, Cudd_addPlus, tmp, ncptNoop);
		Cudd_Ref(cpt);

		//printBDD(cpt);


		Cudd_RecursiveDeref(manager, cptBNoVar);
		Cudd_RecursiveDeref(manager, cptB);
		Cudd_RecursiveDeref(manager, ncptB);


	  }

	  Cudd_RecursiveDeref(manager, ncpt);
	  Cudd_RecursiveDeref(manager, ncptNoop);
	  Cudd_RecursiveDeref(manager, tmp);
	  Cudd_RecursiveDeref(manager, noop);
	  Cudd_RecursiveDeref(manager, cube);




		//    printBDD(cube);
		//printBDD(cptB);


		// printBDD(cptBNoVar);


		//printBDD(ncptB);




		//printBDD(ncptNoop);

	}
}

bool isParent(dbn_node* node, dbn_node* parent){

	//    cout << "IsParent?, par = " << (parent->var-1) <<endl;
	//    printBDD(node->cpt);
	//    //  printBDD(parent->cpt);
	bool isPar = false;


       

  if(node->factored_cpts.size()> 0){
    for(list<DdNode*>::iterator f = node->factored_cpts.begin(); f!=node->factored_cpts.end(); f++){
      DdNode* support = Cudd_Support(manager, *f);
	Cudd_Ref(support);


	if(parent->var == node->var){
		isPar = true;
	}
	else if(parent->var < 2*num_alt_facts){ //fact
		isPar = Cudd_bddIsVarEssential(manager, support, parent->var-1, 1);
	}
	else{//aux_var
		//    if(parent->var != node->var){ //aux vars cannot be own parent
	  for(set<int>::iterator bit = parent->dd_bits->begin(); bit != parent->dd_bits->end(); bit++){
	    if(Cudd_bddIsVarEssential(manager, support, *bit, 1)){
	      isPar = true;
	      break;
	    }
		}
		//}
	}

	Cudd_RecursiveDeref(manager, support);
	if(isPar)
	  break;
    }
  }
  else if(node->cpt){
    
	DdNode* support = Cudd_Support(manager, node->cpt);
	Cudd_Ref(support);


	if(parent->var == node->var){
		isPar = true;
	}
	else if(parent->var < 2*num_alt_facts){ //fact
		isPar = Cudd_bddIsVarEssential(manager, support, parent->var-1, 1);
	}
	else{//aux_var
		//    if(parent->var != node->var){ //aux vars cannot be own parent
		for(set<int>::iterator bit = parent->dd_bits->begin(); bit != parent->dd_bits->end(); bit++){
			if(Cudd_bddIsVarEssential(manager, support, *bit, 1)){
				isPar = true;
				break;
			}
		}
		//}
	}

	Cudd_RecursiveDeref(manager, support);
	//    cout << isPar << endl;
  }

	return isPar;
}

std::list<std::pair<int, bool>*>* dbn::get_variable_elimination_order(){

  if(variable_elimination_order == NULL){
    variable_elimination_order = new list<pair<int, bool>*>();


    std::map<int, std::set<int>*> parents;
    std::map<int, std::set<int>*> children;
    for(map<int, dbn_node*>::iterator child = vars.begin(); child != vars.end(); child++){
      if((*child).first == -1 || (*child).first == 2*(num_alt_facts-1)+1)
	continue;
      for(map<int, dbn_node*>::iterator parent = vars.begin(); parent != vars.end(); parent++){
	if((*parent).first == -1 || (*parent).first == 2*(num_alt_facts-1)+1)
	  continue;

// 	if((((*parent).first == (*child).first ||
// 	   (*child).second->parents.find((*parent).first) != (*child).second->parents.end()
// 	   )
// 	   && !isParent((*child).second, (*parent).second))
// ||
// 	  (!((*parent).first == (*child).first ||
// 	   (*child).second->parents.find((*parent).first) != (*child).second->parents.end()
// 	   )
// 	   && isParent((*child).second, (*parent).second))){
// 	  cout << "diff " << (*child).first << " " << (*parent).first << " " << isParent((*child).second, (*parent).second) << endl;
// 	  copy((*child).second->parents.begin(), (*child).second->parents.end(), ostream_iterator<int>(cout, "\n"));
// 	  printBDD((*child).second->cpt);
// 	  for(list<DdNode*>::iterator i = (*child).second->factored_cpts.begin(); i != (*child).second->factored_cpts.end(); i++){
// 	    printBDD(*i);
// 	  }
// 	  exit(0);
// 	}

	if((*parent).first == (*child).first ||
	   (*child).second->parents.find((*parent).first) != (*child).second->parents.end()
	   ){
	   //isParent((*child).second, (*parent).second)){
	  set<int> *p_set = parents[(*child).first];
	  if(p_set == NULL) {
	    p_set = new set<int>();
	    parents[(*child).first] = p_set;
	  }
	  set<int> *c_set = children[(*parent).first];
	  if(c_set == NULL) {
	    c_set = new set<int>();
	    children[(*parent).first] = c_set;
	  }
	  p_set->insert((*parent).first);
	  c_set->insert((*child).first);
	  //	  cout << "P: " << (*parent).first << " C: " << (*child).first << endl;
	}
      }
    }

		//select a var to apply, sort based on fewer number of parents
		// for each parent that can be eliminated, eliminate

		//while children left to apply or parents left to eliminate
		// find child with fewest parents that are inactive
		// (active parents are not elinated, but chidren have been applied)
		// apply child, remove from children
		// eliminate each parent that all children have been applied, remove from active, remove from parents
		set<int> active_parents; //children applied by cannot eliminate yet
		while(parents.size() > 0 || children.size() > 0){
		  int min_child = -1;
		  int min_num_active_parents = -1;


			

		  for(map<int, set<int>*>::iterator child = parents.begin(); child != parents.end(); child++){
		    
		    set<int> intersection;
		    set_intersection(active_parents.begin(),
				     active_parents.end(),
				     (*child).second->begin(),
				     (*child).second->end(),
				     std::insert_iterator<std::set<int> >(intersection, intersection.begin()));
		    int num_active_parents = intersection.size();
		    intersection.clear();
				
		    bool sampledNode = (RBPF_PROGRESSION && (*child).first >= 2*num_alt_facts && !vars[(*child).first]->goalOrSinkRelevant);


		    //if sampledNode, then only apply if all of its children have been applied
		    bool childrenApplied = (!sampledNode || 
					    children.find((*child).first) == children.end() || 
					    children[(*child).first]->size()<=1);
		    
		    if(sampledNode || 
		       (//childrenApplied && 
			(vars[min_child]->goalOrSinkRelevant ||
			 // min_child == 2*(num_alt_facts-2)+1 || min_child == 2*(num_alt_facts-1)+1 ||
			 min_child == -1 || num_active_parents > min_num_active_parents))){
		      min_child = (*child).first;
		      min_num_active_parents = num_active_parents;
		      if(sampledNode)
			break;
		    }
		  }
			//std::cout << "HI " << min_child << std::endl;


			//add child's parents to active parents
			active_parents.insert(parents[min_child]->begin(), parents[min_child]->end());

			//apply child
			variable_elimination_order->push_back(new pair<int, bool>(min_child, false)); //false means apply
			//cout << "Apply " << min_child << endl;


			//remove child
			for(set<int>::iterator parent = parents[min_child]->begin(); parent != parents[min_child]->end(); parent++){
				children[*parent]->erase(min_child);
			}
			delete parents[min_child];
			parents.erase(min_child);

			set<int> toRemove;
			for(map<int, set<int>*>::iterator parent = children.begin(); parent != children.end(); parent++){
				if((*parent).second->size() == 0){//all children have been applied
					//eliminate parent
					variable_elimination_order->push_back(new pair<int, bool>((*parent).first, true)); //true means eliminate
					active_parents.erase((*parent).first);
					toRemove.insert((*parent).first);
					//cout << "Eliminate " << (*parent).first << endl;
				}
			}

			for(set<int>::iterator parent = toRemove.begin(); parent != toRemove.end(); parent++){
				delete children[*parent];
				children.erase(*parent);
			}

		}


	}

	return variable_elimination_order;

}

std::list<std::set<int>*>* dbn::get_abstraction_order(){
	if(abstraction_order.size() == 0){
		//compute abstraction order



		//for each node get each parent and add node as a child
		map<int, set<int>*> parent_children_map;
		for(map<int, dbn_node*>::iterator node = vars.begin(); node != vars.end(); node++){
			//            std::cout << "node = " << (*node).first << flush;

			//for each parent of node
			// get child set of parent
			// add node to child set
			//      for(int parent = 0; parent < num_alt_facts+num_aux_vars; parent++){
			bool has_parents = false;
			for(map<int, dbn_node*>::iterator parent = vars.begin(); parent != vars.end(); parent++){
				if(isParent((*node).second, (*parent).second)){
					has_parents = true;
					//cout << " parent = " << (*parent).first << " " << std::endl;
					//  cout << " " << (*parent).first << flush;
					std::set<int> *children = parent_children_map[(*parent).first];
					if(children == NULL){
						children = new set<int>();
						parent_children_map[(*parent).first] = children;
					}
					//std::cout << "child = " << (*node).first << std::endl;
					children->insert((*node).first);
				}
			}
			//      cout << endl;
			if(!has_parents){
				std::set<int> *children = parent_children_map[(*node).first]; //children without parents
				if(children == NULL){
					children = new set<int>();
					parent_children_map[(*node).first] = children;
				}
				children->insert((*node).first);
			}


		}

		//group parents that have the same children

		set<set<int>*> parent_groups;
		for(int i = 0; i < num_alt_facts+num_aux_vars; i++){
			int parent = (i < num_alt_facts ? 2*i+1 : i+num_alt_facts);

			std::set<int> *children = parent_children_map[parent];

			//      std::cout << "P: " << parent << " C: "  << std::flush;
			//       if(children){
			// 	for(std::set<int>::iterator child = children->begin();
			// 	    child != children->end(); child++){
			// 	  std::cout << *child  << " " << std::flush;
			// 	}
			//       }
			//       cout << endl;

			// std::cout << "adding " << parent << std::endl;

			//find parent sets with same children


			std::set<std::set<int>*> intersecting_parent_groups;

			if(children != NULL && children->size() > 0){

				for(std::set<std::set<int>*>::iterator j =  parent_groups.begin();
						j != parent_groups.end(); j++){

					for(std::set<int>::iterator k = (*j)->begin();
							k != (*j)->end(); k++){


						std::set<int>* k_children = parent_children_map[*k];

						if(k_children == NULL)
							continue;

						// std::cout << "HI" << std::endl;
						std::set<int> inter;
						set_intersection(k_children->begin(),
								k_children->end(),
								children->begin(),
								children->end(),
								std::insert_iterator<std::set<int> >(inter, inter.begin()));


						if(inter.size() > 0){
							//std::cout << "inter" << std::endl;
							intersecting_parent_groups.insert(*j);
							break;
						}
					}
				}
			}
			//       for(std::set<std::set<int>*>::iterator k = intersecting_parent_groups.begin();
			//  	  k != intersecting_parent_groups.end(); k++){
			//  	for(std::set<int>::iterator j = (*k)->begin(); j != (*k)->end(); j++){
			//  	  std::cout << *j << " " << std::flush;
			//  	}
			//  	std::cout << std::endl;
			//       }


			if(intersecting_parent_groups.size() == 0){
				//create a new parent group with i
				set<int>* p = new set<int>();
				p->insert(parent);
				parent_groups.insert(p);
			}
			else{ //combine the groups and add i
				set<int> *p = new set<int>();
				p->insert(parent);
				for(set<set<int>*>::iterator j =  intersecting_parent_groups.begin();
						j != intersecting_parent_groups.end(); j++){
					//  cout << "adding1 " << (*j)->size() << endl;
					p->insert((*j)->begin(), (*j)->end());
					parent_groups.erase(*j);
					delete *j;
				}
				//	cout << "done" << endl;
				intersecting_parent_groups.clear();
				parent_groups.insert(p);
			}

			//      for(std::set<std::set<int>*>::iterator i = parent_groups.begin();
			//  	  i != parent_groups.end(); i++){
			//  	for(std::set<int>::iterator j = (*i)->begin(); j != (*i)->end(); j++){
			//  	  std::cout << *j << " " << std::flush;
			//  	}
			//  	std::cout << std::endl;
			//        }

		}
		abstraction_order.insert(abstraction_order.end(),
				parent_groups.begin(), 
				parent_groups.end());


		for(int j = 0; j < num_alt_facts+num_aux_vars; j++){
			if(parent_children_map[j] != NULL)
				delete parent_children_map[j];
			//   if(parent_spouse_map[j] != NULL)
			// 	delete parent_spouse_map[j];
		}

	}

	//    for(std::list<std::set<int>*>::iterator i = abstraction_order.begin();
	//        i != abstraction_order.end(); i++){
	//      for(std::set<int>::iterator j = (*i)->begin(); j != (*i)->end(); j++){
	//        std::cout << *j << " " << std::flush;
	//      }
	//      std::cout << std::endl;
	//    }


	return &abstraction_order;
}


list<DdNode*>* dbn::get_abstraction_order_cubes(){
	if(abstraction_order_cubes.size() == 0){



		for(list<set<int>*>::iterator i = abstraction_order.begin();
				i != abstraction_order.end(); i++){

			std::list<DdNode*> ddvars;

			int k = 0;
			for (set<int>::iterator j = (*i)->begin(); j != (*i)->end(); j++){
				int var = (*j < 2*num_alt_facts ? *j-1 : *j);
				//std::cout << "HI " << var << std::endl;

				if(*j < 2*num_alt_facts){
					DdNode* d = Cudd_addIthVar(manager, var);
					Cudd_Ref(d);
					ddvars.push_back(d);
				}
				else{
					std::set<int> *aux_bits = vars[(*j)]->dd_bits;
					//aux_var_bits[(*j)-num_alt_facts];
					for(std::set<int>::iterator i = aux_bits->begin(); i != aux_bits->end(); i++){
						DdNode* d =  Cudd_addIthVar(manager, *i);
						Cudd_Ref(d);
						ddvars.push_back(d);
					}
				}
			}
			if(ddvars.size() > 0){
				DdNode** variables = new DdNode*[ddvars.size()];
				for(std::list<DdNode*>::iterator j = ddvars.begin(); j != ddvars.end(); j++){
					variables[k++] = *j;
				}


				DdNode* cube = Cudd_addComputeCube(manager, variables, NULL, ddvars.size());// (*i)->size());
				Cudd_Ref(cube);
				//printBDD(cube);
				ddvars.clear();
				delete [] variables;


				//printBDD(cube);
				abstraction_order_cubes.push_back(cube);
			}
			else{
				abstraction_order_cubes.push_back(Cudd_ReadOne(manager));
			}
		}
	}
	return &abstraction_order_cubes;
}

std::map<int, DdNode*>* dbn::get_var_cubes(){
	if(var_cubes == NULL){
		var_cubes = new map<int, DdNode*>();
		for (int j = 0; j < num_alt_facts+num_aux_vars; j++){
			std::list<DdNode*> ddvars;
			int var = (j < num_alt_facts ? 2*j : j+num_alt_facts);
			//std::cout << "HI " << var << std::endl;

			if(var < 2*num_alt_facts){
				DdNode* d = Cudd_addIthVar(manager, var);
				Cudd_Ref(d);
				ddvars.push_back(d);
			}
			else{
				std::set<int> *aux_bits = vars[var]->dd_bits;
				//aux_var_bits[(*j)-num_alt_facts];
				for(std::set<int>::iterator i = aux_bits->begin(); i != aux_bits->end(); i++){
					DdNode* d =  Cudd_addIthVar(manager, *i);
					Cudd_Ref(d);
					ddvars.push_back(d);
				}
			}

			if(ddvars.size() > 0){
				int k = 0;
				DdNode** variables = new DdNode*[ddvars.size()];
				for(std::list<DdNode*>::iterator j = ddvars.begin(); j != ddvars.end(); j++){
					variables[k++] = *j;
				}

				DdNode* cube = Cudd_addComputeCube(manager, variables, NULL, ddvars.size());// (*i)->size());
				Cudd_Ref(cube);
				//printBDD(cube);
				ddvars.clear();
				delete [] variables;


				(*var_cubes)[(var < 2*num_alt_facts ? var+1 : var)] = cube;

			}
		}
	}
	return var_cubes;
}

DdNode* dbn::get_reward_dd(){
	if(reward_dd == NULL){


		//    reward_dd = nodeToDD(reward_node, true);
		reward_dd = Cudd_ReadZero(manager);
		Cudd_Ref(reward_dd);

		if(reward_dd != Cudd_ReadZero(manager)){ ///HACK!!!!!
			Cudd_RecursiveDeref(manager, reward_dd);
			reward_dd = Cudd_addApply(manager, Cudd_addMinus, Cudd_ReadZero(manager), Cudd_ReadOne(manager));
			Cudd_Ref(reward_dd);
		}

		//    printBDD(reward_dd);
	}
	return reward_dd;
}





bool dbn::isNoop(dbn_node* node){
  if(node->cpt){
    DdNode* tmp = node->cpt;
    Cudd_Ref(tmp);
    node->make_noop();
    bool rVal = node->cpt == tmp;
    Cudd_RecursiveDeref(manager, node->cpt);
    node->cpt = tmp;
    Cudd_Ref(node->cpt);
    Cudd_RecursiveDeref(manager, tmp);
    return rVal;
  }
  else if(node->factored_cpts.size() == 1){
    DdNode* tmp = *node->factored_cpts.begin();
    Cudd_Ref(tmp);
    node->make_noop();
    bool rVal = node->cpt == tmp;
    Cudd_RecursiveDeref(manager, node->cpt);
    node->cpt = tmp;
    Cudd_Ref(node->cpt);
    Cudd_RecursiveDeref(manager, tmp);
    return rVal;
  }
  else
    return true;
}


int* dbn::get_varmap() {
	if(a_varmap == NULL){
		//std::cout << "make varmap" << std::endl;
		int num = (2*num_alt_facts)+max_num_aux_vars+2*rbpf_bits;
		//std::cout << "num: " << num << " " << max_num_aux_vars<< std::endl;
		a_varmap = new int[num];
		for(int i = 0; i < num_alt_facts; i++){
			//std::cout << (2*i+1) << std::endl;
			if(dbn::isNoop(vars[2*i+1])){
				//std::cout << "noop" << std::endl;
				a_varmap[2*i] = 2*i;
				a_varmap[2*i+1] = 2*i+1;
			}
			else{
				a_varmap[2*i] = 2*i+1;
				a_varmap[2*i+1] = 2*i;
			}
		}
		for(int i = 2*num_alt_facts; i < num; i++)
			a_varmap[i] = i;
	}
	return a_varmap;
}

dbn_node* conjoin_nodes(dbn_node* result, dbn_node* node){

  //  cout << "conjoin " << ((result==NULL) ? -2 : result->var) << " " <<  node->var << endl;
	if(!result || !result->cpt){
	  //cout << "Conjoin0 " << node->var << " " << ((2*(num_alt_facts-2))+1) << endl;
		//printBDD(node->cpt);	  
		return new dbn_node(node);
	}
	if(!node || !node->cpt){
	  //cout << "Conjoin01 " << result->var << " " << ((2*(num_alt_facts-2))+1) << endl;
		return result;
	}


	//cout << "Conjoin " << endl;
	//   printBDD(result->cpt);
	//   printBDD(node->cpt);


	DdNode *tmp, *ta, *m;
	bool rNoop= (result->var == -1 ? true : dbn::isNoop(result));
	bool nNoop = (node->var == -1 ? true : dbn::isNoop(node));
	if(rNoop == nNoop){
		//       DdNode * r = Cudd_addBddThreshold(manager, result->cpt, 1.0);
		//       Cudd_Ref(r);
		//       DdNode * n = Cudd_addBddThreshold(manager, node->cpt, 1.0);
		//       Cudd_Ref(n);

		//       DdNode* t = Cudd_bddAnd(manager, r, n);
		//       Cudd_Ref(t);

		//       DdNode* ta = Cudd_BddToAdd(manager, t);
		//       Cudd_Ref(ta);

		//      printBDD(t);
// 	  if(result->var == (2*(num_alt_facts-1))+1){
// 	    std::cout <<"REW" << std::endl;
// 	  }
	  //cout << "Conjoin1 " << result->var << " " << ((2*(num_alt_facts-2))+1) << endl;


	  if(1|| result->var == (2*(num_alt_facts-2))+1 ||
	     result->var == (2*(num_alt_facts-1))+1){
	    //cout << "conjoin " << ((result==NULL) ? -2 : result->var) << " " <<  node->var << endl;
	    //std::cout << "HI " << node->factored_cpts.size() << std::endl;
	    //printBDD(*node->factored_cpts.begin());
	    result->factored_cpts.insert(result->factored_cpts.begin(), node->factored_cpts.begin(), node->factored_cpts.end());
	    result->probabilistic_parents.insert(node->probabilistic_parents.begin(), node->probabilistic_parents.end());
	    tmp = Cudd_ReadOne(manager);
	  }
	  else{
	  
	    tmp = (//(result->var == -1 || result->var == (2*(num_alt_facts-2))+1) ?
		   //Cudd_addApply(manager, Cudd_addPlus, result->cpt, node->cpt) :
		   Cudd_addApply(manager, Cudd_addMaximum, result->cpt, node->cpt)
		   );
	  }
// 	  if(result->var == (2*(num_alt_facts-1))+1){
// 	    std::cout <<"REW" << std::endl;
// 	  }
		//       Cudd_Ref(m);

		//       Cudd_RecursiveDeref(manager, r);
		//       Cudd_RecursiveDeref(manager, n);
		//       Cudd_RecursiveDeref(manager, t);

		//       tmp = Cudd_addApply(manager, Cudd_addTimes, ta, m);
		Cudd_Ref(tmp);
		//       Cudd_RecursiveDeref(manager, ta);
		//       Cudd_RecursiveDeref(manager, m);

		//    DdNode* tmp1 = Cudd_addApply(manager, Cudd_addPlus, result->cpt, node->cpt);
		//    Cudd_Ref(tmp1);

		//        printBDD(tmp);
		//    printBDD(tmp1);
	}
	else if(rNoop) {
	  //	  cout << "Conjoin2 " << endl;
	  if(1 || result->var == (2*(num_alt_facts-2))+1 ||
	     result->var == (2*(num_alt_facts-1))+1){
	    //std::cout << "HI " << node->factored_cpts.size() << std::endl;
	    //printBDD(*node->factored_cpts.begin());
	    result->factored_cpts.insert(result->factored_cpts.begin(), node->factored_cpts.begin(), node->factored_cpts.end());
	    result->probabilistic_parents.insert(node->probabilistic_parents.begin(), node->probabilistic_parents.end());
	    tmp = Cudd_ReadOne(manager);
	  }
	  else{
	    tmp = node->cpt;
	  }
	  Cudd_Ref(tmp);

	}
	else if(nNoop){
	  //	  cout << "Conjoin3 " << endl;
	  if(1 || result->var == (2*(num_alt_facts-2))+1 ||
	     result->var == (2*(num_alt_facts-1))+1){
	    //std::cout << "HI " << node->factored_cpts.size() << std::endl;
	    // printBDD(*node->factored_cpts.begin());
	    result->factored_cpts.insert(result->factored_cpts.begin(), node->factored_cpts.begin(), node->factored_cpts.end());
	    result->probabilistic_parents.insert(node->probabilistic_parents.begin(), node->probabilistic_parents.end());
	    tmp = Cudd_ReadOne(manager);
	  }
	  else{
	    tmp = result->cpt;
	  }
	  Cudd_Ref(tmp);
	}

	Cudd_RecursiveDeref(manager, result->cpt);
	result->cpt = tmp;
	result->parents.insert(node->parents.begin(), node->parents.end());

// 	if(( result->var == (2*(num_alt_facts-2))+1)){
// 	  cout << "Conjoin " << endl;
// 	  printBDD(result->cpt);
// 	  printBDD(node->cpt);
// 	  printBDD(tmp);
// 	}

	//printBDD(result->cpt);


	//a a' | n | x | m | r
	//0 0  | 1 | 0 | 1 | 0
	//0 1  | 0 | 1 | 1 | 1
	//1 0  | 0 | 0 | 0 | 0
	//1 1  | 1 | 1 | 1 | 1

	//  result->parents.insert(node->parents.begin(), node->parents.end());

	return result;

}

dbn* dbn::conjoin_dbns(list<dbn*>* dbns,
		map<dbn*, map<int, int>*> *dbn_var_map){
  //cout << "Conjoin " << dbns->size() << endl;

	dbn *result = new dbn(num_alt_facts);

	//      for(std::map<int, dbn_node*>::iterator j = result->vars.begin(); j != result->vars.end(); j++){
	//        if((*j).second != NULL && (*j).second->cpt != NULL){
	//          Cudd_RecursiveDeref(manager, (*j).second->cpt);
	//          (*j).second->cpt = Cudd_ReadZero(manager);
	//          Cudd_Ref((*j).second->cpt);
	//        }
	//      }

	for(list<dbn*>::iterator i = dbns->begin();
			i != dbns->end(); i++){
	  //std::cout << "dbn " <<  (*i)->vars[65]->factored_cpts.size() << std::endl;		      

		for(std::map<int, dbn_node*>::iterator j = (*i)->vars.begin(); j != (*i)->vars.end(); j++){
			if((*j).second != NULL ){
				//   cout << "HI" <<endl;
				result->vars[(*j).first] = conjoin_nodes(result->vars[(*j).first], (*j).second);
				//   printBDD(result->vars[(*j).first]->cpt);
				result->vars[(*j).first]->effects.insert( result->vars[(*j).first]->effects.begin(),
						(*j).second->effects.begin(),
						(*j).second->effects.end());
			}
		}
		result->num_aux_vars += (*i)->num_aux_vars;
		DdNode * t = Cudd_addApply(manager, Cudd_addTimes, (*i)->not_all_goal_and_sink, result->not_all_goal_and_sink);
		Cudd_Ref(t);
		Cudd_RecursiveDeref(manager, result->not_all_goal_and_sink);
		result->not_all_goal_and_sink = t;
		Cudd_Ref(result->not_all_goal_and_sink);
		Cudd_RecursiveDeref(manager, t);

		if((*i)->all_goal_and_sink != Cudd_ReadZero(manager)){
		  if(result->all_goal_and_sink == Cudd_ReadZero(manager)){
		    Cudd_RecursiveDeref(manager, result->all_goal_and_sink);
		    result->all_goal_and_sink = (*i)->all_goal_and_sink;
		    Cudd_Ref(result->all_goal_and_sink);
		  }
		  else{
		    t = Cudd_addApply(manager, Cudd_addTimes,(*i)->all_goal_and_sink, result->all_goal_and_sink);
		    Cudd_Ref(t);
		    //      printBDD(t);
		    Cudd_RecursiveDeref(manager, result->all_goal_and_sink);
		    result->all_goal_and_sink = t;
		    Cudd_Ref(result->all_goal_and_sink);
		  }
		}
	}
	//  cout << "done" << endl;
	return result;
}



dbn* dbn::condition_dbn(DdNode* dd, dbn* mdbn){
	

  DdNode* condition_add = Cudd_BddToAdd(manager, dd);
  Cudd_Ref(condition_add);
  
  DdNode* ncondition = Cudd_Not(dd);
  Cudd_Ref(ncondition);
  
  DdNode* ncondition_add = Cudd_BddToAdd(manager, ncondition);
  Cudd_Ref(ncondition_add);
  
  set<int> new_parents;
  DdNode* support = Cudd_Support(manager, dd);
  Cudd_Ref(support);
  for(int i = 0; i < 2*num_alt_facts+max_num_aux_vars; i++){
    if(Cudd_bddIsVarEssential(manager, support, i, 1)){
      if(i < 2*num_alt_facts)
	new_parents.insert(i+1);
      else
	new_parents.insert(i);
    }
  }
  Cudd_RecursiveDeref(manager, support);
  
  
//   cout << "New Parents" << endl;
//   copy(new_parents.begin(), new_parents.end(), ostream_iterator<int>(cout, "\n"));

  dbn *result = new dbn(num_alt_facts);
  
  for(std::map<int, dbn_node*>::iterator j = mdbn->vars.begin(); j != mdbn->vars.end(); j++){
    DdNode* t = NULL;
    //    cout << "condition " << (*j).first << endl;
    //		printBDD((*j).second->cpt);
    
    if((*j).second->factored_cpts.size() > 0){
      //		  cout << "Condition" << endl;
      //		  printBDD(dd);
      //cout << "condition " << (*j).first << " " << (*j).second->factored_cpts.size() << endl;
      for(std::list<DdNode*>::iterator f = (*j).second->factored_cpts.begin(); f != (*j).second->factored_cpts.end(); f++){
	if((*j).first < 2*num_alt_facts && (*j).second->cpt != NULL ){
	  //make part not matching condition
	  DdNode* tmp = *f;
	  Cudd_Ref(tmp);
	  
	  //make part matching condition
	  DdNode* success_condition = Cudd_addApply(manager, Cudd_addTimes, condition_add, tmp);
	  Cudd_Ref(success_condition);
	  
	  //combine parts
	  t = success_condition;//Cudd_addApply(manager, Cudd_addPlus, fail_condition, success_condition);
	  Cudd_Ref(t);
	  
	  Cudd_RecursiveDeref(manager, tmp);
	  Cudd_RecursiveDeref(manager, success_condition);		  
	}
	else if((*j).second->cpt != NULL){      
	  t = *f;
	  Cudd_Ref(t);
	  if(result->vars[(*j).first] == NULL){
	    result->vars[(*j).first] = new dbn_node((*j).second);
	  }
	}
	if(t != NULL){
	  //printBDD(t);
	  result->vars[(*j).first]->factored_cpts.push_back(t);
	  result->vars[(*j).first]->cpt = Cudd_ReadOne(manager);
	  Cudd_Ref(result->vars[(*j).first]->cpt);
	  map<DdNode*, int>::iterator p = (*j).second->probabilistic_parents.find(*f);
	  if(p != (*j).second->probabilistic_parents.end())
	    result->vars[(*j).first]->probabilistic_parents[t] = (*p).second;
	  if(t != Cudd_ReadZero(manager) && (*j).first < 2*num_alt_facts){
	      result->vars[(*j).first]->parents.insert(new_parents.begin(), new_parents.end());	  
	      result->vars[(*j).first]->parents.insert((*j).second->parents.begin(), (*j).second->parents.end());
	      
	    }
	    //cout << "condition " << (*j).first << " " << result->vars[(*j).first]->factored_cpts.size() << endl;		      
	}
      }
      for(list<DdNode*>::iterator e = (*j).second->effects.begin(); e != (*j).second->effects.end(); e++){
	DdNode *ef = Cudd_addApply(manager, Cudd_addTimes, condition_add, *e);
	Cudd_Ref(ef);
	result->vars[(*j).first]->effects.push_back(ef);
      }
    }
    else{
      if((*j).first < 2*num_alt_facts && (*j).second->cpt != NULL ){
	//make part not matching condition
	DdNode* tmp = (*j).second->cpt;
	Cudd_Ref(tmp);
	
	//make part matching condition
	DdNode* success_condition = Cudd_addApply(manager, Cudd_addTimes, condition_add, tmp);
	Cudd_Ref(success_condition);
	
	//combine parts
	t = success_condition;//Cudd_addApply(manager, Cudd_addPlus, fail_condition, success_condition);
	Cudd_Ref(t);
	
	Cudd_RecursiveDeref(manager, tmp);
	Cudd_RecursiveDeref(manager, success_condition);
		  
	for(list<DdNode*>::iterator e = (*j).second->effects.begin(); e != (*j).second->effects.end(); e++){
	  DdNode *ef = Cudd_addApply(manager, Cudd_addTimes, condition_add, *e);
	  Cudd_Ref(ef);
	  result->vars[(*j).first]->effects.push_back(ef);
	}
      }
      else if((*j).second->cpt != NULL){

	t = (*j).second->cpt;
	Cudd_Ref(t);
	
	if(result->vars[(*j).first] == NULL){
	  result->vars[(*j).first] = new dbn_node((*j).second);
	}
	
	//result[(*j).first]->dd_bits = new set<int>();
      }
      if(t != NULL){
	//if((*j).first ==-1)
	//printBDD(t);
	if(result->vars[(*j).first]->cpt!=NULL){
	  Cudd_RecursiveDeref(manager, result->vars[(*j).first]->cpt);
	}
	result->vars[(*j).first]->cpt = t;
	Cudd_Ref(result->vars[(*j).first]->cpt);
	Cudd_RecursiveDeref(manager, t);

	if(t != Cudd_ReadZero(manager) && (*j).first < 2*num_alt_facts){
	     result->vars[(*j).first]->parents.insert(new_parents.begin(), new_parents.end());
result->vars[(*j).first]->parents.insert((*j).second->parents.begin(), (*j).second->parents.end());	  
	   }
	   //cout << "condition " << (*j).first << endl;
	   // printBDD(t);
      }
    }
    
  
  }

  
  result->num_aux_vars = mdbn->num_aux_vars;

  result->not_all_goal_and_sink = mdbn->not_all_goal_and_sink;
  Cudd_Ref(result->not_all_goal_and_sink);
  
  result->all_goal_and_sink = mdbn->all_goal_and_sink;
  Cudd_Ref(result->all_goal_and_sink);
  
  //cout << "done" <<endl;
  return result;
  

}


map<int, DdNode*>* dbn::gen_outcome_dds(set<int>* dd_bits, map<int, double>* prs, int start_index){

	//cout << "gen_outcome_dds " << prs->size()  << " start: " << start_index << endl;

	//  aux_var_bits[var] = dd_bits;

	//std::map<int, DdNode*> *aux_var_value_dd = new std::map<int, DdNode*>();
	// std::cout << *this <<std::endl;

	map<int, DdNode*> *result = new map<int, DdNode*>();
	int num_values = prs->size();
	int num_bits = (int)ceil(log2((double)num_values));

	DdNode* remainder = Cudd_ReadOne(manager);
	Cudd_Ref(remainder);

	DdNode *varDD = Cudd_ReadZero(manager);
	Cudd_Ref(varDD);
	//for(int j = 0; j <= num_values; j++){
	int ji=0;
	for(map<int, double>::iterator j = prs->begin(); j != prs->end(); j++, ji++){
		int bitmask = 1;//0x80000000;
		DdNode* value = Cudd_ReadOne(manager);
		Cudd_Ref(value);

		for(int k=0; k < num_bits; k++)
		{
			int bitIndex = start_index+k;
			DdNode* bit = (((ji & bitmask) == 0) ?
					Cudd_Not(Cudd_bddIthVar(manager, bitIndex)) :
					Cudd_bddIthVar(manager, bitIndex));
			Cudd_Ref(bit);

			DdNode *t = Cudd_bddAnd(manager, value, bit);
			Cudd_Ref(t);
			Cudd_RecursiveDeref(manager, value);
			value = t;
			Cudd_Ref(value);
			Cudd_RecursiveDeref(manager, t);

			dd_bits->insert(bitIndex);

			bitmask <<= 1;
		}

		DdNode *r = Cudd_bddAnd(manager, remainder, Cudd_Not(value));
		Cudd_Ref(r);
		Cudd_RecursiveDeref(manager, remainder);
		remainder = r;
		Cudd_Ref(remainder);
		Cudd_RecursiveDeref(manager, r);
		//    printBDD(remainder);

		if(ji == prs->size()-1){
			//add remainder to last outcome
			r = Cudd_bddOr(manager, remainder, value);
			Cudd_Ref(r);
			Cudd_RecursiveDeref(manager, value);
			value = r;
			Cudd_Ref(value);
			Cudd_RecursiveDeref(manager, r);
		}


		DdNode *t = Cudd_BddToAdd(manager, value);
		Cudd_Ref(t);
		Cudd_RecursiveDeref(manager, value);
		value = t;
		Cudd_Ref(value);
		Cudd_RecursiveDeref(manager, t);

		(*result)[ji] = value;

	}
	Cudd_RecursiveDeref(manager, remainder);
	//  if(remainder != Cudd_ReadLogicZero(manager)){
	//     DdNode *t = Cudd_BddToAdd(manager, remainder);
	//     Cudd_Ref(t);
	//     Cudd_RecursiveDeref(manager, remainder);
	//     remainder = t;
	//     Cudd_Ref(remainder);
	//     Cudd_RecursiveDeref(manager, t);

	//     //(*result)[-1] = remainder;
	//   }

	return result;
}

void dbn::generate_probabilistic_nodes(const pEffect *effect, map<const pEffect*, dbn_node*> *result, int* start_index){

	const AssignmentEffect* fe = dynamic_cast<const AssignmentEffect*>(effect);
	if (fe != NULL) {
		//reward effect that needs to become probabilistic effect

		const Application& application = fe->assignment().application();

		ValueMap values;
		values[&application] = 0.0;
		fe->assignment().affect(values);

		Rational v = (*values.begin()).second;
		size_t n = 2;
		std::map<int, double> prs;
		prs[0] = transform_reward_to_probability(v.double_value());
		prs[1] = (1-gDiscount)-prs[0];
		prs[2] = 1.0-(prs[0]+prs[1]);

		int node_index = 2*num_alt_facts+result->size();

		std::set<int>* dd_bits = new std::set<int>();
		map<int, DdNode*>* outcome_dds = dbn::gen_outcome_dds(dd_bits, &prs,*start_index);
		(*start_index) += (int)ceil(log2((double)outcome_dds->size()));
		dbn_node* node = new dbn_node(node_index);
		node->outcomes.insert(outcome_dds->begin(), outcome_dds->end());
		node->prs.insert(prs.begin(), prs.end());
		delete outcome_dds;
		node->dd_bits = dd_bits;
		(*result)[effect] = node;
		//     cout << "Made node " << node_index << endl;
	}

	const SimpleEffect* se = dynamic_cast<const SimpleEffect*>(effect);
	if (se != NULL) {
	}

	const ConjunctiveEffect* ce =
			dynamic_cast<const ConjunctiveEffect*>(effect);
	if (ce != NULL) {
		// std::cout << "HI CJE" << std::endl;
		size_t n = ce->size();
		std::list<dbn*> conjuncts;
		  // std::cout << n << " conjuncts"<<std::endl;
		if (n > 0) {
			for (size_t i = 0; i < n; i++) {
				generate_probabilistic_nodes(&(ce->conjunct(i)), result, start_index);
			}
		}
	}

	const ConditionalEffect* we =
			dynamic_cast<const ConditionalEffect*>(effect);
	if (we != NULL) {
		//    std::cout << "HI CE"<<std::endl;
		generate_probabilistic_nodes(&(we->effect()), result, start_index);
	}

	const ProbabilisticEffect* pe =
			dynamic_cast<const ProbabilisticEffect*>(effect);
	if (pe != NULL) {


		/*
		 * Add the outcomes of this probabilistic effect.
		 */
		//std::cout << "HI PE"<<std::endl;
		Rational p_rest = 1;
		size_t n = pe->size();

		if(n == 1 && pe->probability(0) == 1.0){
			generate_probabilistic_nodes(&(pe->effect(0)), result, start_index);
		}
		else{
			std::map<int, double> prs;
			for (size_t i = 0; i < n; i++) {
				generate_probabilistic_nodes(&(pe->effect(i)), result, start_index);
				Rational p = pe->probability(i);
				p_rest = p_rest-p;
				prs[i] = p.double_value();
			}
			if(p_rest > 0.0){
				prs[n] = p_rest.double_value();
			}
			int node_index = 2*num_alt_facts+result->size();

			std::set<int>* dd_bits = new std::set<int>();
			map<int, DdNode*>* outcome_dds = dbn::gen_outcome_dds(dd_bits, &prs,*start_index);
			(*start_index) += (int)ceil(log2((double)outcome_dds->size()));
			dbn_node* node = new dbn_node(node_index);
			node->outcomes.insert(outcome_dds->begin(), outcome_dds->end());
			node->prs.insert(prs.begin(), prs.end());
			delete outcome_dds;
			node->dd_bits = dd_bits;
			(*result)[effect] = node;
			//     cout << "Made node " << node_index << endl;
		}
	}


}

map<const pEffect*, dbn_node*>* dbn::generate_probabilistic_nodes(const pEffect *effect){
	map<const pEffect*, dbn_node*>* result = new map<const pEffect*, dbn_node*>();
	int start_index = 2*num_alt_facts;
	generate_probabilistic_nodes(effect, result, &start_index);
	//std::cout << "Effect Aux = " << (start_index - (2*num_alt_facts)) << std::endl;
	return result;
}

dbn* dbn::probabilistic_dbns(std::list<double>* prs, std::list<dbn*>* dbns,
		dbn_node* pr_node){

  //     cout << "probabilistic " << pr_node->var << endl;


  //map each of dbns to an outcome of the pr_node
  dbn* result = new dbn(num_alt_facts);
  result->num_aux_vars++;


  map<int, DdNode*>::iterator outcome =//  = pr_node->outcomes->find(-1);
    //   DdNode* remainder= NULL;
    //   if(outcome != pr_node->outcomes->end()){
    //     remainder = (*outcome).second;
    //     pr_node->outcomes->erase(outcome);
    //   }
    //   outcome =
    pr_node->outcomes.begin();
  
  for(std::map<int, dbn_node*>::iterator j = result->vars.begin(); j != result->vars.end(); j++){
    if((*j).second != NULL){
      //Cudd_RecursiveDeref(manager, (*j).second->cpt);
      //       (*j).second->cpt = NULL;
      (*j).second->cpt = Cudd_ReadZero(manager);
      Cudd_Ref((*j).second->cpt);
    }
  }




  list<dbn*>::iterator dbn = dbns->begin();
  list<double>::iterator pr = prs->begin();
  result->vars[pr_node->var] = pr_node;
  DdNode* cpt = Cudd_ReadZero(manager);
  Cudd_Ref(cpt);
  for(;dbn != dbns->end(); dbn++, outcome++, pr++){
    //    std::cout << "["  << std::flush;
    //Cudd_CheckKeys(manager); std::cout << " | " << std::flush;
    for(std::map<int, dbn_node*>::iterator j = (*dbn)->vars.begin(); j != (*dbn)->vars.end(); j++){
      
      if((*j).second != NULL && (*j).second->cpt != NULL){
	if( (*j).first >= 2*num_alt_facts && result->vars[(*j).first] == NULL){
	  result->vars[(*j).first] = new dbn_node((*j).first);
	  result->vars[(*j).first]->dd_bits = (*j).second->dd_bits;
	  result->vars[(*j).first]->cpt = Cudd_ReadZero(manager);
	  Cudd_Ref(result->vars[(*j).first]->cpt);
	}
	
	result->vars[(*j).first]->parents.insert(pr_node->var);
	//	cout << (*j).first << " " << pr_node->var << endl;
	if((*j).second->factored_cpts.size() > 0){
	  for(std::list<DdNode*>::iterator f = (*j).second->factored_cpts.begin(); f!=(*j).second->factored_cpts.end(); f++){
	    DdNode *t = Cudd_addApply(manager, Cudd_addTimes, (*outcome).second, (*f));
	    Cudd_Ref(t);
	    result->vars[(*j).first]->factored_cpts.push_back(t);
	    result->vars[(*j).first]->probabilistic_parents[t] = pr_node->var;
	  }
	}
	else{
	  

	  //	cout << (*j).first << endl;
	  //printBDD((*outcome).second);
	  //	printBDD((*j).second->cpt);
	  DdNode *t = Cudd_addApply(manager, Cudd_addTimes, (*outcome).second, (*j).second->cpt);
	  Cudd_Ref(t);
	  
	  //printBDD(result->vars[(*j).first]->cpt);
	  DdNode *t1 = Cudd_addApply(manager, Cudd_addMaximum, t, result->vars[(*j).first]->cpt);
	  Cudd_Ref(t1);
	  //	printBDD(t1);
	  
	  Cudd_RecursiveDeref(manager, result->vars[(*j).first]->cpt);
	  result->vars[(*j).first]->cpt = t1;
	  Cudd_Ref(result->vars[(*j).first]->cpt);
	  Cudd_RecursiveDeref(manager, t1);

				
	  //	result->vars[(*j).first]->parents.insert(pr_node->var);
	  Cudd_RecursiveDeref(manager, t);
	  //cout << "CPT " << (*j).first << endl;
	  //printBDD(result->vars[(*j).first]->cpt);
	}
				
	for(list<DdNode*>::iterator e = (*j).second->effects.begin(); e != (*j).second->effects.end(); e++){
	  DdNode *ef = Cudd_addApply(manager, Cudd_addTimes, (*outcome).second, *e);
	  Cudd_Ref(ef);
	  result->vars[(*j).first]->effects.push_back(ef);
	}
	

      }

    }
    //  Cudd_CheckKeys(manager); std::cout << " ] " << std::endl;
    list<double>::iterator pr2 = pr; pr2++;
    double opr = (pr2 == prs->end() ? *pr/(pow(2, pr_node->dd_bits->size())-prs->size()+1) : *pr);
    //cout << "opr: " << opr << " " << ((pow(2, pr_node->dd_bits->size())-prs->size()+1)) << endl;

    DdNode *t3 = Cudd_addConst(manager, opr);
    Cudd_Ref(t3);
    DdNode *pr_outcome = Cudd_addApply(manager, Cudd_addTimes, (*outcome).second, t3);
    Cudd_Ref(pr_outcome);
    //    printBDD(pr_outcome);
    

    DdNode *t2 = Cudd_addApply(manager, Cudd_addPlus, cpt, pr_outcome);
    Cudd_Ref(t2);
    Cudd_RecursiveDeref(manager, cpt);
    cpt = t2;
    Cudd_Ref(cpt);
    Cudd_RecursiveDeref(manager, t2);
    Cudd_RecursiveDeref(manager, t3);
    Cudd_RecursiveDeref(manager, pr_outcome);
    
    result->num_aux_vars += (*dbn)->num_aux_vars;
    
    DdNode * t = Cudd_addApply(manager, Cudd_addTimes, (*dbn)->not_all_goal_and_sink, result->not_all_goal_and_sink);
    Cudd_Ref(t);
    Cudd_RecursiveDeref(manager, result->not_all_goal_and_sink);
    result->not_all_goal_and_sink = t;
    Cudd_Ref(result->not_all_goal_and_sink);
    Cudd_RecursiveDeref(manager, t);
		
    t = Cudd_addApply(manager, Cudd_addTimes,(*dbn)->all_goal_and_sink, result->all_goal_and_sink);
    Cudd_Ref(t);
    Cudd_RecursiveDeref(manager, result->all_goal_and_sink);
    result->all_goal_and_sink = t;
    Cudd_Ref(result->all_goal_and_sink);
  }
	//  if(outcome != pr_node->outcomes->end()){
	//     DdNode *t2 = Cudd_addApply(manager, Cudd_addPlus, cpt, (*outcome).second);
	//     Cudd_Ref(t2);
	//     Cudd_RecursiveDeref(manager, cpt);
	//     cpt = t2;
	//     Cudd_Ref(cpt);
	//     Cudd_RecursiveDeref(manager, t2);
	//   }
	pr_node->cpt = cpt;
	//	pr_node->factored_cpts.push_back(cpt);
	//  cout << "done pr " << endl;

	return result;

} 

dbn* dbn::simple_effect(dbn* mdbn, int var, int value){
	// dbn* ndbn = new dbn(mdbn);
	//delete mdbn;

	//  std::cout << "SE " <<(2*var+1) << " " << value << std::endl;

	dbn_node* n = ///new dbn_node(2*var+1);
			mdbn->vars[2*var+1];//= n;


	DdNode *v = Cudd_bddIthVar(manager, 2*var+1);
	Cudd_Ref(v);
	DdNode* b;
	if(n->cpt){
		Cudd_RecursiveDeref(manager, n->cpt);
	}
	if(value == 1){
		b = v;
	}
	else if(value == 0){
		b = Cudd_Not(v);
	}
	Cudd_Ref(b);
	n->cpt = Cudd_BddToAdd(manager, b);
	Cudd_Ref(n->cpt);
	n->effects.push_back(n->cpt);
	Cudd_Ref(n->cpt);
	n->factored_cpts.push_back(n->cpt);
	

	Cudd_RecursiveDeref(manager,v);
	Cudd_RecursiveDeref(manager,b);


	//  std::cout << "Made SE DBN: " <<std::endl;// << *ndbn  << std::endl;
	//printBDD(n->cpt);

	return mdbn;
}

dbn* dbn::reward(dbn *mdbn, ValueMap *r,
		dbn_node* pr_node){
	//       cout << "Reward DBN" << endl;
	//  dbn* ndbn = new dbn(mdbn);
	// delete mdbn;


	dbn* result = new dbn(num_alt_facts);
	result->num_aux_vars++;


	DdNode* not_goal_or_sink = Cudd_bddAnd(manager,
			Cudd_Not(Cudd_bddIthVar(manager, 2*(num_alt_facts-2))),
			Cudd_Not(Cudd_bddIthVar(manager, 2*(num_alt_facts-1))));
	Cudd_Ref(not_goal_or_sink);
	DdNode* not_goal_or_sink_add = Cudd_BddToAdd(manager, not_goal_or_sink);
	Cudd_Ref(not_goal_or_sink_add);

	map<int, DdNode*> *outcomes = &(pr_node->outcomes);
	map<int, double> *prs = (&pr_node->prs);

	result->vars[pr_node->var] = pr_node;
	//  map<int, double>::iterator pr = pr_node->prs->begin();
	//  printBDD(pr_node->cpt);
	DdNode* pr_cpt = Cudd_ReadZero(manager);
	Cudd_Ref(pr_cpt);

	dbn_node* rw = result->vars[-1];
	rw->cpt = Cudd_addConst(manager, transform_probability_to_reward((*prs)[0]));
	Cudd_Ref(rw->cpt);
	//  printBDD(rw->cpt);

	for(int i = 0; i < 3; i++){

		DdNode* tmp1 = Cudd_addConst(manager, (i!=2 ? (*prs)[i] : (*prs)[i]/2));
		Cudd_Ref(tmp1);
		DdNode* tmp2 = Cudd_addApply(manager, Cudd_addTimes, tmp1, (*outcomes)[i]);
		Cudd_Ref(tmp2);

		DdNode* tmp = Cudd_addApply(manager, Cudd_addPlus, pr_cpt, tmp2);
		Cudd_Ref(tmp);
		//    printBDD(tmp);
		Cudd_RecursiveDeref(manager, pr_cpt);
		pr_cpt = tmp;
		Cudd_Ref(pr_cpt);
		Cudd_RecursiveDeref(manager, tmp);
		Cudd_RecursiveDeref(manager, tmp1);
		Cudd_RecursiveDeref(manager, tmp2);

		if(i==0){//make goal node
			dbn_node* n = result->vars[2*(num_alt_facts-2)+1];
			DdNode *goal_state = Cudd_bddAnd(manager, Cudd_bddIthVar(manager, 2*(num_alt_facts-2)+1), not_goal_or_sink);
			Cudd_Ref(goal_state);
			DdNode *goal_state_add = Cudd_BddToAdd(manager, goal_state);
			Cudd_Ref(goal_state_add);
			n->cpt = Cudd_addApply(manager, Cudd_addTimes, (*outcomes)[i], goal_state_add);
			Cudd_Ref(n->cpt);
			//  printBDD(n->cpt);

			Cudd_RecursiveDeref(manager, goal_state);
			Cudd_RecursiveDeref(manager, goal_state_add);

			DdNode *t = Cudd_addApply(manager, Cudd_addPlus,(*outcomes)[i], result->all_goal_and_sink);
			Cudd_Ref(t);
			Cudd_RecursiveDeref(manager, result->all_goal_and_sink);
			result->all_goal_and_sink = t;
			Cudd_Ref(result->all_goal_and_sink);
			n->factored_cpts.push_back(n->cpt);
			n->effects.push_back(n->cpt);
			n->probabilistic_parents[n->cpt]=pr_node->var;
			n->parents.insert(pr_node->var);
		}
		else if(i==1){//make sink node
			dbn_node* n = result->vars[2*(num_alt_facts-1)+1];
			DdNode *sink_state = Cudd_bddAnd(manager, Cudd_bddIthVar(manager, 2*(num_alt_facts-1)+1), not_goal_or_sink);
			Cudd_Ref(sink_state);
			DdNode *sink_state_add = Cudd_BddToAdd(manager, sink_state);
			Cudd_Ref(sink_state_add);
			n->cpt = Cudd_addApply(manager, Cudd_addTimes, (*outcomes)[i], sink_state_add);
			Cudd_Ref(n->cpt);

			//printBDD(n->cpt);

			Cudd_RecursiveDeref(manager, sink_state);
			Cudd_RecursiveDeref(manager, sink_state_add);

			DdNode *t = Cudd_addApply(manager, Cudd_addPlus,(*outcomes)[i], result->all_goal_and_sink);
			Cudd_Ref(t);
			Cudd_RecursiveDeref(manager, result->all_goal_and_sink);
			result->all_goal_and_sink = t;
			Cudd_Ref(result->all_goal_and_sink);
			n->factored_cpts.push_back(n->cpt);
			n->effects.push_back(n->cpt);
			n->probabilistic_parents[n->cpt]=pr_node->var;
			n->parents.insert(pr_node->var);
		}
		else {//make all other nodes
		  DdNode *t = Cudd_addApply(manager, Cudd_addTimes,(*outcomes)[i], not_goal_or_sink_add);
		  Cudd_Ref(t);
		  DdNode *t1 = Cudd_addApply(manager, Cudd_addTimes, result->not_all_goal_and_sink, t);
		  Cudd_Ref(t1);
		  Cudd_RecursiveDeref(manager, result->not_all_goal_and_sink);
		  result->not_all_goal_and_sink = t1;
		  Cudd_Ref(result->not_all_goal_and_sink);
		  Cudd_RecursiveDeref(manager, t1);
		  Cudd_RecursiveDeref(manager, t);

			if(0){
				DdNode *t = Cudd_addApply(manager, Cudd_addTimes,(*outcomes)[i], not_goal_or_sink_add);
				Cudd_Ref(t);
				for(int j = 0; j <(num_alt_facts-2);j++){
					dbn_node* n = result->vars[(2*j)+1];
					if(n->cpt)
						Cudd_RecursiveDeref(manager, n->cpt);

					n->make_noop();

					DdNode* t1 = Cudd_addApply(manager, Cudd_addTimes, t, n->cpt);
					Cudd_Ref(t1);
					Cudd_RecursiveDeref(manager, n->cpt);
					n->cpt = t1;
					Cudd_Ref(n->cpt);

					Cudd_RecursiveDeref(manager, t1);
					//	printBDD(n->cpt);
				}

				Cudd_RecursiveDeref(manager, t);
			}
		}

	}
	//add to each node the rules:
	// - if act outcome and goal or sink true in current, then noop;
	// - if act outcome and not goal or sink in current, then nothing
	// - if goal outcome and goal false then make goal true, otherwise noop
	// - if sink outcome and sink false then make sink true, otherwise noop
	pr_node->cpt = pr_cpt;
	pr_node->goalOrSinkRelevant = true;
	Cudd_Ref(pr_node->cpt);
	Cudd_RecursiveDeref(manager, pr_cpt);

	// printBDD(pr_node->cpt);


	Cudd_RecursiveDeref(manager, not_goal_or_sink);
	Cudd_RecursiveDeref(manager, not_goal_or_sink_add);


	return result;
}

void dbn::apply_rewards_and_sink(){



	cout << "apply rew and sink" << endl;

	for(int i = 0 ; i < num_alt_facts-2; i++){
		cout << (2*i+1) <<endl;
		dbn_node *n = vars[2*i+1];
		DdNode *t = Cudd_addApply(manager, Cudd_addTimes, n->cpt, not_all_goal_and_sink);
		Cudd_Ref(t);
		Cudd_RecursiveDeref(manager, n->cpt);
		printBDD(t);
		n->make_noop();
		printBDD(all_goal_and_sink);
		DdNode *t1 = Cudd_addApply(manager, Cudd_addTimes, n->cpt, all_goal_and_sink);
		Cudd_Ref(t1);
		Cudd_RecursiveDeref(manager, n->cpt);
		printBDD(t1);
		n->cpt = Cudd_addApply(manager, Cudd_addPlus, t, t1);
		Cudd_Ref(n->cpt);
		Cudd_RecursiveDeref(manager, t);
		Cudd_RecursiveDeref(manager, t1);
		printBDD(n->cpt);
	}
}

ostream& operator<<(ostream& os, dbn_node& d){
	//  if(d.cpt.size() == 0){
	//    return os;
	//  }
	os << "Var: " << d.var  << std::endl;

	//os << "Parents: ";
	//  copy(d.parents.begin(), d.parents.end(),
	//  std::ostream_iterator<int>(os, " "));
	//  os << std::endl;
	//  os << "Children: ";
	//  copy(d.children.begin(), d.children.end(),
	//   std::ostream_iterator<int>(os, " "));
	//os << std::endl;
	//  os << "CPT:" << std::endl << d.cpt << std::endl;



}

std::ostream& operator<<(std::ostream& os, dbn& d){
	//  os << *(d.reward_node) << std::endl;
	for(int i = 0; i < (2*num_alt_facts+d.num_aux_vars); i++){
		//os << "i = " << i << endl;
		if(i < 2*num_alt_facts && i%2==0)
			continue;
		if(d.vars[i] != NULL && d.vars[i]->cpt != NULL){
			//os << *(d.vars[i]) << std::endl;

		}
	}
	os << "done print" << endl;
}

