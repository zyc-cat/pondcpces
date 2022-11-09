#ifndef __FLUENT_H
#define __FLUENT_H

#include <string>
#include <vector>

class Fluent
{
public:

  std::string asRDDL();
  std::string asPPDDL();

protected:
  Fluent();

  std::string clean(std::string str);

  std::string name;
  std::vector<std::string> args;
};

#endif // __FLUENT_H
