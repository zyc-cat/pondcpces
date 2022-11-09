#ifndef LABELLED_ELEMENT_H
#define LABELLED_ELEMENT_H

//#include "cudd/cudd.h"
//#include "cudd/cuddObj.hh"
//#include "nusmv/dd/dd.h"

//#include "../states/clausalState.h"

class LabelledElement;
class alt_action;
struct DdManager;
struct DdNode;

extern DdManager* manager;
extern int op_less_than(LabelledElement*, LabelledElement*);

#include "ipp.h"

class LabelledEffect;
class LabelledAction;
class LabelledFormula;

class LabelledElement{

 public:
  void* elt;
  DdNode* label;// 到达该Element的belief state BDD
  double cost;

  virtual ~LabelledElement();
  LabelledElement();
  LabelledElement(DdNode* l, double c);
  LabelledElement(LabelledElement* le);
  virtual int operator==(LabelledElement* le);
  // virtual int operator<(LabelledElement* q); 
};

class LabelledEffect : public LabelledElement{
 public:
   EfNode* elt; 
 
   virtual ~LabelledEffect();
   LabelledEffect(EfNode* e, DdNode* l, double c);
   LabelledEffect(LabelledEffect* le);
   virtual int operator==(LabelledEffect* le);
   //  virtual int operator<(LabelledEffect* q);
};
class LabelledAction : public LabelledElement{
 public:
   OpNode* elt;
   alt_action* act;
 
   virtual ~LabelledAction();
   LabelledAction(OpNode* e, DdNode* l, double c);
   //   LabelledAction(alt_action* e, DdNode* l, double c);
  LabelledAction(LabelledAction* le);
   virtual int operator==(LabelledAction* le);
   // virtual int operator<(LabelledAction* q);
};
/* class LabelledAltAction : public LabelledElement{ */
/*  public: */
/*    OpNode* elt;  */
 
/*    virtual ~LabelledAltAction(); */
/*    LabelledAltAction(alt_action* e, DdNode* l, double c); */
/*    LabelledAltAction(LabelledAltAction* le); */
/*    virtual int operator==(LabelledAltAction* le); */
/*    virtual int operator<(LabelledAltAction* q); */
/* }; */
class LabelledFormula : public LabelledElement{
 public:
   DdNode* elt; 
 
   virtual ~LabelledFormula();
   LabelledFormula(DdNode* e, DdNode* l, double c);
   LabelledFormula(LabelledFormula* le);
   virtual int operator==(LabelledFormula* le);
   // virtual int operator<(LabelledFormula* q);
};
#endif
