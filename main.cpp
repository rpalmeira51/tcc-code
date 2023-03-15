#include <iostream>
#include <vector>
#include <memory>
#include <tuple> // for tuple
#include <map>
#include <bits/stdc++.h>
#include <limits.h>
#include <utility>
#include<algorithm>
#include "./data-structures/graph.h"
#include "./data-structures/globals.h"
#include "./utils/canonical_ordering.h" 
#include "./utils/combinations.h" 
#include "./utils/utils.h"
using namespace std;

// Calcula o custo para uma cor
unsigned CalculateCost(unsigned vertex)
{
    if (std::find(ClebschGraphObj.badVertices.begin(), ClebschGraphObj.badVertices.end(), vertex) != ClebschGraphObj.badVertices.end())
        return 1;
    else
        return 0;
}

// Calcula o custo para um vetor de cores
unsigned CalculateCost(vector<char> &colors)
{
    unsigned cost = 0;
    for (auto c : colors)
        cost += CalculateCost(c);
    return cost;
}

bool HasPossibleParentColors(vector<char> &levelColors, vector<vector<char>>& possibleColors){
    for (int i = 0; i < levelColors.size(); i += 2)
    {
        auto v = levelColors[i];
        auto u = levelColors[i+1];
        auto cn = PossibleChoicesCommonNeighbours(v, u);
        if (cn.size() == 0){
            return false;
        }
        possibleColors.push_back(cn);
    }
    return true;
}

unsigned BetterColoring(unsigned cost, vector<char> &leafColors)
{    
    if (leafColors.size() == 3){
        auto v =PossibleChoicesCommonNeighbours(leafColors[0], leafColors[1]);
        auto cn = VectorInstersection(v, ClebschGraphObj.adjLis[leafColors[2]].adjs);
        auto levelCost = CalculateCost(leafColors);
        unsigned minCost = cost-levelCost;
        for (auto p : cn)
        {
            minCost = min(minCost,CalculateCost(p));
        }
        return minCost+levelCost;
    }
    vector<vector<char>> possibleColors;
    if(!HasPossibleParentColors(leafColors, possibleColors)) return cost;
    auto levelCost = CalculateCost(leafColors);
    auto nextCost = cost - levelCost;
    if (levelCost > cost) return cost;
    unsigned minCost = nextCost;
    unsigned curCost;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    while (!combinationIterator.stop){
        auto pc = combinationIterator.GetNext();
        CanonicalOrdering(pc);
        if(coloringTable.find(pc) != coloringTable.end()){
            curCost = coloringTable[pc].second;
        }else{
            curCost = BetterColoring(nextCost, pc);
        }
        minCost = min(minCost,curCost);
    }
    return minCost + levelCost;
}

vector<Node *> GrowTree(vector<Node *> children)
{
    auto lastIndex = children[children.size()-1]->label;
    vector<Node *> newChildren;
    for (auto node : children)
    {
        auto lc = new Node();
        lc->parent = node;
        lc->label = lastIndex++;
        auto rc = new Node();
        rc->parent = node;
        rc->label = lastIndex++;
        node->leftChild = lc;
        node->rightChild = rc;
        newChildren.push_back(lc);
        newChildren.push_back(rc);
    }
    return newChildren;
}

// void print_tree(vector<Node *> children, map<unsigned, unsigned> &comb)
// {
//     vector<Node *> currLevel = children;
//     while (currLevel.size() > 0)
//     {
//         vector<Node *> nextLevel;
//         for (auto c : children)
//         {
//             ////////cout << c->label<< ":" << comb[c->label]<< " parent :"<< c->parent->label << "  |  ";
//             // ////cout << comb[c->label] << " ";
//             //  if(c->parent != NULL)
//             //      nextLevel.push_back(c->parent);
//         }
//         // ////cout << endl;
//         currLevel = nextLevel;
//     }
// }

