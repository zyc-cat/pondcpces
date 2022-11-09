#include "correlation.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "ipp.h"
#include "dd.h"
#include "globals.h"
#include "math.h"
#include "memory.h"
#include <float.h>
#include "exclusions.h"

extern int graph_levels;
extern void printFact(int);

using namespace std;
using namespace __gnu_cxx;

/* Options */
bool USE_CORRELATION;// = //true;//false;
bool greedyAlgo = false;// true;

/* Data Structures */
set<FtNode*> goals;
map<int, FtCorrelationHash* > propositionCorrelations;
map<int, AECorrelationHash* > actionEffectCorrelations;
map<int, FtPairSupportersHash* > propPairSupport;

double error = 0.00;

void instantiateCorrelation(){


}
void freeCorrelation(){
  // cout << "FREE"<< endl;
  goals.clear();

     for(int i = 0; i <= graph_levels; i++){
       // cout << "i = " << i << endl;
       map<int, FtCorrelationHash* >::iterator k = propositionCorrelations.find(i);
       map<int, FtPairSupportersHash* >::iterator z = propPairSupport.find(i);

       for(FtNode *ft = gall_fts_pointer; ft; ft = ft->next){
	 for(FtNode *ft1 = ft->next; ft1 ; ft1 = ft1->next){
	   FtNode *fa, *fb;
	   if(ft->index < ft1->index){
	     fa = ft; fb = ft1;
	   }
	   else{
	     fa = ft1; fb = ft;
	   }
	   pair<FtNode*, FtNode*> p (fa, fb);

	   //cout << "HI" << endl; 

	   if(k != propositionCorrelations.end()){
	     //delete correlation
	     FtCorrelationHash::iterator p1;
	     p1 = (*k).second->find(&p);
	     if(p1 != (*k).second->end()){
	       (*k).second->erase(p1);
	       delete (*p1).first;
	     }
	   }
	   
	   if(z != propPairSupport.end()){
	     //delete supporters
	     FtPairSupportersHash::iterator p2;
	     p2 = (*z).second->find(&p);
	     if(p2 != (*z).second->end()){
	       (*z).second->erase(p2);
	       delete (*p2).first;
	       delete (*p2).second;
	     }
	   }
	 } 
       }

       if(k != propositionCorrelations.end()){
	 propositionCorrelations.erase(k);
	 delete (*k).second;
       }
       if(z != propPairSupport.end()){
	 propPairSupport.erase(z);
	 delete (*z).second;	     
       }


  //      //cout << "size = " << (*k).second->size() <<endl;
//        int n = 0;
//         for(FtCorrelationHash::iterator j = (*k).second->begin();
//  	   j != (*k).second->end(); j++){
// 	  cout << n++ << "/" << (*k).second->size() << endl;
// // 	 //	 if((*j).first)
// // 	 //	 delete (*j).first;
//         }
     
      }

     //   cout << "done"<<endl;
}


double marginal(DdNode *distribution_add,
		DdNode *distribution_bdd, 
		DdNode *b, 
		int num_conjuncts){
  //  cout << "marginal: " << num_conjuncts << endl;
  //  printBDD(b);


  DdNode** cube = new DdNode*[num_alt_facts-num_conjuncts];
  int k = 0;
  for(int i = 0; i < num_alt_facts; i++){ 
    DdNode *p =  Cudd_bddIthVar(manager, 2*i);
    Cudd_Ref(p);
    DdNode *n = Cudd_Not(Cudd_bddIthVar(manager, 2*i));
    Cudd_Ref(n);
    
    if(bdd_entailed(manager, b, p) ||
       bdd_entailed(manager, b, n)){
      Cudd_RecursiveDeref(manager, p);
      Cudd_RecursiveDeref(manager, n);
      continue;
    }

    cube[k] =  Cudd_addIthVar(manager,2*i);
    Cudd_Ref(cube[k++]);
    Cudd_RecursiveDeref(manager, p);
    Cudd_RecursiveDeref(manager, n);
  }
  DdNode *cubedd = Cudd_addComputeCube(manager, 
				       cube, NULL, 
				       num_alt_facts-num_conjuncts);
  Cudd_Ref(cubedd);


  for(int i = 0; i < num_alt_facts-num_conjuncts; i++){  
    Cudd_RecursiveDeref(manager, cube[i]);
  }
  delete [] cube;


  DdNode * tmp = Cudd_bddAnd(manager, b, distribution_bdd);
  Cudd_Ref(tmp);
  DdNode *tmp_add = Cudd_BddToAdd(manager, tmp);
  Cudd_Ref(tmp_add);
  DdNode* distribution = Cudd_addApply(manager, Cudd_addTimes,
				       tmp_add, distribution_add);
  Cudd_Ref(distribution);

  DdNode* fr = Cudd_addExistAbstract(manager, distribution, cubedd);
  Cudd_Ref(fr);

  Cudd_RecursiveDeref(manager, cubedd);  
  Cudd_RecursiveDeref(manager, distribution);  
  Cudd_RecursiveDeref(manager, tmp);
  Cudd_RecursiveDeref(manager, tmp_add);

  int* x = new int[num_alt_facts];
  CUDD_VALUE_TYPE value;
  DdGen* gen = Cudd_FirstCube(manager, fr, &x, &value);
  DdNode* tmp_cube = Cudd_CubeArrayToBdd(manager, x);
  //  Cudd_Ref(tmp_cube);
  Cudd_RecursiveDeref(manager, fr);  
  delete [] x;

  return value;
}


void initialStateProbability(DdNode* initADD, DdNode* initBDD){
  //  cout << "Initial state probability" << endl;
  // printBDD(initBDD);
  //  printBDD(initADD);


  for(FtNode* ft = gall_fts_pointer; ft; ft = ft->next){
//     if(!ft->info_at[0])
//       continue;
    //cout << (ft->positive ? "pos " : "neg ");   printFact(ft->index); 

     DdNode *p = Cudd_bddIthVar(manager, ft->index*2);
    Cudd_Ref(p);
    DdNode *d = (ft->positive ?  p : Cudd_Not(p));
    Cudd_Ref(d);
    if(ft->info_at[0])
      //   printBDD(d);
    ft->info_at[0]->probability = marginal(initADD, initBDD, d, 1);
    Cudd_RecursiveDeref(manager, p);
    Cudd_RecursiveDeref(manager, d);

    //cout << " " <<  ft->info_at[0]->probability << endl;    
  }
  //cout << "done" <<endl;
}


void initialStateCorrelation(DdNode* initADD, DdNode* initBDD){
  //  cout << "Initial state correlation" << endl;


   for(FtNode* ft = gall_fts_pointer; ft; ft = ft->next){
     for(FtNode* ft1 = ft->next; ft1; ft1 = ft1->next){
       if(ft->index == ft1->index && 
	  ft->positive != ft1->positive)
	 continue;
       
 
       DdNode *p = Cudd_bddIthVar(manager, ft->index*2);
       Cudd_Ref(p);
       DdNode *d = (ft->positive ?  p : Cudd_Not(p));
       Cudd_Ref(d);

       DdNode *p1 = Cudd_bddIthVar(manager, ft1->index*2);
       Cudd_Ref(p1);
       DdNode *d1 = (ft1->positive ?  p1 : Cudd_Not(p1));
       Cudd_Ref(d1);

       DdNode *a = Cudd_bddAnd(manager, d, d1);
       Cudd_Ref(a);

       double t = marginal(initADD, initBDD, a, 2)/
	 (ft->info_at[0]->probability*ft1->info_at[0]->probability);

       setPropositionCorrelation(ft, ft1, 0, t);
 
       Cudd_RecursiveDeref(manager, a);
       Cudd_RecursiveDeref(manager, p);
       Cudd_RecursiveDeref(manager, d);
       Cudd_RecursiveDeref(manager, p1);
       Cudd_RecursiveDeref(manager, d1);

       //   cout << (ft->positive ? "pos " : "neg ");printFact(ft->index); 
       //cout << (ft1->positive ? "pos " : "neg ");printFact(ft1->index); 
       //cout << "correlation = " << t << endl;
//        cout << " " <<  (*propositionCorrelations[0])[mpair] << endl;
     }
   }

}

void propositionProbability(int time){
  //   cout << "prop prob: " << time <<endl;


  for(FtNode* ft = gall_fts_pointer; ft; ft = ft->next){
    if(greedyAlgo)
      greedySupportPropositions(ft, NULL, time);
    else{
      maximalSubsetSupportPropositions(ft,NULL, time);
    }


 //    cout << ft->info_at[time]->probability
// 	 << (ft->positive ? " 1 " : " 0 "); printFact(ft->index);
  }

  // cout << "Pr(Goals) = " << goalProbability(time) << endl;
}

void propositionCorrelation(int time){
  //    cout << "prop correlatin" << endl;
   for(FtNode* ft = gall_fts_pointer; ft; ft = ft->next){
     for(FtNode* ft1 = ft->next; ft1; ft1 = ft1->next){
       if(ft->index == ft1->index)
	 continue;

       if(MUTEX_SCHEME != MS_NONE &&
	  ARE_MUTEX_FTS(time, ft, ft1, NULL, NULL)){
	 setPropositionCorrelation(ft, ft1, time, 0.0);
	 continue;
       }
	 
       if(greedyAlgo){
	 greedySupportPropositions(ft, ft1, time);
       }
       else{
	 maximalSubsetSupportPropositions(ft,ft1, time);
       }
//         printFact(ft->index); 
//         printFact(ft1->index); 
//         cout << " " <<  correlation(ft, ft1, time) << endl;
   
	 
     }
   } 
}

void setPropositionCorrelation(FtNode* ft, FtNode* ft1, int time, double c){
//   if(MUTEX_SCHEME != MS_NONE &&
//      ARE_MUTEX_FTS(time, ft, ft1, NULL, NULL)){
//     c = 0.0;
//   }
    
//  cout << "set corr, time = " << time << endl;

  if(propositionCorrelations.find(time) == propositionCorrelations.end()){
    //  cout << "new hash" << endl;
    propositionCorrelations[time] = new FtCorrelationHash();
  }

  pair<FtNode*, FtNode*>* mpair;

  if(ft->index < ft1->index){
    //cout << "1"<<endl;
    mpair = new pair<FtNode*, FtNode*>(ft, ft1);
   
    //cout << (ft->positive ? "pos " : "neg "); printFact(ft->index);
    //cout << (ft1->positive ? "pos " : "neg "); printFact(ft1->index);
  }
  else{
    //cout << "2"<<endl;
    mpair = new pair<FtNode*, FtNode*>(ft1, ft);
    //cout << (ft1->positive ? "pos " : "neg "); printFact(ft1->index);
    //cout << (ft->positive ? "pos " : "neg "); printFact(ft->index);
  }

  
  FtCorrelationHash::iterator i = propositionCorrelations[time]->find(mpair);

  //cout << "old size = " <<  propositionCorrelations[time]->size() << endl;

//    cout << time << " set " 
//         << mpair->first->positive << " " << mpair->first->index 
//         << " " << mpair->second->positive << " " << mpair->second->index 
//         << " " << c << " " << &(*i) << " " << &mpair
//         << endl;

  if(0 && i != propositionCorrelations[time]->end()){
    //    cout << "1"<<endl;
    ///cout << "set corr1 = " << c 
    //<< " " << &(*i) 
    //<< endl;
    (*i).second = c;
    delete mpair;
  }
  else{
    //  cout << "2"<<endl;
    //cout << "set corr = " << c 
    //<< " " <<  mpair
    //<< endl;
    (*propositionCorrelations[time])[mpair] = c;
  }

  //  cout << "new size = " <<  propositionCorrelations[time]->size() << endl;


}



