#include <vector>
#include <bits/stdc++.h>
#include "../data-structures/matrix.h"
#include "../data-structures/matrix_edges.h"
#include "../utils/utils.h"
using namespace std;

// 4,7,4,9, 9,7,5,7
// ^ ^      ^ ^
// Dada um vetor determina se para determinado passo o valor no índice dado é menor ou igual
// lexigograficamente que o próximo(compara primeiro o primeiro elemento, depois o segundo e assim por diante)
bool isLexisGE(vector<char> &coloring, unsigned pace_size, int fele_index)
{
    int index = 0;
    while (pace_size > index)
    {
        auto temp = fele_index + index;
        if (coloring[temp] < coloring[temp + pace_size])
        {
            return false;
        }
        else if (coloring[temp] > coloring[temp + pace_size])
        {
            return true;
        }
        index++;
    }
    return false;
}

// Dada uma coloração e um passo, inverte o valor(elemento a elemento) no índice dado
// com o próximo
void swapPartsOfTrees(vector<char> &coloring, unsigned pace_size, int fele_index)
{
    int index = 0;
    while (pace_size > index)
    {
        auto current_ele_i = fele_index + index;
        auto temp = coloring[current_ele_i];
        coloring[current_ele_i] = coloring[current_ele_i + pace_size];
        coloring[current_ele_i + pace_size] = temp;
        index++;
    }
}

class SpecifLexicographicOrdering
{
public:
    virtual void LexicographicOrdering(vector<char> &coloring) = 0;
};

class EdgesLexicographicOrdering : public SpecifLexicographicOrdering
{
public:
    void LexicographicOrdering(vector<char> &coloring) override
    {
        auto cor_size = coloring.size();
        if (cor_size == 1)
            return;
        unsigned levels = log2(cor_size);
        unsigned pace_size = 1;
        while (levels > 0)
        {
            unsigned index = 0;
            while (index + pace_size < cor_size)
            {
                if (isLexisGE(coloring, pace_size, index))
                {
                    swapPartsOfTrees(coloring, pace_size, index);
                }
                index += 2 * pace_size;
            }
            levels--;
            pace_size *= 2;
        }
    }
};

class VerticeLexicographicOrdering : public SpecifLexicographicOrdering
{
public:
    void LexicographicOrdering(vector<char> &coloring) override
    {
        auto cor_size = coloring.size();
        if (cor_size == 1)
            return;
        unsigned adjSize = (cor_size / 3) * 2;
        unsigned levels = log2(adjSize);
        unsigned pace_size = 1;
        while (levels > 1)
        {
            unsigned index = 0;
            while (index + pace_size < cor_size)
            {
                if (isLexisGE(coloring, pace_size, index))
                {
                    swapPartsOfTrees(coloring, pace_size, index);
                }
                index += 2 * pace_size;
            }
            levels--;
            pace_size *= 2;
        }
        unsigned findex = 0;
        unsigned sindex = findex + pace_size;
        if (isLexisGE(coloring, pace_size, findex))
        {
            swapPartsOfTrees(coloring, pace_size, findex);
        }
        if (isLexisGE(coloring, pace_size, sindex))
        {
            swapPartsOfTrees(coloring, pace_size, sindex);
        }
        if (isLexisGE(coloring, pace_size, findex))
        {
            swapPartsOfTrees(coloring, pace_size, findex);
        }
    }
};


bool isLexisG(vector<char> v1, vector<char> v2)
{
    for (int i = 0; i < v1.size(); i++)
    {
        if (v1[i] > v2[i])
            return true;
        if (v1[i] < v2[i])
            return false;
    }
    return false;
}


//0
// 1 2

//0 
// 2 1 

void GenericCanonicalOrdering(vector<char> &coloring, char *matrix, int numberAutomorphisms, SpecifLexicographicOrdering& slo)
{
    slo.LexicographicOrdering(coloring);
    vector<char> tempRepresentative = coloring;
    vector<char> candidate = coloring;
    for (int i = 0; i < numberAutomorphisms; i++)
    {
        auto m_index = (i)*16;
        for (int j = 0; j < 16; j++)
        {
            candidate[j] = matrix[m_index + coloring[j]];
        }
        slo.LexicographicOrdering(candidate);
        if (isLexisG(candidate, tempRepresentative))
        {
            //cout<< "+++++++==" <<endl;
            //cout << candidate << endl;
            //cout << tempRepresentative << endl;
            //cout<< "+++++++==" <<endl;
            tempRepresentative = candidate;
        }
    }
    coloring = tempRepresentative;
}

void TestLex(vector<char> &coloring){
    EdgesLexicographicOrdering sf;
    sf.LexicographicOrdering(coloring);
}

void CanonicalOrderingEdges(vector<char> &coloring)
{
    EdgesLexicographicOrdering sf;
    GenericCanonicalOrdering(coloring, autoEdgesMorphismMatrix, 48, sf);
}

void CanonicalOrderingVertices(vector<char> &coloring)
{
    VerticeLexicographicOrdering sf;
    GenericCanonicalOrdering(coloring, autoMorphismMatrix, 120, sf);
}