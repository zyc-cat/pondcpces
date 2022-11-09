#ifndef CORRELATION_H
#define CORRELATION_H

#include "ipp.h"
//#include <stl.h>
#include "math.h"
#include <ext/hash_map>
#include <algorithm>

struct DdNode;
void printBDD(DdNode*);

/* Options */
extern bool USE_CORRELATION;


/* Data Structures */
extern void printFact(int);

//pair-wise correlations for facts//
namespace __gnu_cxx{
template<> struct hash<pair<FtNode*, FtNode*>*>{
  size_t const operator()( pair<FtNode*, FtNode*>* p) const {
    //   cout << "key = " << (size_t)std::min(p->first->index, p->second->index) << endl;
    return (size_t)std::min(p->first->index, p->second->index);

  }
};
struct eqftmap
{
  bool operator()(const pair<FtNode*, FtNode*>* p, const pair<FtNode*, FtNode*>* q) const
  {
/*     cout << "cmp " */
/* 	 << p->first->positive << " " << p->first->index << " " */
/* 	 << p->second->positive << " " << p->second->index << " | " */
/* 	 << q->first->positive << " " << q->first->index << " " */
/* 	 << q->second->positive << " " << q->second->index */
/* 	 <<endl; */
      

    return  
/*       p->first == q->first &&  */
/*       p->second == q->second; */
      p->first->index == q->first->index &&
      p->first->positive == q->first->positive &&
      p->second->index == q->second->index &&
      p->second->positive == q->second->positive;
  }
};
/* struct leftmap */
/* { */
/*   bool operator()(const pair<FtNode*, FtNode*>* p, const pair<FtNode*, FtNode*>* q) const */
/*   { */
/*     int p1, p2, q1, q2; */
/*     p1 = p->first->index * (1+p->first->positive); */
/*     q1 = q->first->index * (1+q->first->positive); */
/*     p2 = p->second->index * (1+p->second->positive); */
/*     q2 = q->second->index * (1+q->second->positive); */

/*     return std::min(p1, p2) < std::min(q1, q2); */

/*   } */
/* }; */

struct FtCorrelationHash : public hash_map<pair<FtNode*, FtNode*>*, double, hash<pair<FtNode*, FtNode*>*>, eqftmap >{
};
//struct FtCorrelationHash : public map<pair<FtNode*, FtNode*>*, double, leftmap >{
//};

struct FtPairSupportersHash : public hash_map<std::pair<FtNode*, FtNode*>*, std::set<EfNode*>*, hash<std::pair<FtNode*, FtNode*>*>, eqftmap >{
};




//pair-wise correlations for effects and actions//
template<> struct hash< pair<EfNode*, EfNode*>*>{
  size_t const operator()( pair<EfNode*, EfNode*>* p) const {return (size_t)p->first->alt_index;}
};
struct eqaemap
{
  bool operator()(const pair<EfNode*, EfNode*>* p, const pair<EfNode*, EfNode*>* q) const
  {
/*     OpNode *o1p, *o2p, *o1q, *o2q; */
/*     EfNode *e1p, *e2p, *e1q, *e2q; */

/*     o1p = dynamic_cast<OpNode*>(p->first); */
/*     o2p = dynamic_cast<OpNode*>(p->second); */
/*     o1q = dynamic_cast<OpNode*>(q->first); */
/*     o2q = dynamic_cast<OpNode*>(q->second); */
    
/*     e1p = dynamic_cast<OpNode*>(p->first); */
/*     e2p = dynamic_cast<OpNode*>(p->second); */
/*     e1q = dynamic_cast<OpNode*>(q->first); */
/*     e2q = dynamic_cast<OpNode*>(q->second); */
 
    
/*     return  ((o1p == o1q && o1p && o1q) ||  */
/* 	     (e1p == e1q && e1p && e1q)) && */
/*       ((o2p == o2q && o2p && o2q) ||  */
/*        (e2p == e2q && e2p && e2q)) */
      return  p->first == q->first && p->second == q->second;
    
  }
};

struct AECorrelationHash : public hash_map<pair<EfNode*, EfNode*>*, double, hash<pair<EfNode*, EfNode*>*>, eqaemap >{
};
}

/* Functions */
void instantiateCorrelation();
void freeCorrelation();

void initialStateProbability(DdNode*, DdNode*);
void initialStateCorrelation(DdNode*,DdNode*);

void propositionProbability(int time);
void propositionCorrelation(int time);
double propositionConjunctionProbability(std::set<FtNode*>*, int time);
double correlation(FtNode*, FtNode*, int);
void greedySupportPropositions(FtNode*, FtNode*, int);
void maximalSubsetSupportPropositions(FtNode*, FtNode*, int);
void setPropositionCorrelation(FtNode*, FtNode*, int, double);

void actionProbability(int time);
void actionCorrelaton(int time);
double actionAndEffectSetProbability(std::set<OpNode*>* ops, 
				   std::map<EfNode*, std::set<int>* >* efs, 
				   int time);

void actionAndEffectProbability(int time);
void actionAndEffectCorrelation(int time);
void setActionCorrelation(OpNode*, OpNode*, int, double);
void setActionEffectCorrelation(OpNode*, EfNode*, int, double);

double correlation(OpNode*, OpNode*, int);
double correlation(OpNode*, EfNode*, int);

void effectProbability(int time);
void effectCorrelaton(int time);
void setEffectCorrelation(EfNode*, EfNode*, int, double);
double correlation(EfNode*, EfNode*, int);

double goalProbability(int time);
double relaxedPlanHeuristic();

bool stopCORRGraph(int time);


#endif