double propositionConjunctionProbability(set<FtNode*>* fts, int time){
  float pr = 1.0;  // This line of code is intellectual propety of dr. nay nay
                   // who fixed her husband's code.  What a great wife.
                   // This code may be used free of use if these comments
                   // remain in tact. 
  bool print = //true;// 
    false;
  if(print)
    cout << "Prop conjunction time = " << time << endl;
  for(set<FtNode*>::iterator p = fts->begin(); 
      p != fts->end() && pr > 0.0; p++){
    


    pr *= (float)(*p)->info_at[time]->probability;
    if(print){
      cout << ((*p)->positive ? "pos " : "neg "); printFact((*p)->index); cout<<flush;
      cout << "pr0 = "<<  pr <<endl;
    }
    for(set<FtNode*>::iterator p1 = fts->begin(); 
	p1 != p && pr > 0.0; p1++){
      //       if(MUTEX_SCHEME != MS_NONE &&
      // 	 ARE_MUTEX_FTS(time, *p, *p1, NULL, NULL))
      // 	pr  = 0;
      //       else
      float c = correlation(*p, *p1, time);
      pr *= c;
      if(print){
	cout << "corr = "<< c   <<endl;
	cout << ((*p1)->positive ? "pos " : "neg "); printFact((*p1)->index);
	cout << "pr1 = "<<  pr <<endl;
      }
    }
    if(print)
      cout << "pr2 = "<<  pr <<endl;
  }

  if(pr > 1.0 && pr < 1.0 + error){
    pr = 1.0;
  }

  if(pr > 1.0 || pr < 0.0) {
    cout << "Error, Prop conjunction time = " << time << endl;
    cout << "pr = "<<  pr << " " << (pr == 1.0) << " " << (pr < 0.0) <<endl;
    printf("%f %f %d\n",pr, 1.0, (pr > 1.0));
    for(set<FtNode*>::iterator p = fts->begin(); p != fts->end() ; p++){
      cout << ((*p)->positive ? "pos " : "neg "); printFact((*p)->index);
      cout << "pr = "<<(*p)->info_at[time]->probability << endl;
    }   
    for(set<FtNode*>::iterator p = fts->begin(); p != fts->end() ; p++){
     for(set<FtNode*>::iterator p1 = fts->begin(); p1 != p ; p1++){
	cout << ((*p)->positive ? "pos " : "neg "); printFact((*p)->index);
 	cout << ((*p1)->positive ? "pos " : "neg "); printFact((*p1)->index);
 	cout << "corr = "<< correlation(*p, *p1, time)   <<endl;
     }   
    }   


    exit(0);
  }


  return pr;
}

double correlation(FtNode* ft1, FtNode* ft2, int time){
  //    cout << "get corr time = " << time << endl;

  pair<FtNode*, FtNode*>* mpair;
  if(ft1->index == ft2->index){
    if(ft1->positive == ft2->positive)
      return 1.0;
    else
      return 0.0; 
  }
  else if(MUTEX_SCHEME != MS_NONE && 
 	  ARE_MUTEX_FTS(time, ft1, ft2, NULL, NULL))
    return 0.0;
  else if( propositionCorrelations.find(time) != 
	   propositionCorrelations.end()){
    
    //    cout << ft1->index << " " << ft2->index << endl;
    if(ft1->index < ft2->index){
      //  cout << "1"<<endl;
      mpair = new pair<FtNode*, FtNode*>(ft1, ft2);    
      //cout << (ft1->positive ? "pos " : "neg "); printFact(ft1->index);
      //cout << (ft2->positive ? "pos " : "neg "); printFact(ft2->index);
   }
    else{ //if(ft1->index > ft2->index){
      //cout << "2"<<endl;
      mpair = new pair<FtNode*, FtNode*>(ft2, ft1);
      //cout << (ft2->positive ? "pos " : "neg "); printFact(ft2->index);
      //cout << (ft1->positive ? "pos " : "neg "); printFact(ft1->index);
   }
    
    FtCorrelationHash::iterator i = propositionCorrelations[time]->find(mpair);
 
    
    //cout << "size = " << propositionCorrelations[time]->size() <<endl;

 //    cout << time << " get " 
// 	 << mpair->first->positive << " " << mpair->first->index 
// 	 << " " << mpair->second->positive << " " << mpair->second->index 
// 	 << flush;

//     if(i != propositionCorrelations[time]->end())
//       cout << " " << (*i).second << " " << &(*i) << flush;

//     cout << endl;

   delete mpair;

    if(i != propositionCorrelations[time]->end()){
      //cout << "return1 = " << (*i).second 
      //<< " " <<  &(*i)
      //<<endl;
      return (*i).second;
    }
    else{
       cout << "Not found, assume ind" << endl;
     cout << time << " get " 
 	 << ft1->positive << " " << ft1->index 
 	 << " " << ft2->positive << " " << ft2->index 
 	 << endl;

       exit(0);
      return 1.0;
    }
  }


  return 1.0;
}
void actionAndEffectProbability(int time){
   double pr = 1.0;
   set<FtNode*> fts;
   EfNode* ef;
 
   //  cout << "getting  pr of acteff"<<endl;
 
  for(OpNode* op = gall_ops_pointer; op; op=op->next){
        if(!op->info_at[time])
      continue;
    

//     for(FtEdge* pre = op->preconds; pre; pre=pre->next){
//       fts.insert(pre->ft);
//     }
//     pr = propositionConjunctionProbability(&fts, time);
//     op->info_at[time]->probability = pr;  
//     (*op->unconditional->info_at[time]->probability)[0] = pr;  
//     fts.clear();

    for(int t = 0; t < 2; t++){
      if(t == 0)
	ef = op->unconditional;
      else
	ef = op->conditionals;

      for( ; ef; ef=ef->next){
	if(!ef->info_at[time])
	  continue;
	
	for(FtEdge* pre = ef->conditions; pre; pre=pre->next){
	  fts.insert(pre->ft);
	}
	for(FtEdge* pre = op->preconds; pre; pre=pre->next){
	  fts.insert(pre->ft);
	}
	pr = propositionConjunctionProbability(&fts, time);
	(*ef->info_at[time]->probability)[0] = pr;
	ef->info_at[time]->probability_sum += pr;
	fts.clear();
	// }  

//         if(!ef->op->is_noop)
//  	 cout << "Pr of " << ef->op->name << " " << ef->alt_index << endl;
//         else
//  	 cout << "Pr of NOOP" << endl;


	if(pr > 1 || pr < 0){
	  cout << "getting  pr of acteff"<<endl;
	  cout << "P = " << pr << endl;
	 exit(0);
	}
       //pr = 1;
      }
    }
  }

  //    cout << "done pr of acteff"<<endl;
 
}

void actionAndEffectCorrelation(int time){
  double corr = 1.0, pr = 1.0;
  set<FtNode*> fts;
  EfNode* ef, *ef1;
  
  for(OpNode* op = gall_ops_pointer; op; op=op->next){
   for(OpNode* op1 = op->next; op1; op1=op1->next){
//       //op/op1 corr
//       for(FtEdge* pre = op->preconds; pre ; pre=pre->next){
// 	fts.insert(pre->ft);
//       }
//       for(FtEdge* pre = op1->preconds; pre ; pre=pre->next){
// 	fts.insert(pre->ft);
//       }
//       pr = propositionConjunctionProbability(&fts, time);
//      corr = pr /(op->info_at[time]->probability * op1->info_at[time]->probability);
//       setActionCorrelation(op, op1, time, corr);
//       fts.clear();

//       for(EfNode* ef = op1->conditionals; ef; ef=ef->next){
// 	//op/op1-ef corr
// 	for(FtEdge* pre = op->preconds; pre ; pre=pre->next){
// 	  fts.insert(pre->ft);
// 	}
// 	for(FtEdge* pre = ef->conditions; pre ; pre=pre->next){
// 	  fts.insert(pre->ft);
// 	}
// 	pr = propositionConjunctionProbability(&fts, time);
// 	corr = pr /(op->info_at[time]->probability * (*ef->info_at[time]->probability)[0]);
// 	setActionEffectCorrelation(op, ef, time, corr);
// 	fts.clear();
//       }      
      
     for(int t = 0; t < 2; t++){
       if(t==0)
	 ef=op->unconditional;
       else
	 ef=op->conditionals;
       
       for(; ef; ef=ef->next){
	 // 	//op-ef/op1 corr
	 // 	for(FtEdge* pre = ef->conditions; pre ; pre=pre->next){
	 // 	  fts.insert(pre->ft);
	 // 	}
	 // 	for(FtEdge* pre = op1->preconds; pre ; pre=pre->next){
	 // 	  fts.insert(pre->ft);
	 // 	}
	 // 	pr = propositionConjunctionProbability(&fts, time);
	 // 	corr = pr /((*ef->info_at[time]->probability)[0] * op1->info_at[time]->probability);
	 // 	setActionEffectCorrelation(op1, ef, time, corr);
	 // 	fts.clear();
	 
	 for(int t1 = 0; t1 < 2; t1++){
	   if(t1==0)
	     ef1=op1->unconditional;
	   else
	     ef1=op1->conditionals;
	   
	   for(; ef1; ef1=ef1->next){
	     //op-ef/op1-ef corr


	     if(MUTEX_SCHEME != MS_NONE &&
		ARE_MUTEX_EFS(time, ef, ef1, NULL, NULL)){
	       setEffectCorrelation(ef, ef1, time, 0.0);
	       break;
	     }

	     for(FtEdge* pre = ef->conditions; pre ; pre=pre->next){
	       fts.insert(pre->ft);
	     }
	     for(FtEdge* pre = op->preconds; pre ; pre=pre->next){
	       fts.insert(pre->ft);
	     }
	     for(FtEdge* pre = ef1->conditions; pre ; pre=pre->next){
	       fts.insert(pre->ft);
	     }
	     for(FtEdge* pre = op1->preconds; pre ; pre=pre->next){
	       fts.insert(pre->ft);
	     }
	     pr = propositionConjunctionProbability(&fts, time);
	     // 	     cout << pr << " / (" 
	     //		  <<  (*ef->info_at[time]->probability)[0] << " * "
	     //		  <<(*ef1->info_at[time]->probability)[0] << ")"<<endl;
	     //	     cout << endl;
	     corr = pr /((*ef->info_at[time]->probability)[0] * (*ef1->info_at[time]->probability)[0]);
	     setEffectCorrelation(ef, ef1, time, corr);
	     fts.clear();
	   }
	 }
       }      
     }
   }
   
   //    //op-ef/op-ef1
   //     for(EfNode* ef = op->conditionals; ef; ef=ef->next){
   //       for(EfNode* ef1 = ef->next; ef1; ef1=ef1->next){
   //  	for(FtEdge* pre = ef->conditions; pre ; pre=pre->next){
   // 	  fts.insert(pre->ft);
   // 	}
   // 	for(FtEdge* pre = ef1->conditions; pre ; pre=pre->next){
   // 	  fts.insert(pre->ft);
   // 	}
   // 	pr = propositionConjunctionProbability(&fts, time);
   // 	corr = pr /((*ef->info_at[time]->probability)[0] * (*ef1->info_at[time]->probability)[0]);
   // 	setEffectCorrelation(ef, ef1, time, corr);
   // 	fts.clear();
   //       }
   //     }
  }
  
}

