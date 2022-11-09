#include "observations.h"
#include <typeinfo>
#include "problems.h"
#include "domains.h"

const Observation& Observation::instantiation(const SubstitutionMap& subst,
					      const Problem& problem) const {
  //  cout << "instan ob: " << observations_.size() <<endl;
  Observation* o = new Observation();
  ObservationVector &ob =(ObservationVector&)observations_;
  for(ObservationVector::iterator i = ob.begin();
      i != ob.end(); i++){
    o->add_entry(&(*i)->instantiation(subst, problem));
  }
  //cout << "HO"<<endl;
  return *o;
}

const ObservationEntry& 
ObservationEntry::instantiation(const SubstitutionMap& subst,
				const Problem& problem) const {
  //std::cout << "instan ob ent"<<std::endl;

  //       if (typeid(symbol_->instantiation(subst,problem)) == typeid(Constant)) {
  // std::cout << "symbol is cosnt"<<std::endl;
  //   }
  const ObservationEntry* o;
  if(symbol_){

    const Atom* af = dynamic_cast<const Atom*>(symbol_);
    if (af != NULL) {
      ((PredicateTable&)problem.domain().predicates()).make_dynamic(af->predicate());
      //std::cout <<af->predicate() << std::endl;
    }
    

    o = 
      new ObservationEntry(formula_->instantiation(subst,problem),
			   &symbol_->instantiation(subst,problem),
			   *new Rational(posProbability_.double_value()),
			   *new Rational(negProbability_.double_value()));
  }
  else if(obs_){
    o =     
      new ObservationEntry(formula_->instantiation(subst,problem),

			     *dynamic_cast<const ProbabilisticEffect*>(&obs_->instantiation(subst,problem))
			   );
  }
  else
    o = 
      new ObservationEntry(formula_->instantiation(subst,problem),
			   NULL,//symbol_->instantiation(subst,problem),
			   *new Rational(posProbability_.double_value()),
			   *new Rational(negProbability_.double_value()));
  ///cout << "HOOO"<<endl;					   
  
  return *o;
}
  /* Prints this action schema on the given stream. */
void ObservationCpt::print(std::ostream& os, const PredicateTable& predicates,
			   const FunctionTable& functions, const TermTable& terms) const{
  os << "CPT " << observations_.size() << std::endl;
  ObservationCptRowVector &ob =(ObservationCptRowVector&)observations_;
  for(ObservationCptRowVector::iterator i = ob.begin();
      i != ob.end(); i++){
  
      (*i)->print(os, predicates, functions, terms);
   
  }
}

const ObservationCpt& ObservationCpt::instantiation(const SubstitutionMap& subst,
						    const Problem& problem) const {
  ObservationCpt* o = new ObservationCpt();
  ObservationCptRowVector &ob =(ObservationCptRowVector&)observations_;
  for(ObservationCptRowVector::iterator i = ob.begin();
      i != ob.end(); i++){
    o->add_cpt_entry(&(*i)->instantiation(subst, problem));
  }
  //cout << "HO"<<endl;
  return *o;

}

  /* Prints this action schema on the given stream. */
void ObservationCptRow::print(std::ostream& os, const PredicateTable& predicates,
			      const FunctionTable& functions, const TermTable& terms) const{
  // os << "row" << std::endl;
 formula_->print(os, predicates, functions, terms);
 //
  effect_->print(os, predicates, functions, terms);
 os << std::endl;
}

const ObservationCptRow& ObservationCptRow::instantiation(const SubstitutionMap& subst,
						    const Problem& problem) const {

  ObservationCptRow *o = new ObservationCptRow(formula_->instantiation(subst,problem),
					       (const ProbabilisticEffect&)effect_->instantiation(subst,problem));

  ///cout << "HOOO"<<endl;					   
  
  return *o;
}
