#ifndef SAMPLE_SIZE_H
#define SAMPLE_SIZE_H

#include "graph.h"

class Search;

int random_walk_sample_size(StateNode *start, double epsl);
int mclugSampleSize(DdNode *d, double delta, double epsl);

#endif