void setActionCorrelation(OpNode* op, OpNode* op1, int time, double c){
//   if(actionEffectCorrelations.find(time) == actionEffectCorrelations.end())
//     actionEffectCorrelations[time] = new AECorrelationHash();

//   pair<void*, void*>* mpair;
//   if(op->alt_index < op1->alt_index)
//     mpair = new pair<void*, void*>(op, op1);
//   else
//     mpair = new pair<void*, void*>(op1, op);

//   AECorrelationHash::iterator i = actionEffectCorrelations[time]->find(mpair);

//   if(i != actionEffectCorrelations[time]->end()){
//     (*i).second = c;
//     delete mpair;
//   }
//   else
//     (*actionEffectCorrelations[time])[mpair] = c;

}
void setActionEffectCorrelation(OpNode* op, EfNode* ef, int time, double c){
//   if(actionEffectCorrelations.find(time) == actionEffectCorrelations.end())
//     actionEffectCorrelations[time] = new AECorrelationHash();

//   pair<void*, void*>* mpair = new pair<void*, void*>(op, ef);

//   AECorrelationHash::iterator i = actionEffectCorrelations[time]->find(mpair);

//   if(i != actionEffectCorrelations[time]->end()){
//     (*i).second = c;
//     delete mpair;
//   }
//   else
//     (*actionEffectCorrelations[time])[mpair] = c;
}

void setEffectCorrelation(EfNode* ef, EfNode* ef1, int time, double c){
  if(actionEffectCorrelations.find(time) == actionEffectCorrelations.end())
    actionEffectCorrelations[time] = new AECorrelationHash();

  pair<EfNode*, EfNode*>* mpair;
  if(ef->alt_index < ef1->alt_index)
    mpair = new pair<EfNode*, EfNode*>(ef, ef1);
  else
    mpair = new pair<EfNode*, EfNode*>(ef1, ef);

  AECorrelationHash::iterator i = actionEffectCorrelations[time]->find(mpair);

  //  cout << "set eff "<< ef->alt_index << " " << ef1->alt_index 
  //       << " corr = " << c 
  //       << " at time = " << time
  //       <<endl;
//   if(ef->op->is_noop){
//     cout << "NOOP " << ef->alt_index << endl; printFact(ef->op->preconds->ft->index);
//   }
//   else{
//     cout << ef->op->name << " " << ef->alt_index << endl;
//   }
//   if(ef1->op->is_noop){
//     cout << "NOOP " << ef1->alt_index << endl; printFact(ef1->op->preconds->ft->index);
//   }
//   else{
//     cout << ef1->op->name << " " << ef1->alt_index << endl;
//   }

  if(i != actionEffectCorrelations[time]->end()){
    (*i).second = c;
    delete mpair;
  }
  else
    (*actionEffectCorrelations[time])[mpair] = c;
}


void actionProbability(int time){
//   //   cout << "act prob: " << time <<endl;

//   for(OpNode* op = gall_ops_pointer; op; op=op->next){
//     //    cout << op->name;
//    double pr = 1.0;
//    set<FtNode*> fts;
//     for(FtEdge* pre = op->preconds; pre && pr > 0.0; pre=pre->next){
//       fts.insert(pre->ft);
//     }
//     pr = propositionConjunctionProbability(&fts, time);
//      //    cout << " " << pr << endl;
//     op->info_at[time]->probability = pr;
//   }
}
 
void actionCorrelaton(int time){
  cout << "act corr: " << time <<endl;
}

bool sameOutcome(set<int>* a, set<int>* b){
  //is there a set intersection

  for(set<int>::iterator i = a->begin(); i != a->end(); i++){
    if(b->find(*i) != b->end())
      return true;
  }
  return false;

}

double actionAndEffectSetProbability(set<OpNode*>* ops, 
				     map<EfNode*, set<int>* >* efs,
				     int time){
  double pr = 1.0;
  set<FtNode*> fts;


  //cout << "eff set pr " <<endl;
  
//   for(set<OpNode*>::iterator op = ops->begin(); 
//       op != ops->end(); op++){


//     //pr *= (*op)->info_at[time]->probability;
//     set<OpNode*>::iterator opt = op;
//     opt++;
//     for(set<OpNode*>::iterator op1 = opt;
// 	op1 != ops->end(); op1++){

//       pr *= correlation(*op, *op1, time);
//     }   
//     for(map<EfNode*, set<int>* >::iterator ef = efs->begin();
// 	ef != efs->end(); ef++){
//       if((*ef).second->size() > 0)
// 	pr *= correlation(*op, (*ef).first, time);
//     }   
//   }
 
  for(map<EfNode*, set<int>* >::iterator ef = efs->begin();
      ef != efs->end(); ef++){
    if((*ef).second && (*ef).second->size() > 0){
      pr *= (*(*ef).first->info_at[time]->probability)[0];

      cout << " p " << (*(*ef).first->info_at[time]->probability)[0];
    for(map<EfNode*, set<int>* >::iterator eft = efs->begin();
	eft != ef; eft++){
      if((*ef).second && (*ef).second->size() > 0 && 
	 (*eft).second && (*eft).second->size() > 0 ){
	if(//(*ef).first != (*eft).first &&
	   //!(!sameOutcome((*ef).second, (*eft).second) &&
	     (*ef).first->op != (*eft).first->op
	   ){

	  pr *= correlation((*ef).first, (*eft).first, time);
	  cout << " c1 " << correlation((*ef).first, (*eft).first, time);
	}
	else{ //if(//(*ef).first == (*eft).first ||
		//sameOutcome((*ef).second, (*eft).second)||
	  //	(*ef).first->op == (*eft).first->op){
	  pr *= 0.0;
	 cout << " c 0.0";
	}	
      }

    }
    }
    
  }   
    cout <<endl;
//   for(set<OpNode*>::iterator op = ops->begin(); 
//       op != ops->end(); op++){
//     for(FtEdge* pre = (*op)->preconds; pre; pre=pre->next){
//       fts.insert(pre->ft);
//     }       
//   }
//   for(map<EfNode*, set<int>* >::iterator ef = efs->begin(); 
//       ef != efs->end(); ef++){
//     for(FtEdge* pre = (*ef).first->conditions; pre; pre=pre->next){
//       fts.insert(pre->ft);
//     }       
//   }
//   pr = propositionConjunctionProbability(&fts, time);
// 
  if(pr>1 || pr < 0){
    //pr = 1;
    exit(0);
    cout << "eff set pr " <<endl;
 cout << "pr = " << pr << endl;
  }
  return pr;
}


double correlation(OpNode* op1, OpNode* op2, int time){
//   pair<void*, void*>* mpair;
//   if(MUTEX_SCHEME != MS_NONE &&
//      ARE_MUTEX_OPS(time, op1, op2, NULL, NULL))
//     return 0.0;
//   if( actionEffectCorrelations.find(time) != 
// 	   actionEffectCorrelations.end()){
    
//     if(op1->alt_index < op2->alt_index){
//       mpair = new pair<void*, void*>(op1, op2);    
//     }
//     else if(op1->alt_index > op2->alt_index){
//       mpair = new pair<void*, void*>(op2, op1);
//     }
    
//     AECorrelationHash::iterator i = actionEffectCorrelations[time]->find(mpair);
//     delete mpair;
    
//     if(i != actionEffectCorrelations[time]->end()){
//       //cout << " HO " << (*i).second << endl;
//       //exit(0);
//       return (*i).second;
//     }
//     else{
//       return 1.0;
//     }
//   }
  return 1.0;
}

void effectProbability(int time){
  //cout << "eff prob: " << time <<endl;

  for(OpNode* op = gall_ops_pointer; op; op=op->next){
    for(EfNode* ef = op->conditionals; ef; ef=ef->next){
      for(int i = 0; i < ef->info_at[time]->probability->size(); i++){
	//      cout << ef->op->name;
	double pr = //ef->op->info_at[time]->probability *
	  (*ef->effect->probability)[i];
	set<FtNode*> fts;
	for(FtEdge* pre = ef->conditions; pre && pr > 0.0; pre=pre->next){
	  fts.insert(pre->ft);
	}
	for(FtEdge* pre = ef->op->preconds; pre && pr > 0.0; pre=pre->next){
	  fts.insert(pre->ft);
	}
	pr *= propositionConjunctionProbability(&fts, time);
	//     cout << " " << pr << endl;
	(*ef->info_at[time]->probability)[i] = pr;
	ef->info_at[time]->probability_sum += pr;
      }  
    }
  }
}

void effectCorrelaton(int time){
}

double correlation(OpNode* op1, EfNode* ef2, int time){
//   pair<void*, void*>* mpair;
//   if(MUTEX_SCHEME != MS_NONE &&
//      ARE_MUTEX_EFS(time, op1->unconditional, ef2, NULL, NULL))
//      return 0;
//   else if( actionEffectCorrelations.find(time) != 
// 	   actionEffectCorrelations.end()){
    
//     mpair = new pair<void*, void*>(op1, ef2);    
        
//     AECorrelationHash::iterator i = actionEffectCorrelations[time]->find(mpair);
//     delete mpair;
    
//     if(i != actionEffectCorrelations[time]->end()){
//       return (*i).second;
//     }
//     else{
//       return 1.0;
//     }
//   }
  return 1.0;
}

