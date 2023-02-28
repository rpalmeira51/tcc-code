#include <vector>
#include <map>
#include "combinations.h"
#include "../data-structures/globals.h"
using namespace std;


//Inicializa a matriz de permutação
void InitializeParentPermutationMatrix(){
    int index =0;
    for(int i =0; i< 5 ; i++){
        for(int j = i; j<5; j++){
            parentPermutationMatrix[index] = make_pair(i,j);
            index++;
        }
    }
}

//Incrementa um vetor de escolha possíveis(14) e retorna se há algum novo incrmento possível
bool GetNextPermutationTopDown(vector<char> &permutationIndentifier)
{
    for (int i = permutationIndentifier.size() - 1; i >= 0; i--)
    {
        if (permutationIndentifier[i] < 14)
        { 
            permutationIndentifier[i]++;
            return true;
        }
        permutationIndentifier[i] = 0;
    }
    return false;
}

//Incrementa um vetor de escolha possíveis baseado em outro vetor que guarda o número de possibilidades
// e retorna se há algum novo incrmento possível
bool GetNextPermutation(vector<char> &vet, vector<char> &nc)
{
    for (int i = vet.size() - 1; i >= 0; i--)
    {
        if (vet[i] < nc[i] - 1)
        {
            vet[i]++;
            return true;
        }
        vet[i] = 0;
    }
    return false;
}


//Construtores dos iteradores
CombinationIterator::CombinationIterator(map<unsigned, vector<char>> &possibleColorsByVertexArg) : possibleColorsByVertex(possibleColorsByVertexArg) {
    n = possibleColorsByVertex.size();
}

CombinationIteratorBottomUp::CombinationIteratorBottomUp(map<unsigned, vector<char>> &possibleColorsByVertexArg): CombinationIterator(possibleColorsByVertexArg) {
    combinationChoices.resize(n);
    for (auto c : possibleColorsByVertex)
    {
        numberOfChoices.push_back(c.second.size());
    }
}

CombinationIteratorTopDown::CombinationIteratorTopDown(map<unsigned, vector<char>> &possibleColorsByVertexArg): CombinationIterator(possibleColorsByVertexArg){
    n = possibleColorsByVertex.size();
    combinationChoices.resize(n/2);
}

//A cada chamada, retorna uma possível coloração possível baseado no possibleColorsByVertex
map<unsigned, char> CombinationIteratorBottomUp::GetNext()
{
    map<unsigned, char> leafColoring;
    int i = 0;
    for (auto ps : possibleColorsByVertex)
    {
        leafColoring[ps.first] = ps.second[combinationChoices[i]];
        i++;
    }
    if (!GetNextPermutation(combinationChoices, numberOfChoices))
    {
        stop = true;
    }
    return leafColoring;
}

//A cada chamada, retorna uma possível coloração possível baseado no possibleColorsByVertex
// e na parentPermutationMatrix 
map<unsigned, char> CombinationIteratorTopDown::GetNext()
{
    map<unsigned, char> leafColoring;
    int i = 0;
    for (auto ps : possibleColorsByVertex)
    {
        auto pair = parentPermutationMatrix[combinationChoices[i/2]];
        int index = i%2==0 ? pair.first : pair.second;
        leafColoring[ps.first] = ps.second[index];
        i++;
    }
    if (!GetNextPermutationTopDown(combinationChoices))
    {
        stop = true;
    }
    return leafColoring;
}
