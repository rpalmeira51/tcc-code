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
    map<unsigned, vector<char>> &possibleColorsByVertex;
    vector<char> combinationChoices;
    bool stop = false;
    CombinationIterator(map<unsigned, vector<char>> &possibleColorsByVertexArg);
    virtual map<unsigned, char> GetNext() =0;
    // ~CombinationIterator();
};


// Possível coloração -> baseado em uma mapa de adjacencias 
// Agnostica de TopDown ou BottomUp -> para lógica principal
class CombinationIteratorTopDown: public CombinationIterator
{
public:
    CombinationIteratorTopDown(map<unsigned, vector<char>> &possibleColorsByVertexArg);
    void InitializeParentPermutationMatrix();
    map<unsigned, char> GetNext() override;
};


class CombinationIteratorBottomUp: public CombinationIterator
{
public:
    vector<char> numberOfChoices;
    CombinationIteratorBottomUp(map<unsigned, vector<char>> &possibleColorsByVertexArg);
    map<unsigned, char> GetNext() override;
};

void InitializeParentPermutationMatrix();
#endif