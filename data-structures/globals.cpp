#include <vector>
#include "graph.h"
#include <map>
#include <unordered_map>
#include <atomic>

// Matriz de adjacência que guarda os vizinhos em comuns para cada par de vértices
vector<char> adjMatrix[16][16];

// Objeto global do grafo de Clebsch
//SuperClebschGraph ClebschGraphObj;
ClebschGraph ClebschGraphObj;

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
//Mapa que relaciona uma coloração canonica a um par 
// em que é o primeiro elemento é o custo da árvore 
// e o segundo é o melhor custo encontrado pelo BottomUp
unordered_map<vector<char>, pair<char, char>, VectorHasher> coloringTable;

// Array que enumera os pares de combinação(de (0,0) ... (4,4)) de possíveis escolhas de filhos 
pair<int,int> parentPermutationMatrix[15];