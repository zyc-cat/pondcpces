#include "samplegen.h"
using namespace std;

// samplegen::samplegen(){}



StateFormula& samplegen::computeSingleCounterExample(std::vector<const Action *> candidateplan){
    try
    {
        if (!candidateplan.empty())
        {
            cout << "可以正常运行" << endl;
            return *tmp_counterexample;
        } 
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }   
}
    