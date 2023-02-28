#ifndef GLOBALS_H
#define GLOBALS_H
#include <vector>
#include "graph.h"
#include <map>

using namespace std;

extern vector<char> adjMatrix[16][16];
extern ClebschGraph ClebschGraphObj;
extern pair<int,int> parentPermutationMatrix[15];

// Min TopDownCost e Min BottomUp Cost->Not for now
// Min TopDownCost <= Min BottomUp Cost -> Ruim 
// Testar com igualdade 
extern map<vector<char>, pair<unsigned,unsigned>> coloringTable;
#endif