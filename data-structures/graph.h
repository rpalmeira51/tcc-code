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
    vector<char> badVertices = {0};
    ClebschGraph();
};

#endif