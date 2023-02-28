#include <vector>
#include "graph.h"
#include <map>


// Matriz de adjacência que guarda os vizinhos em comuns para cada par de vértices
vector<char> adjMatrix[16][16];

// Objeto global do grafo de Clebsch
ClebschGraph ClebschGraphObj;

//Mapa que relaciona uma coloração canonica a um par 
// em que é o primeiro elemento é o custo da árvore 
// e o segundo é o melhor custo encontrado pelo BottomUp
map<vector<char>, pair<unsigned,unsigned>> coloringTable;

// Array que enumera os pares de combinação(de (0,0) ... (4,4)) de possíveis escolhas de filhos 
pair<int,int> parentPermutationMatrix[15];