#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
using namespace std;

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
    Node *leftChild;
    Node *rightChild;
    Node *middleChild;
};

class Graph
{
public:
    vector<Vertice> adjLis;
};

class ClebschGraph : public Graph
{
public:
    vector<char> badVertices = {0, 1, 2, 5, 7};
    ClebschGraph();
};

#endif