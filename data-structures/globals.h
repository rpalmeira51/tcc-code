#ifndef GLOBALS_H
#define GLOBALS_H
#include <vector>
#include "graph.h"
#include <unordered_map>

using namespace std;

extern vector<char> adjMatrix[16][16];
//extern ClebschGraph ClebschGraphObj;
extern SuperClebschGraph ClebschGraphObj;
extern pair<int,int> parentPermutationMatrix[15];
struct VectorHasher
{
    typedef vector<char> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& vec) const
    {
        std::size_t seed = vec.size();
        for(auto& i : vec) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
// Min TopDownCost e Min BottomUp Cost->Not for now
// Min TopDownCost <= Min BottomUp Cost -> Ruim 
// Testar com igualdade 
extern unordered_map<vector<char>, pair<char, char>, VectorHasher> coloringTable;
#endif