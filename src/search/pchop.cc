#include "pchop.h"

PCHOP::PCHOP(int numSamples, int maxDepth)
: SampledSearch(PCHOP_SEARCH), PCHOPAdvance(numSamples, maxDepth){
}