double correlation(EfNode* ef1, EfNode* ef2, int time){
  pair<EfNode*, EfNode*>* mpair;

  //  cout << "get eff corr " 
  //       << ef1->alt_index << " " << ef2->alt_index 
  //       << " at time = " << time 
  //   <<endl;

    if(MUTEX_SCHEME != MS_NONE &&
        ARE_MUTEX_EFS(time, ef1, ef2, NULL, NULL))
      return 0;
    else if(ef1 == ef2){
      return 0;
    }
    else  if( actionEffectCorrelations.find(time) != 
	   actionEffectCorrelations.end()){
    

    if(ef1->alt_index < ef2->alt_index){
      mpair = new pair<EfNode*, EfNode*>(ef1, ef2);    
    }
    else {//if(ef1->alt_index > ef2->alt_index){
      mpair = new pair<EfNode*, EfNode*>(ef2, ef1);
    }
    //    else
    //      return 1.0;
     
    
    AECorrelationHash::iterator i = actionEffectCorrelations[time]->find(mpair);
    delete mpair;
    
    if(i != actionEffectCorrelations[time]->end()){
      return (*i).second;
    }
    else{
      cout << "NOT FOUND "<<endl;
      exit(0);
      return 1.0;
    }
  }
  return 1.0;
}


bool goal = false;

double recurseAlternatingSums(map<EfNode*, set<int>* >* efs,
			      map<EfNode*, set<int>* >::iterator ef,
			      set<int>::iterator ef_outcome,
			      map<EfNode*, set<int>* >* in, 
			      int num_efs,
			      int depth,
			      int supporters,
			      bool actions, 
			      int time){
 double pr;

//   if(goal)
//       cout << (*ef).second->size() << " "
// 	   << depth << " "
// 	   << supporters << " "
// 	   << endl;


  if(depth == supporters){//ef_outcome == (*ef).second->end()){
//      if(goal)
//    cout << "term " << num_efs << " " << actions << endl;
    if(num_efs > 0){
      pr = pow(-1.0, num_efs-1);
      //cout << "pr1 = " <<  pr << flush << endl;
      if(!actions){
	for(map<EfNode*, set<int>* >::iterator i = in->begin(); i != in->end(); i++){
	  for(set<int>::iterator j = (*i).second->begin(); j != (*i).second->end(); j++){
	    if((*(*i).first->effect->probability).find(*j) !=
	       (*(*i).first->effect->probability).end()){
	      //if((*(*i).first->effect->probability)[*j] != 0.0)  	  
	      // 	    cout << " * " <<  (*(*i).first->effect->probability)[*j] << flush;
	      // 	    if((*i).first->op->is_noop)
	      // 	      cout << "(noop) " <<flush;
	      // 	      else
	      // 	    cout << "(" <<(*i).first->op->name << " " << (*i).first->alt_index << ") " <<flush;
	      pr *= (*(*i).first->effect->probability)[*j];  	  
	      //   else{
	      // 	      cout << "problem" <<endl;
	      // 	      exit(0);
	      // 	    }
	  }
	  }
	}
      }
      else if(actions){	    
	pr *= actionAndEffectSetProbability(NULL, in, time);
      }
      //cout << "pr2 = " << pr << endl; 
    }    

  //     cout << endl;

    else
      pr = 0.0;   
  }
  else{
//     if(goal)
    //  cout << "recurse" << endl;
    pr = 0.0;
  

    while(ef != efs->end() &&
	  ((*ef).second->size() == 0 )){
      // cout << "ef+"<<endl;
      ef++;
      if(ef != efs->end())
	ef_outcome = (*ef).second->begin();
 //    if(!(*ef).first->op->is_noop)
//       cout << (*ef).first->op->name << " " << *ef_outcome << endl;
    
    }
 
    map<EfNode*, set<int>* >::iterator efp = ef;
    set<int>::iterator efp_outcome = ef_outcome;

    //try next outcome
    if(efp_outcome != (*ef).second->end())
      efp_outcome++;
  

    if(efp_outcome == (*ef).second->end()){
      //if no next outcome, try next effect

      if(efp != efs->end())
	efp++;

      while(efp != efs->end() &&
	    (*efp).second->size() == 0){	
	efp++;
      }
  
      if(efp == efs->end()){
	efp = ef;		
      }

      efp_outcome = (*efp).second->begin();


    }

    if(in->find((*ef).first) == in->end()){
      (*in)[(*ef).first] = new set<int>();
    }


 
 //        cout << "recurse" << flush;
//     if((*ef).first->op->is_noop)
//       cout << "(noop) " <<flush;
//     else
//       cout << "(" <<(*ef).first->op->name << " " << (*ef).first->alt_index << ") " <<flush;     
//     cout << endl;
    //only try outcomes if another outcome of same action is not used

    bool other_outcome = false;
    for(map<EfNode*, set<int>* >::iterator e = in->begin(); e != in->end(); e++){
      if(actions && 
	 (*e).second && (*e).second->size() > 0  &&
	 
	 (((*e).first->op == (*ef).first->op && 
	   (*e).second->find(*ef_outcome) != (*e).second->end())	  ||
	  
 	  (MUTEX_SCHEME != MS_NONE && 
 	   (ARE_MUTEX_EFS(time, (*e).first, (*ef).first, NULL, NULL) ||
 	    ARE_MUTEX_OPS(time, (*e).first->op, (*ef).first->op, NULL, NULL))
	   
	   )
	  )){
	//printf("!!!!!MUTEX!!!!!!!!!!!\n");fflush(stdout);
	other_outcome = true;
	break;
      }
    }
  
    if(!other_outcome){
      (*in)[(*ef).first]->insert(*ef_outcome);
      pr += recurseAlternatingSums(efs, efp, efp_outcome, in, num_efs+1, depth+1, supporters, actions, time);
    }
    (*in)[(*ef).first]->erase(*ef_outcome);
    pr += recurseAlternatingSums(efs, efp, efp_outcome, in, num_efs, depth+1, supporters, actions, time);


  }
//   if(goal)
  //    cout << "pr = " << pr << endl;
  return pr;
}
double actionDisjunctionProbability(map<EfNode*, set<int>* >* efs,
				    int supporters, int time){
  map<EfNode*, set<int>* > in;


//   if(goal){
//  cout << "E size = " << efs->size() << endl;
//   }

  map<EfNode*, set<int>* >::iterator ef = efs->begin();
  set<int>::iterator ef_outcome = (*ef).second->begin();


  while(ef != efs->end() &&
	((*ef).second->size() == 0)){

    ef++;
    if(ef != efs->end())
      ef_outcome = (*ef).second->begin();
  } 

  double val;

  if(ef != efs->end()){
    val = recurseAlternatingSums(efs, ef,ef_outcome, &in, 0, 0, supporters, true, time);
  }
  else
    val = 0.0;


  for(map<EfNode*, set<int>* >::iterator i = in.begin(); i != in.end(); i++){
    delete (*i).second;
  }

  cout << "val = " << val << endl;

  return val;
}

double effectsGivenActionsProbability(map<EfNode*, set<int>* >* efs, 
				      int supporters, int time){//, set<OpNode*>* ops){
  map<EfNode*, set<int>* > in;


//   if(goal){
//  cout << "E size = " << efs->size() << endl;
//   }

  map<EfNode*, set<int>* >::iterator ef = efs->begin();
  set<int>::iterator ef_outcome = (*ef).second->begin();


  while(ef != efs->end() &&
	((*ef).second->size() == 0)){

    ef++;
    if(ef != efs->end())
      ef_outcome = (*ef).second->begin();
  } 


  
  double val;

  if(ef != efs->end()){
    val = recurseAlternatingSums(efs, ef,ef_outcome, &in, 0, 0, supporters, false, time);
  }
  else
    val = 0.0;


  for(map<EfNode*, set<int>* >::iterator i = in.begin(); i != in.end(); i++){
    delete (*i).second;
  }
  return val;
}

int my_time = 0;

