#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "cudd.h"
#include "cudd/dd.h"

#include "labelledElement.h"

// can be replaced with le->label == label
// EquivDC(.,.,.,0) == "=="

//int num_lab_elts = 0;

LabelledElement::~LabelledElement(){
//num_lab_elts--;
//   Cudd_RecursiveDeref(manager, label);
}
int  LabelledElement::operator==(LabelledElement* le){
  return (le->elt == elt && le->label == label);
}
// int  LabelledElement::operator<(LabelledElement* q){
//     return std::op_less_than(this, q);
// }
LabelledElement::LabelledElement(){
//num_lab_elts++;
 elt = NULL; label = NULL;  cost = 0;
}
LabelledElement::LabelledElement(DdNode* l, double c){
//num_lab_elts++;
 elt = NULL; label = l;  cost = c;
   Cudd_Ref(label);   
   //Cudd_Ref(label);
}
LabelledElement::LabelledElement(LabelledElement *le){
//num_lab_elts++;
   elt = NULL; label = le->label;  cost = le->cost;
   Cudd_Ref(label);   
   //Cudd_Ref(label);
}


LabelledEffect::~LabelledEffect(){
//num_lab_elts--;
 //   Cudd_RecursiveDeref(manager, label);
}
int  LabelledEffect::operator==(LabelledEffect* le){
  return (le->elt->alt_index == elt->alt_index && le->label == label);  
}
// int  LabelledEffect::operator<(LabelledEffect* q){
//   return std::op_less_than(this, q);
// }
LabelledEffect::LabelledEffect(EfNode* e, DdNode* l, double c){
//num_lab_elts++;
   elt = e;  label = l;  cost = c;
   Cudd_Ref(label);   
   // Cudd_Ref(label);
}
LabelledEffect::LabelledEffect(LabelledEffect *le){
//num_lab_elts++;
  elt = le->elt;  label = le->label;  cost = le->cost;
   Cudd_Ref(label);
   //   Cudd_Ref(label);
}


LabelledAction::~LabelledAction(){
//num_lab_elts--;
//   Cudd_RecursiveDeref(manager, label);
  //cout << "del act"<<endl<<flush;
}
int  LabelledAction::operator==(LabelledAction* le){
  return (le->elt->alt_index == elt->alt_index && le->label == label);  
}
// int  LabelledAction::operator<(LabelledAction* q){
//   return std::op_less_than(this, q);
// }
LabelledAction::LabelledAction(OpNode* e, DdNode* l, double c){
//num_lab_elts++;
  elt = e;  label = l;  cost = c;
   Cudd_Ref(label);   
   //Cudd_Ref(label);
}
// LabelledAction::LabelledAction(alt_action* e, DdNode* l, double c){
//   act = e;  label = l;  cost = c;
//    Cudd_Ref(label);   
//    //Cudd_Ref(label);
// }
LabelledAction::LabelledAction(LabelledAction *le){
//num_lab_elts++;
  act = le->act; elt = le->elt;  label = le->label;  cost = le->cost;
   Cudd_Ref(label);   
   //Cudd_Ref(label);
}

// int  LabelledAltAction::~LabelledAltAction(){
//   Cudd_RecursiveDeref(manager, label);
// }
// int  LabelledAltAction::operator==(LabelledAltAction* le){
//   return (le->elt == elt && le->label == label);  
// }
// int  LabelledAltAction::operator<(LabelledAltAction* q){
//   return std::op_less_than(this, q);
// }
// LabelledAltAction::LabelledAltAction(alt_action* e, DdNode* l, double c){
//   elt = e;  label = l;  cost = c;
// }
// LabelledAction::LabelledAltAction(LabelledAction *le){
//   elt = le->elt;  label = le->label;  cost = le->cost;
// }

LabelledFormula::~LabelledFormula(){
//num_lab_elts--;
//   Cudd_RecursiveDeref(manager, label);
//   Cudd_RecursiveDeref(manager, elt);
}
int  LabelledFormula::operator==(LabelledFormula* le){
  return (le->elt == elt && le->label == label);  
}
// int  LabelledFormula::operator<(LabelledFormula* q){
//   return std::op_less_than(this, q);
// }
LabelledFormula::LabelledFormula(DdNode* e, DdNode* l, double c){
//num_lab_elts++;
  elt = e;  label = l;  cost = c;
  Cudd_Ref(elt);  Cudd_Ref(label);
  //Cudd_Ref(elt);  Cudd_Ref(label);
}
LabelledFormula::LabelledFormula(LabelledFormula *le){
//num_lab_elts++;
  elt = le->elt;  label = le->label;  cost = le->cost;
  Cudd_Ref(elt);  Cudd_Ref(label);
  // Cudd_Ref(elt);  Cudd_Ref(label);
}
