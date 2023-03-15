#include <vector>
#include <utility>
#include <bits/stdc++.h>
#include "../data-structures/globals.h"
#include "../data-structures/graph.h"

using namespace std;
extern vector<char> adjMatrix[16][16];
extern ClebschGraph ClebschGraphObj;

ostream& operator<<(ostream& os, const vector<char>& vec)
{
    for(auto e:vec){
        os << (int)e << " ,";
    }
    return os;
}

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& vec)
{
    for(auto e:vec){
        os << e << " ,";
    }
    return os;
}


vector<char> VectorInstersection(vector<char> &nv, vector<char> &nu)
{
    vector<char> v3;
    std::sort(nv.begin(), nv.end());
    std::sort(nu.begin(), nu.end());

    std::set_intersection(nv.begin(), nv.end(),
                          nu.begin(), nu.end(),
                          back_inserter(v3));
    return v3;
}

// Inicializa a adjMatrix
void InitializeMatrix()
{
    for (int i = 0; i < ClebschGraphObj.adjLis.size(); i++)
    {
        for (int j = 0; j < ClebschGraphObj.adjLis.size(); j++)
        {
            auto nv = ClebschGraphObj.adjLis[i].adjs;
            auto nu = ClebschGraphObj.adjLis[j].adjs;
            adjMatrix[i][j] = VectorInstersection(nv, nu);
        }
    }
}

vector<char> PossibleChoicesCommonNeighbours(unsigned v, unsigned u)
{
    return adjMatrix[v][u];
}