double recurseSupportProbabilities(map<EfNode*, set<int>* >* efs,
				   map<EfNode*, set<int>* >::iterator ef,
				   set<int>::iterator ef_outcome,
				   map<EfNode*, set<int>* >* in, 
				   map<EfNode*, set<int>* >* out, 
				   int num_efs,
				   int depth,
				   int supporters,
				   int method,
				   int time){

  set<OpNode*> A, B, C;
  map<EfNode*, set<int>* > inout;
  double pr;
  bool print = //  
  true;//false;

  ///cout << depth << " " << supporters << endl;

  if(depth == supporters){
    //cout << "term" <<endl;
   if(num_efs > 0){
      for(map<EfNode*, set<int>* >::iterator i = in->begin();
	  i != in->end(); i++){
	if((*i).second->size() > 0){
	  A.insert((*i).first->op);
	  C.insert((*i).first->op);
	  
	  if(inout.find((*i).first) == inout.end())
	    inout[(*i).first] = new set<int>();
	  for(set<int>::iterator j = (*i).second->begin();
	      j != (*i).second->end(); j++)
	    inout[(*i).first]->insert(*j);
	}
      }

      if(A.size() == 0)
	return 0.0;

      for(map<EfNode*, set<int>* >::iterator i = out->begin();
	  i != out->end(); i++){
	if((*i).second->size() > 0){
	  B.insert((*i).first->op);
	  C.insert((*i).first->op);

	  if(inout.find((*i).first) == inout.end())
	    inout[(*i).first] = new set<int>();
	  for(set<int>::iterator j = (*i).second->begin();
	      j != (*i).second->end(); j++)
	    inout[(*i).first]->insert(*j);
	}
      }
      if(print)
	cout << "|A| = " << A.size() <<  " |B| = " << B.size() << endl;
     
      switch (method) {
      case 1 : 
	pr = actionAndEffectSetProbability(&A, in, time);
	if(print)
	  cout << "pr0 = " << pr << endl;
	if(pr > 0 && B.size() > 0){
	  pr *= (1.0 - actionDisjunctionProbability(out, depth-num_efs, time));
	  //pr *= (1.0 - actionAndEffectSetProbability(&B, out, time));
	  if(print)
	    cout << "pr0a = " << pr << endl;
	}
	break;
      case 2 :
	pr = actionAndEffectSetProbability(&A, in, time);
	//	cout << "pr0 = " << pr << endl;
	if(B.size() > 0)
	  //pr *= 1.0 - actionAndEffectSetProbability(&C, &inout, time);
	  pr *= 1.0 - actionDisjunctionProbability(&inout, depth-num_efs, time);
	break;
      case 3 :
      default :
	double z = actionAndEffectSetProbability(&C, &inout, time);
	if(B.size() > 0){
	  pr = (z * (1.0 - z))/actionAndEffectSetProbability(&B, out, time);
	}
	else
	  pr = z;
	//	cout << "pr0 = " << pr << endl;
	break;
      }
      //      cout << "pr1 = " << pr << endl;
      pr *= effectsGivenActionsProbability(in, num_efs, time);
//       for(map<EfNode*, set<int>* >::iterator i = in->begin(); i != in->end(); i++){
// 	for(set<int>::iterator j = (*i).second->begin(); j != (*i).second->end(); j++){
// 	  if((*(*i).first->effect->probability).find(*j) !=
/// 	     (*(*i).first->effect->probability).end()){
// 	    // 	    if((*i).first->op->is_noop)
// 	    // 	      cout << "(noop) " <<flush;
// 	    // 	      else
// 	    // 	    cout << "(" <<(*i).first->op->name << " " << (*i).first->alt_index << ") " <<flush;
// 	    pr *= (*(*i).first->effect->probability)[*j];  	  
// 	  }
// 	}    
//       }
     for(map<EfNode*, set<int>* >::iterator i = inout.begin();
	 i != inout.end(); i++)
       delete (*i).second;

    }
    else
      pr = 0.0;   
      if(print)
   cout << "pr2 = " << pr << endl;
  }
  else{
    //cout << "recurse" << endl;
    pr = 0.0;
  
    while(ef != efs->end() &&
	  ((*ef).second->size() == 0 )){
      ef++;
      if(ef != efs->end())
	ef_outcome = (*ef).second->begin();
    }
 
    map<EfNode*, set<int>* >::iterator efp = ef;
    set<int>::iterator efp_outcome = ef_outcome;

    //try next outcome
    if(efp_outcome != (*ef).second->end())
      efp_outcome++;
  
    if(efp_outcome == (*ef).second->end()){
      //if no next outcome, try next effect
      
      if(efp != efs->end())
	efp++;
      
      while(efp != efs->end() &&
	    (*efp).second->size() == 0){	
	efp++;
      }
      
      if(efp == efs->end()){
	efp = ef;		
      }

      efp_outcome = (*efp).second->begin();


    }

    if(in->find((*ef).first) == in->end()){
      (*in)[(*ef).first] = new set<int>();
    }
    if(out->find((*ef).first) == out->end()){
      (*out)[(*ef).first] = new set<int>();
    }


 
    //        cout << "recurse" << flush;


    //only try outcomes if another outcome of same action is not used
    bool other_outcome = false;
    for(map<EfNode*, set<int>* >::iterator e = in->begin(); e != in->end(); e++){
      if((*e).second->size() > 0  &&
	 
	 (((*e).first->op == (*ef).first->op //&& 
	   //(*e).second->find(*ef_outcome) != (*e).second->end()
)	  ||
 	  (MUTEX_SCHEME != MS_NONE && 
 	   (ARE_MUTEX_EFS(time, (*e).first, (*ef).first, NULL, NULL) ||
 	    ARE_MUTEX_OPS(time, (*e).first->op, (*ef).first->op, NULL, NULL))
	   
	   )
	  )){
	if(print)
	  printf("!!!!!MUTEX!!!!!!!!!!!\n");fflush(stdout);
	other_outcome = true;
	break;
      }
    }
    

    if(!other_outcome){
      if(print){
       cout << "in " << depth <<endl;
       if((*ef).first->op->is_noop)
 	cout << "(noop) "  << *ef_outcome<<flush;
       else
            cout << "(" <<(*ef).first->op->name << " " << (*ef).first->alt_index << ") outcome = " << *ef_outcome <<flush;     
       cout << endl;
      }
      (*in)[(*ef).first]->insert(*ef_outcome);    
      pr += recurseSupportProbabilities(efs, efp, efp_outcome, in, out, 
					num_efs+1, depth+1, supporters, method, time);
      (*in)[(*ef).first]->erase(*ef_outcome);
    }
    if(print){
     cout << "out " << depth <<endl;
          if((*ef).first->op->is_noop)
            cout << "(noop) " <<flush;
          else
            cout << "(" <<(*ef).first->op->name << " " << (*ef).first->alt_index << ") " <<flush;     
 	 cout << endl;
    }
    if(!(*ef).first->op->is_noop){
    (*out)[(*ef).first]->insert(*ef_outcome);    
    pr += recurseSupportProbabilities(efs, efp, efp_outcome, in, out, 
				      num_efs, depth+1, supporters, method, time);
    (*out)[(*ef).first]->erase(*ef_outcome);
    }
  }
  //cout << "pr = " << pr << endl;
  return pr;
}


double recurseSupportProbabilities(map<EfNode*, set<int>* >*Ep, 
				   int num_supporters, 
				   int time,
				   int method){
  map<EfNode*, set<int>* > in;
  map<EfNode*, set<int>* > out;
  map<EfNode*, set<int>* >::iterator current_eff = Ep->begin();
  set<int>::iterator current_outcome = (*current_eff).second->begin();
  
  while(current_eff != Ep->end() &&
	((*current_eff).second->size() == 0)){
    current_eff++;
    if(current_eff != Ep->end())
      current_outcome = (*current_eff).second->begin();
  } 

  double val;
  if(current_eff != Ep->end()){
    val = recurseSupportProbabilities(Ep, current_eff, current_outcome, 
				      &in, &out, 0, 0, num_supporters, 
				      method, time);
  }
  else
    val = 0.0;


  for(map<EfNode*, set<int>* >::iterator i = in.begin(); i != in.end(); i++){
    delete (*i).second;
  }
  for(map<EfNode*, set<int>* >::iterator i = out.begin(); i != out.end(); i++){
    delete (*i).second;
  }
  return val;
}

double effectProbabilityGivenPreconds(map<EfNode*, set<int>* > *Ep, 
				      set<FtNode*>* preconds,
				      int time, 
				      FtNode* ft1,
				      FtNode* ft2){
  map<EfNode*, set<int>* > enabled_ft1, enabled_ft2;
  int supporters_ft1 = 0, supporters_ft2 = 0;
  bool supported_ft1 = false, supported_ft2 = false;

  //find enabled effects

  if(!ft2)
    supported_ft2 = true;
  
  //cout<< "effpr given prec" <<endl;
  
  for(map<EfNode*, set<int>* >::iterator i = Ep->begin();
      i != Ep->end(); i++){

    if((*i).second->size() == 0)
      continue;

    bool enable = true;
    for(FtEdge *ft = (*i).first->conditions; ft; ft= ft->next){
      if(preconds->find(ft->ft) == preconds->end()){
	enable = false;
	break;
      }
    }

    for(FtEdge *ft = (*i).first->op->preconds; ft && enable; ft= ft->next){
      if(preconds->find(ft->ft) == preconds->end()){
	enable = false;
	break;
      }
    }

    if(!enable)
      continue;
    else{
//       if((*i).first->op->is_noop){
// 	cout << "supp by  noop" << endl;
//       }
//       else
// 	cout << "supp by " << (*i).first->op->name << endl;                       

      for(EfEdge *ef = ft1->adders; ef ; ef = ef->next){
	if((*i).first == ef->ef){
	  for(set<int>::iterator j = (*i).second->begin();
	      j !=  (*i).second->end(); j++)
	    supporters_ft1++;
	  enabled_ft1[(*i).first]=(*i).second;
	  supported_ft1 = true;
	  
	  break;
	}
      }
      if(ft2){
	for(EfEdge *ef = ft2->adders; ef; ef = ef->next){
	  if((*i).first == ef->ef){
	    for(set<int>::iterator j = (*i).second->begin();
		j !=  (*i).second->end(); j++)
	      supporters_ft2++;
	    enabled_ft2[(*i).first]=(*i).second;
	    supported_ft2 = true;	    
	    break;
	  }
	}
      }
    }
  }
  
  //cout << "is supp by " << endl;//(*i).first->op->name << endl;

    if(supported_ft2 && supported_ft1){
      //compute pr of enabled effects working
      double ret = effectsGivenActionsProbability(&enabled_ft1, supporters_ft1, time);
      if(supporters_ft2 > 0)
	ret *= effectsGivenActionsProbability(&enabled_ft2, supporters_ft2, time);

      return ret;
	//actionDisjunctionProbability(&enabled, supporters, time);
    }
    else
      return 0.0;
}

double recurseSumSupporterPreconditions(map<EfNode*, set<int>* > *Ep, 
					set<FtNode*>::iterator precond,
					set<FtNode*>* preconds,
					set<FtNode*>* in,
					set<FtNode*>* out,					
					int depth,
					int num_supporters, 
					int time,
					FtNode* ft1,
					FtNode* ft2, 
					double* tau_sum){
  float result = 0.0, ta = 0.0;
    FtNode* f;
  if(precond == preconds->end()){
    result = 1.0;
    set<FtNode*> t;

//     for(set<FtNode*>::iterator j = in->begin();
//   	j != in->end(); j++){
//       cout << (*j)->positive << flush; printFact((*j)->index);
//     }


//     }
    t.insert(in->begin(), in->end());

    //cout << "size = " << t.size() << endl;
   
    //cout << "result1 = " << result << endl;
  

      ta = propositionConjunctionProbability(&t, time);
      //cout << "1 ta =  " << ta <<endl;
     if(ta < 0){
	cout << "1 ta =  " << ta <<endl;
	exit(0);
      }


      for(set<FtNode*>::iterator j = out->begin();       
	ta > 0.0 && j != out->end(); j++){
	if((*j)->positive){
	  f = gft_table[NEG_ADR((*j)->index)];
	}
	else
	  f = gft_table[(*j)->index];
	
	if(in->find(f) == in->end()){
	  ta *= (1.0 - (float)(*j)->info_at[time]->probability);
	}
      }
      //cout << "ta =  " << ta <<endl;


      if(ta < 0){
	//cout << "2 ta =  " << ta <<endl;
	for(set<FtNode*>::iterator j = out->begin();       
	    j != out->end(); j++){			 
	  cout << (*j)->info_at[time]->probability << endl;
	}
	exit(0);
      }

      (*tau_sum) += ta;
      result *= ta;
     
      if(result > 0.0){      
	result *= effectProbabilityGivenPreconds(Ep, &t, time, ft1, ft2);
      }   
      else if (result < 0){
	cout << "result2 = " << result << endl;
	exit(0);
     }

      //cout << "result2 = " << result << endl;

  }
  else{

     if((*precond)->positive){
       f = gft_table[NEG_ADR((*precond)->index)];
     }
     else
       f = gft_table[(*precond)->index];
	
    if(in->find(f) == in->end()){
      in->insert(*precond);
    
    precond++;
    result += recurseSumSupporterPreconditions(Ep, precond, preconds, in, out,
					       0, num_supporters, time,
					       ft1, ft2, tau_sum);
    precond--;

      in->erase(*precond);
    }
    
    if((*precond)->positive){
      f = gft_table[NEG_ADR((*precond)->index)];
    }
    else
      f = gft_table[(*precond)->index];
    
    if(out->find(f) == out->end()){
    out->insert(*precond);
    precond++;
    result += recurseSumSupporterPreconditions(Ep, precond, preconds, in, out, 
					       0, num_supporters, time, ft1, ft2, tau_sum);
    precond--;
    out->erase(*precond);
    }
  }
  
  return result;
}

