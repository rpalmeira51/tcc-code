#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <cstdint>
using namespace std;
uint8_t CalculateCostVertex(unsigned vertex);
uint8_t CalculateCostEdges(vector<char> const &parentColors, vector<char> const &colors);
uint8_t CalculateCostVertex(vector<char> const &colors);

class Vertice
{
public:
    unsigned index;
    unsigned color;
    vector<char> adjs;
};

class Node
{
public:
    Node *leftChild;
    Node *rightChild;
    Node *parent;
    unsigned label;
};

class RootedTree
{
public:
    Node *root;
    vector<Node *> children;
    RootedTree(int n);
};

class Graph
{
public:
    vector<Vertice> adjLis;
};

class ClebschGraph : public Graph
{
public:
    //vector<char> badVertices = {0,1,2,7,12};
    vector<char> badVertices = {0};
    char badVertice = 0;
    vector<char> badEdge = {0,1};  
    ClebschGraph();
    uint8_t EdgeCost(unsigned u, unsigned v);
    bool AreAdjacent(unsigned u, unsigned v);
};

class SuperClebschGraph : public ClebschGraph
{
public:
    SuperClebschGraph();
};

#endif