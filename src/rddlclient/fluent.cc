#include "fluent.h"

#include <string>
#include <vector>
#include <sstream>

using namespace std;

Fluent::Fluent()
{
}

string Fluent::clean(string str)
{
  for(int i = 0; i < str.length(); i++)
    if(!isalnum(str[i]))
      str.replace(i, 1, 1, '_');

  return str;
}

std::string Fluent::asRDDL()
{
  ostringstream os;
  bool first = true;

  os << name << "(";
  for(vector<string>::iterator arg_it = args.begin(); arg_it != args.end(); arg_it++)
  {
    if(first)
      first = false;
    else
      os << ", ";
    os << (*arg_it);
  }
  os << ")";

  return os.str();
}

std::string Fluent::asPPDDL()
{
  ostringstream os;

  os << clean(name);
  if(!args.empty())
  {
    os << "_";
    for(vector<string>::iterator arg_it = args.begin(); arg_it != args.end(); arg_it++)
      os << "_" << clean(*arg_it);
  }

  return os.str();
}