double sumSupporterPreconditions(map<EfNode*, set<int>* > *Ep, 
				 int num_supporters, 
				 int time,
				 FtNode* ft1,
				 FtNode* ft2){
  set<FtNode*> preconds, in, out;
  double tau_sum = 0.0, tau_numer;
    FtNode* f;
 for(map<EfNode*, set<int>* >::iterator i = Ep->begin();
      i != Ep->end(); i++){
    if((*i).second->size() > 0){
      for(FtEdge* ft = (*i).first->conditions; ft; ft=ft->next){
// 	if(ft->ft->positive){
// 	  f = gft_table[NEG_ADR(ft->ft->index)];
// 	}
// 	else
// 	  f = gft_table[ft->ft->index];
	
//	if(preconds.find(f) == preconds.end()){
//	cout << "1 insert precond " << ft->ft->positive << flush; printFact(ft->ft->index);
// 	  if(ft->ft->info_at[time]->probability  == 1.0)
// 	    in.insert(ft->ft); //don't recurse over values if it is certain
// 	  else
	    preconds.insert(ft->ft); //recurse over uncertian props
	  //	}
      }
      for(FtEdge* ft = (*i).first->op->preconds; ft; ft=ft->next){
// 	if(ft->ft->positive){
// 	  f = gft_table[NEG_ADR(ft->ft->index)];
// 	}
// 	else
// 	  f = gft_table[ft->ft->index];
	
//	if(preconds.find(f) == preconds.end()){
//cout << "2 insert precond " << ft->ft->positive << flush; printFact(ft->ft->index);
// 	  if(ft->ft->info_at[time]->probability  == 1.0)
// 	    in.insert(ft->ft); //don't recurse over values if it is certain
// 	  else
	    preconds.insert(ft->ft);//recurse over uncertian props
	  //	}
      }
    }
  }
  tau_numer = recurseSumSupporterPreconditions(Ep, preconds.begin(), &preconds,
					       &in, &out, 0, num_supporters, time,
					       ft1, ft2, &tau_sum);
  //  cout << "pr of support = " << tau_numer << " / " << tau_sum << " = " << (tau_numer/tau_sum) << endl;

    if(tau_sum == 0.0)
       return 0.0;
     else
       return tau_numer/tau_sum; //normalize 
}


double computeProbabilityOfSupporters(set<OpNode*>* Ap, 
				      map<EfNode*, set<int>* > *Ep, 
				      int num_supporters, 
				      int time){

  //method 1 approximates Pr(a,-b) as Pr(a)(1-Pr(b))
  //method 2 approximates Pr(a,-b) as (1-Pr(a,b))Pr(a)
  //method 3 approximates Pr(a,-b) as (1-Pr(a,b))(Pr(a,b)/Pr(b))
  int method = 1; 
  double pr = 1.0;

  //compute action probabilities
  switch (method)  {
  case 1 : //nothing here
    break;
  case 2 : 
    pr = 1.0 - actionAndEffectSetProbability(Ap, Ep, time);
    break;
  case 3 :
  default :
    double p = actionAndEffectSetProbability(Ap, Ep, time);
    pr = (1.0 - p) * p;
    break;
  };

  //compute effect given action probabilities
  //sum over all possible ways to acheive a proposition
  pr = recurseSupportProbabilities(Ep, num_supporters, time, method);
  
  return pr;

}


bool noop_first(EfNode* e1, EfNode *e2){
  //cout << my_time <<endl;
  if(e1->op->is_noop)
    return 0;
  else if(e2->op->is_noop)
    return 1;
  //  else
  //  return e1->info_at[my_time]->probability_sum > e2->info_at[my_time]->probability_sum;
}

//since each efnode has a list of the action outcomes
//in which it appears, the following code is a little
//funky.  We need to consider and may use each efnode as
//if were several efnodes because of the multiple outcomes
void greedySupportPropositions(FtNode* ft1, FtNode* ft2, int time){
  set<OpNode*> Ap, tmpA;
  list<EfNode*> E;//, tmpE;
  map<EfNode*, set<int>* > Ep, EpPersist;//, tmpE, 
  map<EfNode*, set<int>* >::iterator ei;
  pair<EfNode*, int> bestE;
  double G = 0.0, Gp, bestG, GpPersist;
  int supporters = 0, tmp_supporters = 0;
  bool found_one = false;
  goal = 0;// goals.find(ft1) != goals.end();
  bool print = //      true;//
        false;

  bool supported_ft1 = false, supported_ft2 = false;

  if(print ||goal){
     cout << time << " Supoorting pr of " 
          << (ft1->positive ? "1 " : "0 "); 
     printFact(ft1->index); 
     if(ft2){ 
       cout << time << " And pr of " 
          << (ft2->positive ? "1 " : "0 "); 
     printFact(ft2->index); 
     }
   }

  //initialize E
  for(EfEdge *adder = ft1->adders; adder; adder=adder->next){
    E.push_back(adder->ef);
    

     if(goal){
      if(!adder->ef->op->is_noop)
        cout << "Can be Supported by " << adder->ef->op->name << endl;
      else{
        cout << "Can be Supported by NOOP "; 
      }
     }
  }

  if(ft2){
    for(EfEdge *adder = ft2->adders; adder; adder=adder->next){
      E.push_back(adder->ef);
     if(goal){
      if(!adder->ef->op->is_noop)
        cout << "Can be Supported by " << adder->ef->op->name << endl;
      else
        cout << "Can be Supported by NOOP" << endl;
     }
    }
  }
  else
    supported_ft2 = true;
  my_time = time-1;
  //   E.sort(noop_first);
  
  bool improved = true;


  while(improved){
    bestG = G;
    for(list<EfNode*>::iterator e = E.begin(); e != E.end(); e++){
      for(int i = 0; i < (*e)->effect->outcome->size(); i++){
	ei = Ep.find(*e);
	

	if(ei != Ep.end() && !(*ei).second)
	  (*ei).second = new set<int>();


	if((ei == Ep.end() || //*e is not in Ep
	    //!(*ei).second || (printf("HO\n") && 0) ||
	    (*ei).second->find(i) == (*ei).second->end()) &&
	   (*(*e)->effect->probability)[i] > 0){ //this outcome is not in Ep
 
	  if(print ||goal){
	  if(!(*e)->op->is_noop)
	    cout << "Trying " <<  (*e)->op->name << " "<< (*e)->alt_index << endl;
	  else
	    cout << "Trying NOOP" <<endl;
	  cout << "Outcome = " << i << " pr = " << (*(*e)->effect->probability)[i] <<endl;
	}
	  
	  
	  bool action_contained = false;


	  action_contained = (Ap.find((*e)->op) != Ap.end());
	  Ap.insert((*e)->op);
 	  if( ei == Ep.end()){
 	    Ep[*e] = new set<int>();
 	  }	    
	  Ep[*e]->insert(i);
	  
	  if(1){	  

	    if(0){
	      //sum over actions
	      Gp = computeProbabilityOfSupporters(&Ap, &Ep, supporters+1, time-1);
	    }
	    else{
	      //sum over preconditions
	      Gp = sumSupporterPreconditions(&Ep, supporters+1, time-1, ft1, ft2);
	    }
	  }
	  else{
	    double prAp = actionAndEffectSetProbability(&Ap, &Ep, time-1);
	    double prEpAp = 0.0;
	    if(prAp > 0){
	      prEpAp = effectsGivenActionsProbability(&Ep, supporters+1, time);
	    }
	    else
	      prEpAp = 0.0;
	    
	      
	    // 	  if(goal){  
	    // 	    //cout << "Considering " <<  (*e)->op->name << endl;
	    // 	    cout << "P(a) = " << (*e)->op->info_at[time-1]->probability << endl;
	    // 	    cout << "P(Ap) = " << prAp << endl;
	    // 	    cout << "P(Ep|Ap) = " << prEpAp << endl;
	    // 	    //cout << "Gp = " << prAp * prEpAp <<endl;
	    // 	  }
	      
	    Gp =  prAp * prEpAp;
	  }
	  
	  if(print || goal){
	    cout << "Gp = " << Gp  <<  endl;
	  }

	    if(Gp > bestG || 
	       (!found_one && !(supported_ft1 && supported_ft2)) ){
	      // if(goal){	    
	      //	      cout << "Set best " << Gp << endl;
	      // }
// 	      if(Gp > 1.0){
// 		Gp = 1.0;
// 	      } 
	      found_one = true;
	      bestG = Gp;
	      bestE.first = *e;
	      bestE.second = i;
	    }
	    if(!action_contained)
	      Ap.erase((*e)->op);

	    Ep[*e]->erase(i);	    

  if(Gp > 1.0 || Gp < 0.0){
      cout << "BUGGG" << endl << flush;
      cout << time << " Supoorting pr of " 
          << (ft1->positive ? "1 " : "0 "); 
     printFact(ft1->index); 
     if(ft2){ 
       cout << time << " And pr of " 
          << (ft2->positive ? "1 " : "0 "); 
     printFact(ft2->index); 
     }
    cout << " Gp = " << Gp <<endl;
     for(map<EfNode*, set<int>* >::iterator e = Ep.begin(); e != Ep.end(); e++){
       if((*e).second->size() == 0)
	 continue;

       if(!(*e).first->op->is_noop)
	 cout << "Supported by " << (*e).first->op->name << " " << (*e).first->alt_index << endl;
       else{
	 cout << "Supported by NOOP" << endl; printFact((*e).first->op->preconds->ft->index);
       }
       for(set<int>::iterator q = (*e).second->begin(); q != (*e).second->end(); q++)
	 cout << "Outcome = " << *q << " pr = " << (*(*e).first->effect->probability)[*q] <<endl;
     for(map<EfNode*, set<int>* >::iterator e1 = Ep.begin(); e1 != Ep.end(); e1++){
	if(*e != *e1 && (*e1).second->size() == 0 &&
	   MUTEX_SCHEME!=MS_NONE && 
	   ARE_MUTEX_EFS(time-1, (*e).first, (*e1).first, NULL, NULL)){
	  cout << "Mutex with: "<< endl;
	  if(!(*e1).first->op->is_noop)
	    cout << (*e1).first->op->name << " " << (*e1).first->alt_index << endl;
	  else
	    cout << "NOOP" << endl;
 	}
      }   
     }    
     exit(0);    
    }

	    


	  }
	}
      }
      
      if(found_one){
	supporters++;
	if(print ||goal){
	  if(!bestE.first->op->is_noop)
	    cout << "Supported by " << bestE.first->op->name << " " << bestE.first->alt_index << endl;
	  else
	    cout << "Supported by NOOP" << endl;
	  cout << bestG << endl;
	}

	for(EfEdge *e = ft1->adders; e; e=e->next){
	  if(bestE.first == e->ef){
	    supported_ft1 = true;
	    if(print)
	      cout << "supported ft1"<<endl;
	    break;
	  }
	}
	if(ft2){
	  for(EfEdge *e = ft2->adders; e; e=e->next){
	    if(bestE.first == e->ef){
	      supported_ft2 = true;
	      if(print)
		cout << "supported ft2"<<endl;
	      break;
	    }
	  }
	}

	//E.erase(bestE);
	G = (double)bestG;
	if(G < 1.0 || !(supported_ft1 && supported_ft2))
	  improved = true;
	else
	  improved = false;

	Ap.insert(bestE.first->op);
	Ep[bestE.first]->insert(bestE.second);
      }
      else{
	improved = false;      
      }
      found_one = false;
    }
  

  if(!ft2){
    if(supporters == 0){
      cout << "No supporters!!!" << endl;
      exit(0);
    }


    ft1->info_at[time]->probability = G;
    if(print ||goal)
      cout << " G = " << G <<endl;
    
  

    //probably also need to store actions in Ap for relaxed plan extraction
    EfEdge *ee = NULL;
    set<EfNode*> done;
   for(map<EfNode*, set<int>* >::iterator e = Ep.begin(); e != Ep.end(); e++){
     if(done.find((*e).first) != done.end() || (*e).second->size() == 0)
       continue;

     done.insert((*e).first);
     //      cout << "Supported by " << (*e).first->op->name << endl;
      EfEdge *ee1 = new_ef_edge((*e).first);
      ee1->next = ee;
      ee = ee1;      
    }    
    ft1->info_at[time]->relaxedPlanEdges = ee;
    

  }
  else{
    double a = G;
    double b = ft1->info_at[time]->probability*ft2->info_at[time]->probability;
    double t = (b > 0.0 ? G : a/b);
    if(print ||goal){
      cout << " G = " << G 
	   << " b = " << b
	   << " " << ft1->info_at[time]->probability
	   << " " << ft2->info_at[time]->probability
	   << endl << "corr = " << t
	   << endl;
    }

    setPropositionCorrelation(ft1, ft2, time, t);
    
  }

 for(map<EfNode*, set<int>* >::iterator i = Ep.begin(); i != Ep.end(); i++){
    delete (*i).second;
  }

}

