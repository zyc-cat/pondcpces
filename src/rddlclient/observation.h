#ifndef __OBSERVATION_H
#define __OBSERVATION_H

#include "fluent.h"

#include <string>

// Forward Declarations
class XMLNode;

class Observation : public Fluent
{
public:
  Observation(const XMLNode *atomNode);

  bool isTrue() { return value; }

protected:
  bool value;
};

#endif // __OBSERVATION_H
