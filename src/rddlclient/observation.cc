#include "observation.h"

#include "strxml.h"

using namespace std;

// Extracts an observation from the given XML node
Observation::Observation(const XMLNode *atomNode)
: Fluent(), value(false)
{
  if(atomNode == NULL || atomNode->getName() != "observed-fluent")
    return;

  // Get fluent name
  if(!atomNode->dissect("fluent-name", name))
    return;

  // Get fluent arguments and value
  for(int i = 0; i < atomNode->size(); i++)
  {
    const XMLNode *termNode = atomNode->getChild(i);
    if(termNode != NULL)
    {
      if(termNode->getName() == "fluent-arg")
        args.push_back(termNode->getText());
      else if(termNode->getName() == "fluent-value")
        value = (termNode->getText() == "true");
    }
  }
}