bool inconsistent(EfNode* e1, EfNode* e2, int time){
  
  if(MUTEX_SCHEME!=MS_NONE)
    return ARE_MUTEX_OPS(time, e1->op, e2->op, NULL, NULL);
      //ARE_MUTEX_EFS(time, e1, e2, NULL, NULL);// && 
  //      !(e1->op->is_noop || e2->op->is_noop);
  else{
    return ef_interfere(e1, e2) || 
      interfere(e1->op, e2->op) || 
      ef_interfere(e1->op->unconditional, e2) ||
      ef_interfere(e1, e2->op->unconditional);
    }
    

}

bool checkConsistent(set<EfNode*>* in, int time){
  for(set<EfNode*>::iterator i = in->begin(); i != in->end(); i++){
    set<EfNode*>::iterator j = i;
    j++;
    for(; j != in->end(); j++){
      //cout << "CHECK MUTEX " << (*i)->alt_index << " " << (*j)->alt_index << endl;
      if(//!(*i)->op->is_noop && !(*j)->op->is_noop &&
	 inconsistent(*i, *j, time-1)){
	//cout << "MUTEX"<< endl;
	return false;
      }
      //cout << "OK"<< endl;
    }
  }
  return true;
}

bool supportBoth(set<EfNode*>* in,FtNode* ft1,FtNode* ft2){
  bool s1 = false, s2 = false;

  for(set<EfNode*>::iterator i = in->begin(); i != in->end(); i++){
    for(EfEdge *e = ft1->adders; e && !s1; e = e->next)
      if(e->ef == (*i))
	s1 = true;
    for(EfEdge *e = ft2->adders; e && !s2; e = e->next)
      if(e->ef == (*i))
	s2 = true;
  }

  return s1 && s2;
}

namespace __gnu_cxx{
struct efsetcmp
{
  bool operator()(const set<EfNode*>* s1, const set<EfNode*>* s2) const
  {
    if(s1->size() == s2->size()){
      for(set<EfNode*>::iterator i = s1->begin(); i != s1->end(); i++){
	if(s2->find(*i) == s2->end())
	  return false;
      }
      return true;
    }
    else
      return false;
  }
};
template<> struct hash<set<EfNode*>*>{
  size_t const operator()( set<EfNode*>* p) const {
    return (size_t)p->size();
  }
};
}
void recurseChopSubsets(list<EfNode*>::iterator eff,
			list<EfNode*>* E,
			set<EfNode*>* in,			
			hash_set<set<EfNode*>*,hash<set<EfNode*>*>, efsetcmp>* subsets,
			FtNode* ft1,
			FtNode* ft2,
			int time){
//   if(eff != E->end())
//     cout << "enter " << (*eff)->alt_index  << endl;
//   else
//     cout << "enter END" <<endl;

  if(in->size() == 0)
    return;
  

  //check if E - out is consistent
  bool consistent = checkConsistent(in, time);
  bool doesSupportBoth = (!ft2 || supportBoth(in, ft1, ft2));
  
//   cout << "consistent = " << consistent << endl;

  if(consistent && doesSupportBoth){

    set<EfNode*> *e = new set<EfNode*>();
    e->insert(in->begin(), in->end());
    //    if(subsets->find(e) == subsets->end()){
      subsets->insert(e);
      cout << "found subset " << in->size() << " " <<  subsets->size() << endl;
//     }
//     else{
//       cout << "Repeat" << endl;
//       delete e;
//     }


  }
  else //DAN// PUT THIS BACK IN TO GET MAXIMAL SUBSETS, NOT ALL SUBSETS
  if(eff != E->end() && doesSupportBoth) {

  
      cout << "leave " << (*eff)->alt_index <<endl;
      eff++;
      recurseChopSubsets(eff, E, in, subsets, ft1, ft2, time);
      eff--;
  
    
     cout << "remove "<< (*eff)->alt_index <<endl;
    in->erase(*eff);
    eff++;
    recurseChopSubsets(eff, E, in, subsets, ft1, ft2, time);
    eff--;
    in->insert(*eff);

  }
//   if(eff != E->end())
//     cout << "exit " << (*eff)->alt_index  << endl;
//   else
//     cout << "exit END" <<endl;
}

void maximalSubsetSupportPropositions(FtNode* ft1, FtNode* ft2, int time){
  bool print = //
true;//false;

  if(1||print ||goal){
     cout << time << " Supoorting pr of " 
          << (ft1->positive ? "1 " : "0 ");
     printFact(ft1->index); 
     if(ft2){ 
       cout << time << " And pr of " 
          << (ft2->positive ? "1 " : "0 ");
     printFact(ft2->index); 
     }
   }

  list<EfNode*> E;
  
  //initialize E
  for(EfEdge *adder = ft1->adders; adder; adder=adder->next){
    E.push_back(adder->ef);
  

     if(print){
      if(!adder->ef->op->is_noop)
        cout << "Can be Supported by " << adder->ef->op->name << endl;
      else{
        cout << "Can be Supported by NOOP "; 
      }
     }
  }

  if(ft2){
    for(EfEdge *adder = ft2->adders; adder; adder=adder->next){
      E.push_back(adder->ef);
     if(print){
      if(!adder->ef->op->is_noop)
        cout << "Can be Supported by " << adder->ef->op->name << endl;
      else
        cout << "Can be Supported by NOOP" << endl;
     }
    }
  }
  
  //find all maximal subsets of E, where no two actions are mutex
  hash_set<set<EfNode*>*,hash<set<EfNode*>*>, efsetcmp> subsets; //resulting subsets
  list<EfNode*>::iterator eff = E.begin(); //pointer to effect to branch on
  set<EfNode*> in; //temporary set of effects in set

  in.insert(E.begin(), E.end());
  recurseChopSubsets(eff, &E, &in, &subsets, ft1, ft2, time);


  double bestG = 0.0, G;
  double bestActs = DBL_MAX; 
  set<OpNode*> acts;
  set<EfNode*>* bestSet = NULL;
  map<EfNode*, set<int>* > Ep;

  if(print)
    cout << "Supported by subsets: " << subsets.size() << endl;

  for(hash_set<set<EfNode*>*,hash<set<EfNode*>*>, efsetcmp>::iterator i = subsets.begin(); 
      i != subsets.end(); i++){

    acts.clear();

    if(print)
      cout << endl << "Size = " << (*i)->size() << endl;

    int supporters = 0;
    for(set<EfNode*>::iterator e = (*i)->begin(); e != (*i)->end(); e++){
      if(!(*e)->op->is_noop)
      acts.insert((*e)->op);


      if(print){
   	if(!(*e)->op->is_noop)
	  cout << "Supported by " << (*e)->op->name << " " << (*e)->alt_index << endl;
 	else{
 	  cout << "Supported by NOOP" << endl; printFact((*e)->op->preconds->ft->index);
	}
      }

 
      for(int j = 0; j < (*e)->effect->outcome->size(); j++){
 	map<EfNode*, set<int>* >::iterator ei = Ep.find(*e);
 	if(ei != Ep.end() && !(*ei).second)
 	  (*ei).second = new set<int>();
	else if( ei == Ep.end())
	  Ep[*e] = new set<int>();

	Ep[*e]->insert(j);
	supporters++;

     }
    }
    G = sumSupporterPreconditions(&Ep, supporters, time-1, ft1, ft2);
    if(print)
      cout << "G = " << G << endl;

    for(map<EfNode*, set<int>* >::iterator j = Ep.begin(); j != Ep.end(); j++){
      (*j).second->clear();
    }

    if(G >= bestG || !bestSet){

      if(G == bestG && acts.size() >= bestActs){
	continue;
      }

      bestActs = acts.size();
      bestG = G;
      bestSet = (*i);
      if(bestG == 1.0 && bestActs == 0)
	break;
    }

  }
   for(map<EfNode*, set<int>* >::iterator j = Ep.begin(); j != Ep.end(); j++){
     if((*j).second)
       delete (*j).second;
   }
 
   if(1|| print){
     if(bestSet){
       for(set<EfNode*>::iterator e = bestSet->begin(); e != bestSet->end(); e++){
	 if(!(*e)->op->is_noop)
	   cout << "Supported by " << (*e)->op->name << " " << (*e)->alt_index << endl;
	 else{
	   cout << "Supported by NOOP" << endl; printFact((*e)->op->preconds->ft->index);
	 }
       }
     }
   }


  if(!ft2){
    if(bestG < 0 && bestG >= 0.0-error)
      bestG = 0;

    if(bestG < 0.0-error){
      cout << "1 bsetG = " << bestG <<endl;
     if(bestSet){
       for(set<EfNode*>::iterator e = bestSet->begin(); e != bestSet->end(); e++){
	 if(!(*e)->op->is_noop)
	   cout << "Supported by " << (*e)->op->name << " " << (*e)->alt_index << endl;
	 else{
	   cout << "Supported by NOOP" << endl; printFact((*e)->op->preconds->ft->index);
	 }
       }
     }
     else {
       cout << "No support" << endl;
     }
      exit(0);
    }
    ft1->info_at[time]->probability = bestG;
    if(1||print ||goal)
      cout << " G = " << bestG <<endl;
    
    EfEdge *ee = NULL;
    set<EfNode*> done;
    for(set<EfNode*>::iterator e = bestSet->begin(); e != bestSet->end(); e++){
      //      cout << "Supported by " << (*e).first->op->name << endl;
      EfEdge *ee1 = new_ef_edge((*e));
      ee1->next = ee;
      ee = ee1;      
    }    
    ft1->info_at[time]->relaxedPlanEdges = ee;
  }
  else{
    if(bestG < 0 && bestG >= 0.0-error)
      bestG = 0;

    if(bestG < 0.0-error){
      cout << "2 bsetG = " << bestG <<endl;
      if(bestSet){
       for(set<EfNode*>::iterator e = bestSet->begin(); e != bestSet->end(); e++){
	 if(!(*e)->op->is_noop)
	   cout << "Supported by " << (*e)->op->name << " " << (*e)->alt_index << endl;
	 else{
	   cout << "Supported by NOOP" << endl; printFact((*e)->op->preconds->ft->index);
	 }
       }
     }
     else {
       cout << "No support" << endl;
     }
     exit(0);
    }

    double a = bestG;
    double b = ft1->info_at[time]->probability*ft2->info_at[time]->probability;
    double t = (b <= 0.0 ? bestG : a/b);
     if(1||print ||goal){
      cout << " G = " << a
	   << " b = " << b
	   << " " << ft1->info_at[time]->probability
	   << " " << ft2->info_at[time]->probability
	   << endl << "corr = " << t
	   << endl;
    }

    setPropositionCorrelation(ft1, ft2, time, t);
    
    set<EfNode*> *support = new set<EfNode*>();
    support->insert(bestSet->begin(), bestSet->end());

    pair<FtNode*, FtNode*>* mpair;
    if(ft1->index < ft2->index)
      mpair = new pair<FtNode*, FtNode*>(ft1, ft2);
    else
      mpair = new pair<FtNode*, FtNode*>(ft2, ft1);

    map<int, FtPairSupportersHash* >::iterator y;
    y = propPairSupport.find(time);
    if(y == propPairSupport.end())
      propPairSupport[time] = new FtPairSupportersHash();

    (*propPairSupport[time])[mpair] = support;

  }
 
//   if(print){
//     cout << "Supported by subsets: " << subsets.size() << endl;
//     for(list<set<EfNode*>*>::iterator i = subsets.begin(); 
// 	i != subsets.end(); i++){
//       cout << "Size = " << (*i)->size() << endl;
//       for(set<EfNode*>::iterator e = (*i)->begin(); e != (*i)->end(); e++){
// 	if(!(*e)->op->is_noop)
// 	  cout << "Supported by " << (*e)->op->name << " " << (*e)->alt_index << endl;
// 	else{
// 	  cout << "Supported by NOOP" << endl; printFact((*e)->op->preconds->ft->index);
// 	}
//       }
//       cout << endl;
//     }

//   }

  for(hash_set<set<EfNode*>*,hash<set<EfNode*>*>, efsetcmp>::iterator i = subsets.begin(); 
      i != subsets.end(); i++){
    delete *i;
  }
}



