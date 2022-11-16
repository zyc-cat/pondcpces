#ifndef SAMPLEGEN_H
#define SAMPLEGEN_H

#include "actions.h"

class samplegen{
private:
    StateFormula *tmp_counterexample;
public:
    // samplegen();
    // ~samplegen();
    // bool refineCPCESSample(std::vector<const Action*> candidateplan);

    StateFormula& computeSingleCounterExample(std::vector<const Action *> candidateplan);
};

#endif