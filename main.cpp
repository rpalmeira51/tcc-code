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
#include "./data-structures/matrix.h"
#include "./utils/canonical_ordering.h" 
#include "./utils/combinations.h" 
#include "./utils/utils.h"
using namespace std;


bool HasCachedValue(vector<char> comb, pair<unsigned, unsigned>& costs);





// Calcula o custo para uma cor
unsigned CalculateCost(unsigned vertex)
{
    if (std::find(ClebschGraphObj.badVertices.begin(), ClebschGraphObj.badVertices.end(), vertex) != ClebschGraphObj.badVertices.end())
        return 1;
    else
        return 0;
}

// Calcula o custo para um vetor de cores
unsigned CalculateCost(vector<char> const &colors)
{
    unsigned cost = 0;
    for (auto c : colors)
        cost += CalculateCost(c);
    return cost;
}

bool HasPossibleParentColors(vector<char> const &levelColors, vector<vector<char>>& possibleColors){
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

struct CacheInfo{
    int CacheHit;
    int CacheMiss;
};

unsigned BetterColoring(unsigned cost, vector<char> const &leafColors, CacheInfo& cacheInfo ,bool useCache =true)
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
    if (levelCost >= cost) return cost;

    auto nextCost = cost - levelCost;
    unsigned minCost = nextCost;
    unsigned curCost;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);

    while (!combinationIterator.stop){
        auto pc = combinationIterator.GetNext();
        pair<unsigned, unsigned> costs;
        CanonicalOrdering(pc);
        if(useCache && HasCachedValue(pc,costs)){
            curCost = costs.second;
            cacheInfo.CacheHit++;
        }else {
            curCost = BetterColoring(nextCost, pc, cacheInfo,useCache);
            cacheInfo.CacheMiss++;
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


bool HasCanonicalOrderingOnTable(const vector<char>& comb, pair<unsigned, unsigned>& costs){
    if(coloringTable.find(comb) != coloringTable.end()){
        costs = coloringTable[comb];
        return true;
    }
    return false;
}

bool HasCachedValue(vector<char> comb, pair<unsigned, unsigned>& costs){
    if(HasCanonicalOrderingOnTable(comb,costs)) return true;
    for(int i=0; i< 119; i++){
        auto m_index = (i)*16;
        for(int j=0; j< 15; j++){
            comb[j] = autoMorphismMatrix[m_index+comb[j]];
        }
        CanonicalOrdering(comb);
        if(HasCanonicalOrderingOnTable(comb,costs)) return true;
    }
    return false;
}


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
                // for(int i =0; i< children.size(); i++){
                //     auto child = children[i];
                //     auto parentColor = c[0];
                //     possibleColors.push_back(ClebschGraphObj.adjLis[parentColor].adjs);
                // }
                possibleColors = {{1},{6}, {10}};
            }else {
                for(int i =0; i< children.size(); i++){
                    auto child = children[i];
                    auto parentColor = c[i/2];
                    possibleColors.push_back(ClebschGraphObj.adjLis[parentColor].adjs);
                }
            }
            CombinationIterator* combinationIterator;
            if(level == 2){
                cout << "pc size: " <<possibleColors.size() << endl;
                for(auto v: possibleColors)
                    cout << v << endl;
            }
            if(possibleColors.size() ==3){
                combinationIterator = new CombinationIteratorBottomUp(possibleColors);
            }else{
                combinationIterator = new CombinationIteratorTopDown(possibleColors);
            }
            unsigned gcCounter = 0;
            unsigned bcCounter = 0;
            unsigned cgcCounter = 0;
            unsigned cbcCounter =0;
            unsigned cCCounter = 0;
            unsigned cacheHit = 0;
            unsigned cacheMiss = 0;
            unsigned index = 0;
            // cout << "Size: " << possibleColors.size() << " Total possibilities" << pow(5,possibleColors.size())<< endl;
            vector<vector<char>> badCanonicalColeringsToPrint;
            vector<vector<char>> goodCanonicalColeringsToPrint;
            CacheInfo cacheInfo;
            cacheInfo.CacheHit = cacheInfo.CacheMiss = 0;
            while (!combinationIterator->stop)
            {
                // cout << "Index : "<< ((index++)*100)/pow(5,possibleColors.size()) << "%" << endl;
                auto comb = combinationIterator->GetNext();
                auto combCost = CalculateCost(comb);
                unsigned cost = fatherCost + combCost ;// Prestar atenção no custo
                pair<unsigned, unsigned> cached;
                CanonicalOrdering(comb);
                if(HasCachedValue(comb,cached)){
                    bool alreadyBadColoring = cached.first == cached.second;
                    if(cost < cached.first ){
                        cached = make_pair(cost, cached.second);
                        coloringTable[comb] = cached;
                        if(cost == cached.second && !alreadyBadColoring){
                            newbadColorings.push_back(comb);
                            cbcCounter++;
                            gcCounter--;
                        }    
                    }
                    cacheHit++;
                    if(cost == cached.second)
                        bcCounter++;
                    else 
                        gcCounter++;  
                }else {
                    cacheMiss++;
                    auto result = BetterColoring(cost, comb, cacheInfo); 
                    auto leafsCost = CalculateCost(comb);
                    cCCounter++;
                    coloringTable[comb] = make_pair(cost, result);
                    if( cost == result){
                        bcCounter++;
                        cbcCounter++;
                        badCanonicalColeringsToPrint.push_back(comb);
                        newbadColorings.push_back(comb);
                    }
                    else {
                        goodCanonicalColeringsToPrint.push_back(comb);
                        gcCounter++;
                        cgcCounter++;
                    }
                }
                // if(index == 10000) break;
            }
            delete combinationIterator;
            cout << "TopDown CH: " << cacheHit << " CM:" <<cacheMiss<< endl;
            cout << "BetterColoring CH: " << cacheInfo.CacheHit << " CM:" <<cacheInfo.CacheMiss<< endl;
            //cout<< "Custo do pai: "<< fatherCost <<" Com pais: " << c << endl;
            cout <<"Colorações canonicas ruins :" << cbcCounter << " colorações canonicas boas " << cgcCounter << "  colorações canonicas " << cCCounter << endl;
            cout<<"+++++++++++++++++++++++++++++++++"<<endl;
            cout << "Bad colorings, total: "<< badCanonicalColeringsToPrint.size() <<endl;
            for(auto c : badCanonicalColeringsToPrint)
                cout << "[" << c << "]" << endl;
            cout << "============================="<< endl;
            cout<< "Good Colorings, total: " << goodCanonicalColeringsToPrint.size() << endl;
            for(auto c : goodCanonicalColeringsToPrint)
                cout << "[" << c << "]" << endl;
            cout<<"+++++++++++++++++++++++++++++++++"<<endl;
            testindex++;
            colorIndex++;
            totalgcCounter += gcCounter;
            totalbcCounter += bcCounter;
            totalcbcCounter+= cbcCounter;
            totalcCCounter += cCCounter;
            // if(level == 2){
            //     return level == maxTreeLevel;
            // }
        }
        // cout <<"Colorações totais canonicas ruins :" << totalcbcCounter << "  colorações canonicas totais  "<<totalcCCounter << endl; //" Colorações ruins : "<< totalbcCounter <<" Colarações boas : "<< totalgcCounter <<  " total:"<< totalgcCounter + totalbcCounter <<endl;
        // cout << "Bad colorings " << newbadColorings.size() << "  " << endl;
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

// int main(){
//     vector<vector<char>> possibleColors { {1,2,3,4,5}, {1,2,3,4,5}, {1,2,3,4,5},{1,2,3,4,5}, {1,2,3,4,5}, {1,2,3,4,5} };
//     auto combinationIterator = new CombinationIteratorBottomUp(possibleColors);
//     int totalPerm = 0;
//     while (!combinationIterator->stop){
//         auto comb = combinationIterator->GetNext();
//         totalPerm++;
//     }
//     cout<<"===================" << endl;
//     cout << totalPerm << endl;
// }