double goalProbability(int time){
  if(goals.empty()){
    for(Integers *i = gbit_goal_state->positive->indices; i ; i=i->next){
      //      cout << "Setting +Goal "; printFact(i->index);
      if(gft_table[i->index])
	 goals.insert(gft_table[i->index]);
      else{
	goals.clear();
	return 0.0;
      }
    }
    for(Integers *i = gbit_goal_state->negative->indices; i ; i=i->next){
      //            cout << "Setting -Goal "; printFact(i->index);
      if(gft_table[NEG_ADR(i->index)])
	goals.insert(gft_table[NEG_ADR(i->index)]);
      else{
	goals.clear();
	return 0.0;
      }
    }
    // cout << "Set goal " << goals.size() << endl;
  }

  return propositionConjunctionProbability(&goals, time);
}




double relaxedPlanHeuristic(){

  double value = 0.0;
  set<FtNode*>* fts[graph_levels];
  set<EfNode*>* efs[graph_levels];
  set<OpNode*>* ops[graph_levels];


  if(graph_levels == IPP_MAX_PLAN-1)
    return DBL_MAX;

  fts[graph_levels-1] = &goals;
  //  cout << "levels = " << graph_levels << endl;
  if(graph_levels == IPP_MAX_PLAN-1)
    printBDD(b_initial_state);
  for(int i = graph_levels-1; i > -1; i--){
    // cout << "extracting rp " << i << endl;
    //support facts with effects
    efs[i] = new set<EfNode*>();
    ops[i] = new set<OpNode*>();


    //support the pairs of propositions
    if(0){//greedy find pairs
      while(fts[i]->size() > 1){
	set<FtNode*>::iterator fi = fts[i]->begin(), f = fts[i]->begin();
	f++;
	FtNode *maxFt = NULL;
	double maxCorr = 0.0, corr;
	//find max coorelation ft
	for(; f != fts[i]->end(); f++){
	  corr = correlation(*f, *fi, i+1);
	  if(!maxFt ||  maxCorr < corr){
	    maxFt = *f;
	    maxCorr = corr;
	  }
	}
	
	if(maxCorr == 0.0)
	  break;
	
	pair<FtNode*, FtNode*> mpair;
	if((*fi)->index < maxFt->index){
	  mpair.first = *fi;
	  mpair.second = maxFt;
	}
	else{
	  mpair.first = maxFt;
	  mpair.second = *fi;
	}
	//cout << "support pair " << mpair.first->index << " " << mpair.second->index << " " << maxCorr << endl;
	set<EfNode*> *supporters = (*propPairSupport[i+1])[&mpair];
	for(set<EfNode*>::iterator s = supporters->begin();
	    s != supporters->end(); s++){	
	  efs[i]->insert(*s);      
	  ops[i]->insert((*s)->op);
	}
	fts[i]->erase(*fi);
	fts[i]->erase(maxFt);
      }
    }
    else{//find highest regret pairs
      while(fts[i]->size() > 1){
	FtNode *b1 = NULL, *b2 = NULL;
	int bRegret = -1, regret, ffiSup;
	for(set<FtNode*>::iterator fi = fts[i]->begin();
	    fi != fts[i]->end(); fi++){
	  for(set<FtNode*>::iterator f = fts[i]->begin();
	      f != fi && f != fts[i]->end(); f++){

	    pair<FtNode*, FtNode*> mpair;
	    if((*fi)->index < (*f)->index){
	      mpair.first = *fi;
	      mpair.second = *f;
	    }
	    else{
	      mpair.first = *f;
	      mpair.second = *fi;
	    }

	    set<EfNode*> *supporters = (*propPairSupport[i+1])[&mpair];

	    if(supporters == NULL)
	      continue;

	    ffiSup = 0;
	    for(set<EfNode*>::iterator s = supporters->begin();
		s != supporters->end(); s++){	
	      if(!(*s)->op->is_noop)
		ffiSup++;
	    }
	    
	    set<OpNode*> iSup;
	    for(EfEdge *e = (*fi)->info_at[i+1]->relaxedPlanEdges; e; e = e->next){
	      if(!e->ef->op->is_noop)
		iSup.insert(e->ef->op);
	    }
	    for(EfEdge *e = (*f)->info_at[i+1]->relaxedPlanEdges; e; e = e->next){
	      if(!e->ef->op->is_noop)
		iSup.insert(e->ef->op);
	    }
	    //cout << "isup = " << iSup.size() << ", ffiSup = " << ffiSup <<endl;
	    regret = iSup.size() - ffiSup;
	    //cout << "regret = " << regret << " " << bRegret << endl;	    
	    if(regret > bRegret){
	     
	      b1 = *f;
	      b2 = *fi;
	      bRegret = regret;
	    }

	  }

	}
	if(!(b1 && b2)){
	  //cout << "No Good pair" << endl;
	  break;
	}

	pair<FtNode*, FtNode*> mpair1;
	if(b1->index < b2->index){
	  mpair1.first = b1;
	  mpair1.second = b2;
	}
	else{
	  mpair1.first = b2;
	  mpair1.second = b1;
	}
	//cout << "support pair " << mpair1.first->index << " " << mpair1.second->index << " " << bRegret << endl;
	set<EfNode*> *supporters = (*propPairSupport[i+1])[&mpair1];
	for(set<EfNode*>::iterator s = supporters->begin();
	    s != supporters->end(); s++){	
	  efs[i]->insert(*s);      
	  ops[i]->insert((*s)->op);
	}
	fts[i]->erase(b1);
	fts[i]->erase(b2);
      }
     
    }


    //support the single propositions
    for(set<FtNode*>::iterator f = fts[i]->begin(); 
	f != fts[i]->end(); f++){
      //cout << "Supoorting rp  "; printFact((*f)->index);
      for(EfEdge *e = (*f)->info_at[i+1]->relaxedPlanEdges; e; e = e->next){
	//cout << "Supported by " << e->ef->op->name << endl;
	efs[i]->insert(e->ef);      
	ops[i]->insert(e->ef->op);
      }
    }



    if(i != graph_levels-1)
      delete fts[i];



    for(set<OpNode*>::iterator o = ops[i]->begin(); o != ops[i]->end(); o++){
      if(!(*o)->is_noop)
	value += 1;
    }


//       for(set<OpNode*>::iterator o = ops[i]->begin(); o != ops[i]->end(); o++){
//         if(!(*o)->is_noop)
//   	cout << (*o)->name << endl;
// 	//        else{
// // 	 cout << "Noop "; printFact((*o)->preconds->ft->index);
// //        }
//       }
//       cout << "============" << endl;   
    
    //insert preconditions
    if(i > 0){
      fts[i-1] = new set<FtNode*>();
      for(set<EfNode*>::iterator e = efs[i]->begin(); 
	  e != efs[i]->end(); e++){
	for(FtEdge *f = (*e)->conditions; f; f=f->next){
	  fts[i-1]->insert(f->ft);	
	}
      }
      delete efs[i];
      for(set<OpNode*>::iterator o = ops[i]->begin(); 
	  o != ops[i]->end(); o++){
	for(FtEdge *f = (*o)->preconds; f; f=f->next){
	  fts[i-1]->insert(f->ft);			
	}
      }	        
//       for(set<FtNode*>::iterator f = fts[i-1]->begin();
// 	  f != fts[i-1]->end(); f++){
// 	cout << ((*f)->positive ? "pos " : "neg "); printFact((*f)->index);
//       }

      
      delete ops[i];
    }
  }
  

    cout  << "h = " << value <<endl;

  return value;

}

bool stopCORRGraph(int time){
  double pr = goalProbability(time);
     cout << time << " P(G) = " << pr << endl;

      //if(time > 10) exit(0);
  return  pr > goal_threshold;
} 
