#ifndef CANONICAL_H
#define CANONICAL_H
#include <vector>
#include <iostream>
using namespace std;
bool isLexisGE(vector<char>& coloring, unsigned pace_size, int fele_index);
void swapPartsOfTrees(vector<char>& coloring, unsigned pace_size, int fele_index);
void CanonicalOrderingVertices(vector<char>& coloring);
void CanonicalOrderingEdges(vector<char>& coloring);
void TestLex(vector<char> &coloring);
#endif