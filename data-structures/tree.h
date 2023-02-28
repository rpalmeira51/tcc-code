#ifndef TREE_H
#define TREE_H
#include <map>
#include <iostream>
using namespace std;


class TreeColoring
{
public:
    map<unsigned, char> *leafColors;
    TreeColoring *nextLevel;
};
#endif