#ifndef __ACTION_GROUP_H
#define __ACTION_GROUP_H

#include "action.h"
#include <string>
#include <vector>

class ActionGroup
{
public:
  ActionGroup(const std::string &ppddlActGrp);

  std::string asXML();
  std::string asRDDL();
  std::string asPPDDL();

protected:
  std::vector<Action> actions;
};

#endif // __ACTION_GROUP_H
