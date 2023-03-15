#ifndef COMBINATIONS_H
#define COMBINATIONS_H
#include <vector>
#include <iostream>
#include <map>
using namespace std;
bool GetNextPermutationTopDown(vector<char> &permutationIndentifier);
bool GetNextPermutation(vector<char> &vet, vector<char> &nc);


class CombinationIterator
{
public:
    unsigned n;
    vector<vector<char>> &possibleColors;
    vector<char> combinationChoices;
    bool stop = false;
    CombinationIterator(vector<vector<char>>&possibleColorsByVertexArg);
    virtual vector<char> GetNext() =0;
    // ~CombinationIterator();
};


// Possível coloração -> baseado em uma mapa de adjacencias 
// Agnostica de TopDown ou BottomUp -> para lógica principal
class CombinationIteratorTopDown: public CombinationIterator
{
public:
    CombinationIteratorTopDown(vector<vector<char>>&possibleColorsByVertexArg);
    void InitializeParentPermutationMatrix();
    vector<char> GetNext() override;
};


class CombinationIteratorBottomUp: public CombinationIterator
{
public:
    vector<char> numberOfChoices;
    CombinationIteratorBottomUp(vector<vector<char>> &possibleColorsByVertexArg);
    vector<char> GetNext() override;
};

void InitializeParentPermutationMatrix();
#endif