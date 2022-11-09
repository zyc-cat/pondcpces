#ifndef __ACTION_H
#define __ACTION_H

#include "fluent.h"

#include <string>
#include <vector>

class Action : public Fluent
{
public:
  Action(const std::string &ppddlAct);

  std::string asXML();

protected:
};

#endif // __ACTION_H
