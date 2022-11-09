#include "action_group.h"

#include <string>
#include <vector>
#include <sstream>

using namespace std;

ActionGroup::ActionGroup(const string &ppddlActGrp)
{
  const string delim = "___";
  int endIndex;

  for(int startIndex = 0; startIndex < ppddlActGrp.length(); startIndex = endIndex + delim.length())
  {
    endIndex = ppddlActGrp.find(delim, startIndex);
    if(endIndex == string::npos)
      endIndex = ppddlActGrp.length();
    if(startIndex != endIndex)
      actions.push_back(Action(ppddlActGrp.substr(startIndex, endIndex - startIndex)));
  }
}

std::string ActionGroup::asXML()
{
  ostringstream os;

  os << "<actions>";
  for(vector<Action>::iterator act_it = actions.begin(); act_it != actions.end(); act_it++)
    os << (*act_it).asXML();
  os << "</actions>";

  return os.str();
}

std::string ActionGroup::asRDDL()
{
  ostringstream os;
  bool first = true;

  for(vector<Action>::iterator act_it = actions.begin(); act_it != actions.end(); act_it++)
  {
    if(first)
      first = false;
    else
      os << ' ';
    os << (*act_it).asRDDL() << ';';
  }

  return os.str();
}

std::string ActionGroup::asPPDDL()
{
  ostringstream os;
  bool first = true;

  for(vector<Action>::iterator act_it = actions.begin(); act_it != actions.end(); act_it++)
  {
    if(first)
      first = false;
    else
      os << "___";
    os << (*act_it).asPPDDL();
  }

  return os.str();
}
