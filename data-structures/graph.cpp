#include <vector>
#include "graph.h"
using namespace std;


//Função auxiliar que cria um vértice com a lista de adjacência preenchida com índices(dos vizinhos) passados no argumento
Vertice fromInts(vector<int> vertices, int index)
{
    Vertice v;
    v.index = index;
    v.color = -1;
    for (int i = 0; i < vertices.size(); i++)
        v.adjs.push_back(vertices[i]);
    return v;
}


ClebschGraph:: ClebschGraph(){
    adjLis.push_back(fromInts({1, 10, 6, 8, 13}, 0));
    adjLis.push_back(fromInts({0, 3, 4, 9, 15}, 1));
    adjLis.push_back(fromInts({3, 10, 8, 12, 15}, 2));
    adjLis.push_back(fromInts({1, 2, 6, 5, 11}, 3));
    adjLis.push_back(fromInts({1, 10, 5, 12, 14}, 4));
    adjLis.push_back(fromInts({3, 4, 8, 7, 13}, 5));
    adjLis.push_back(fromInts({0, 3, 7, 12, 14}, 6));
    adjLis.push_back(fromInts({10, 6, 5, 9, 15}, 7));
    adjLis.push_back(fromInts({0, 2, 5, 9, 14}, 8));
    adjLis.push_back(fromInts({1, 8, 7, 11, 12}, 9));
    adjLis.push_back(fromInts({0, 2, 4, 7, 11}, 10));
    adjLis.push_back(fromInts({3, 10, 9, 13, 14}, 11));
    adjLis.push_back(fromInts({2, 4, 6, 9, 13}, 12));
    adjLis.push_back(fromInts({0, 5, 11, 12, 15}, 13));
    adjLis.push_back(fromInts({4, 6, 8, 11, 15}, 14));
    adjLis.push_back(fromInts({1, 2, 7, 13, 14}, 15));
}