// AnyBadColoringsOnLevel ?
bool TopDownOnTree(unsigned maxTreeLevel)
{
    RootedTree rtree(3);
    vector<vector<char>> badColorings;
    for (auto c : ClebschGraphObj.badVertices)
    {
        vector<char> initialColoring{c};
        badColorings.push_back(initialColoring);
        auto cost = CalculateCost(c);
        coloringTable[initialColoring] = make_pair(cost,cost);
    }
    unsigned level = 0;
    bool isFirst = true;
    auto children = rtree.children;
    while (badColorings.size() > 0 && level < maxTreeLevel)
    {
        vector<vector<char>> newbadColorings; 
        unsigned colorIndex = 0;
        int testindex = 0;
        unsigned totalgcCounter = 0;
        unsigned totalbcCounter = 0;
        unsigned totalcbcCounter =0;
        unsigned totalcCCounter =0;
        for (auto c : badColorings)
        {
            vector<vector<char>> possibleColors;
            CanonicalOrdering(c);
            auto fatherCost = coloringTable[c].first;
            if(children.size() ==3){
                for(int i =0; i< children.size(); i++){
                    auto child = children[i];
                    auto parentColor = c[0];
                    possibleColors.push_back(ClebschGraphObj.adjLis[parentColor].adjs);
                }
            }else {
                for(int i =0; i< children.size(); i++){
                    auto child = children[i];
                    auto parentColor = c[i/2];
                    possibleColors.push_back(ClebschGraphObj.adjLis[parentColor].adjs);
                }
            }
            CombinationIterator* combinationIterator;
            if(possibleColors.size() ==3){
                combinationIterator = new CombinationIteratorBottomUp(possibleColors);
            }else{
                combinationIterator = new CombinationIteratorTopDown(possibleColors);
            }
            unsigned gcCounter = 0;
            unsigned bcCounter = 0;
            unsigned cbcCounter =0;
            unsigned cCCounter = 0;
            while (!combinationIterator->stop)
            {
                auto comb = combinationIterator->GetNext();
                auto combCost = CalculateCost(comb);
                unsigned cost = fatherCost + combCost ;// Prestar atenção no custo
                CanonicalOrdering(comb); 
                if(coloringTable.find(comb) != coloringTable.end()){
                    auto cached = coloringTable[comb];
                    bool alreadyBadColoring = cached.first == cached.second;
                    if(cost < cached.first ){
                        cached = make_pair(cost, cached.second);
                        coloringTable[comb] = cached;
                        if(cost == cached.second){
                            //cbcCounter++;
                            newbadColorings.push_back(comb);
                        }
                    }
                    if( cost == cached.second){
                        bcCounter++;
                    }    
                    else 
                        gcCounter++;  
                }else {
                    auto result = BetterColoring(cost, comb); 
                    //cout << "cost: " << cost << result 
                    cCCounter++;
                    coloringTable[comb] = make_pair(cost, result);
                    if( cost == result){
                        bcCounter++;
                        cbcCounter++;
                        newbadColorings.push_back(comb);
                    }
                    else {
                        gcCounter++;
                    }
                }
            }
            delete combinationIterator;
            //cout<< "Custo do pai: "<< fatherCost <<" Com pais: " << c << endl;
            cout <<"Colorações canonicas ruins :" << cbcCounter << "  colorações canonicas" << cCCounter << endl; //" Colorações ruins : "<< bcCounter <<" Colarações boas : "<< gcCounter <<  " total:"<< gcCounter + bcCounter <<endl;
            testindex++;
            colorIndex++;
            totalgcCounter += gcCounter;
            totalbcCounter += bcCounter;
            totalcbcCounter+= cbcCounter;
            totalcCCounter += cCCounter;
            if(level == 2){
                return level == maxTreeLevel;
            }
        }
        cout <<"Colorações totais canonicas ruins :" << totalcbcCounter << "  colorações canonicas totais  "<<totalcCCounter << endl; //" Colorações ruins : "<< totalbcCounter <<" Colarações boas : "<< totalgcCounter <<  " total:"<< totalgcCounter + totalbcCounter <<endl;
        badColorings = newbadColorings; // use pointer
        children = GrowTree(children);
        level++;
        cout << level << endl;
    }
    return level == maxTreeLevel;
}


// Fazer essa otm 
// Revisão do código e otimizações gerais 
// Utilizar o banco de dados -> para guardar
int main()
{
    InitializeMatrix();
    InitializeParentPermutationMatrix();
    for(int i =0; i< 15; i++)
        cout << "(" << parentPermutationMatrix[i].first <<"," << parentPermutationMatrix[i].second <<")" << " ";
    cout << endl;
    TopDownOnTree(2);
    return 0;
}