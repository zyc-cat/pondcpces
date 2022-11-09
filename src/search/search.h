#ifndef SEARCH_H
#define SEARCH_H

#include <list>

enum SearchType{
	ONLINE,
  ASTAR, // **
  EHC_SEARCH,
  MOLAO,
  LAO,// **
  RTDP_SEARCH,
  MOASTAR,
  REPLAN,
  HOP_SEARCH,// **
  PCHOP_SEARCH,
  ALAO_STAR,
  EXPH,// **
  AOSTAR
};

class DdNode;
class StateNode;
/**
 * 搜索的抽象基类
 */
class Search{
public:
  Search(SearchType type);// 仅设置类型
  virtual ~Search();

  virtual void init(int num_acts, DdNode* b_initial_state, DdNode*  b_goal_state);
	void resetPolicy();

	void incremental_search();// using several times search to reach to goal
  virtual void search() = 0;// the implement call in incremental_search

  SearchType getType(){ return type; }

protected:
  void printStateList();
  void graphToFile(const char* filename);

//private:
public:
  static SearchType type;     // TO-DO: Make it non-static once everything's in order
};

#endif
