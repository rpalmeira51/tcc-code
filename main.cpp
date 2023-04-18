#include <iostream>
#include <vector>
#include <memory>
#include <tuple> // for tuple
#include <map>
#include <bits/stdc++.h>
#include <limits.h>
#include <utility>
#include <algorithm>
#include "./data-structures/graph.h"
#include "./data-structures/globals.h"
#include "./data-structures/matrix.h"
#include "./utils/canonical_ordering.h"
#include "./utils/combinations.h"
#include "./utils/utils.h"
#include <thread>
using namespace std;

bool HasCachedValue(vector<char> comb, pair<unsigned, unsigned> &costs);
vector<thread> threadpool;
const auto processor_count = thread::hardware_concurrency() == 0 ? 8: thread::hardware_concurrency() ;
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

bool HasPossibleParentColors(vector<char> const &levelColors, vector<vector<char>> &possibleColors)
{
    for (int i = 0; i < levelColors.size(); i += 2)
    {
        auto v = levelColors[i];
        auto u = levelColors[i + 1];
        auto cn = PossibleChoicesCommonNeighbours(v, u);
        if (cn.size() == 0)
        {
            return false;
        }
        possibleColors.push_back(cn);
    }
    return true;
}

struct CacheInfo
{
    int CacheHit;
    int CacheMiss;
};

struct CounterInfo
{
    unsigned gcCounter;
    unsigned bcCounter;
    unsigned cgcCounter;
    unsigned cbcCounter;
    unsigned cCCounter;
    unsigned cacheHit;
    unsigned cacheMiss;
    unsigned index;
};
mutex tableLocker;

bool HasCanonicalOrderingOnTable(const vector<char> &comb, pair<unsigned, unsigned> &costs)
{
    tableLocker.lock();
    if (coloringTable.find(comb) != coloringTable.end())
    {
        costs = coloringTable[comb];
        tableLocker.unlock();
        return true;
    }
    tableLocker.unlock();
    return false;
}

bool HasCachedValue(vector<char> comb, pair<unsigned, unsigned> &costs)
{
    if (HasCanonicalOrderingOnTable(comb, costs))
        return true;
    for (int i = 0; i < 119; i++)
    {
        auto m_index = (i)*16;
        for (int j = 0; j < 15; j++)
        {
            comb[j] = autoMorphismMatrix[m_index + comb[j]];
        }
        CanonicalOrdering(comb);
        if (HasCanonicalOrderingOnTable(comb, costs))
            return true;
    }
    return false;
}

unsigned BetterColoring(unsigned cost, vector<char> const &leafColors, CacheInfo &cacheInfo, bool useCache = true)
{
    if (leafColors.size() == 3)
    {
        auto v = PossibleChoicesCommonNeighbours(leafColors[0], leafColors[1]);
        auto cn = VectorInstersection(v, ClebschGraphObj.adjLis[leafColors[2]].adjs);
        auto levelCost = CalculateCost(leafColors);
        unsigned minCost = cost - levelCost;
        for (auto p : cn)
        {
            minCost = min(minCost, CalculateCost(p));
        }
        return minCost + levelCost;
    }
    vector<vector<char>> possibleColors;
    if (!HasPossibleParentColors(leafColors, possibleColors))
        return cost;
    auto levelCost = CalculateCost(leafColors);
    if (levelCost >= cost)
        return cost;

    auto nextCost = cost - levelCost;
    unsigned minCost = nextCost;
    unsigned curCost;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);

    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNext();
        pair<unsigned, unsigned> costs;
        CanonicalOrdering(pc);
        if (useCache && HasCachedValue(pc, costs))
        {
            curCost = costs.second;
            cacheInfo.CacheHit++;
        }
        else
        {
            curCost = BetterColoring(nextCost, pc, cacheInfo, useCache);
            cacheInfo.CacheMiss++;
        }
        minCost = min(minCost, curCost);
    }
    return minCost + levelCost;
}

vector<Node *> GrowTree(vector<Node *> children)
{
    auto lastIndex = children[children.size() - 1]->label;
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

mutex badColoringsLocker;
mutex cacheLocker;


void TryImproveBadColoring(vector<char> &comb,
                           unsigned fatherCost, vector<vector<char>> &newbadColorings, 
                           CacheInfo &cacheInfo, CounterInfo& ci)
{

    auto combCost = CalculateCost(comb);
    unsigned cost = fatherCost + combCost; // Prestar atenção no custo
    pair<unsigned, unsigned> cached;
    CanonicalOrdering(comb);
    if (HasCachedValue(comb, cached))
    {
        bool alreadyBadColoring = cached.first == cached.second;
        if (cost < cached.first)
        {
            cached = make_pair(cost, cached.second);
            tableLocker.lock();
            coloringTable[comb] = cached;
            tableLocker.unlock();
            cacheLocker.lock();
            if (cost == cached.second && !alreadyBadColoring)
            {
                badColoringsLocker.lock();
                newbadColorings.push_back(comb);
                badColoringsLocker.unlock();
                ci.cbcCounter++;
                ci.gcCounter--;
            }
            cacheLocker.unlock();
        }
        cacheLocker.lock();
        ci.cacheHit++;
        if (cost == cached.second)
            ci.bcCounter++;
        else
            ci.gcCounter++;
        cacheLocker.unlock();
    }
    else
    {
        auto result = BetterColoring(cost, comb, cacheInfo);
        tableLocker.lock();
        coloringTable[comb] = make_pair(cost, result);
        tableLocker.unlock();
        cacheLocker.lock();
        ci.cacheMiss++;
        ci.cCCounter++;
        if (cost == result)
        {
            ci.bcCounter++;
            ci.cbcCounter++;
            // badCanonicalColeringsToPrint.push_back(comb);
            badColoringsLocker.lock();
            newbadColorings.push_back(comb);
            badColoringsLocker.unlock();
        }
        else
        {
            // goodCanonicalColeringsToPrint.push_back(comb);
            ci.gcCounter++;
            ci.cgcCounter++;
        }
        cacheLocker.unlock();
    }
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
        //Desnecessário
        tableLocker.lock();
        coloringTable[initialColoring] = make_pair(cost, cost);
        tableLocker.unlock();
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
        unsigned totalcbcCounter = 0;
        unsigned totalcCCounter = 0;
        for (auto c : badColorings)
        {
            vector<vector<char>> possibleColors;
            CanonicalOrdering(c);
            tableLocker.lock();
            auto fatherCost = coloringTable[c].first;
            tableLocker.unlock();
            if (children.size() == 3)
            {
                // for(int i =0; i< children.size(); i++){
                //     auto child = children[i];
                //     auto parentColor = c[0];
                //     possibleColors.push_back(ClebschGraphObj.adjLis[parentColor].adjs);
                // }
                possibleColors = {{1}, {6}, {10}};
            }
            else
            {
                for (int i = 0; i < children.size(); i++)
                {
                    auto child = children[i];
                    auto parentColor = c[i / 2];
                    possibleColors.push_back(ClebschGraphObj.adjLis[parentColor].adjs);
                }
            }
            CombinationIterator *combinationIterator;
            if (level == 2)
            {
                cout << "pc size: " << possibleColors.size() << endl;
                for (auto v : possibleColors)
                    cout << v << endl;
            }
            if (possibleColors.size() == 3)
            {
                combinationIterator = new CombinationIteratorBottomUp(possibleColors);
            }
            else
            {
                combinationIterator = new CombinationIteratorTopDown(possibleColors);
            }
            unsigned index = 0;
            // cout << "Size: " << possibleColors.size() << " Total possibilities" << pow(5,possibleColors.size())<< endl;
            vector<vector<char>> badCanonicalColeringsToPrint;
            vector<vector<char>> goodCanonicalColeringsToPrint;
            CacheInfo cacheInfo;
            CounterInfo counterInfo{0,0,0,0,0,0,0,0};
            cacheInfo.CacheHit = cacheInfo.CacheMiss = 0;
            int thIndex = 1;
            vector<thread> threads;
            while (!combinationIterator->stop)
            {
                auto comb = combinationIterator->GetNext();
                cout << "ALOHA" << endl;
                if(thIndex < processor_count){
                    threads.push_back(thread(TryImproveBadColoring,ref(comb),fatherCost,ref(newbadColorings),ref(cacheInfo), ref(counterInfo)));
                    thIndex++;
                }else {
                    for(auto &th: threadpool)
                        th.join();
                    threads.clear();
                }
                cout << "FLAAAG" << endl;
            }
            delete combinationIterator;
            cout << "TopDown CH: " << counterInfo.cacheHit << " CM:" << counterInfo.cacheMiss << endl;
            cout << "BetterColoring CH: " << cacheInfo.CacheHit << " CM:" << cacheInfo.CacheMiss << endl;
            // cout<< "Custo do pai: "<< fatherCost <<" Com pais: " << c << endl;
            cout << "Colorações canonicas ruins :" << counterInfo.cbcCounter << " colorações canonicas boas " << counterInfo.cgcCounter << "  colorações canonicas " << counterInfo.cCCounter << endl;
            cout << "+++++++++++++++++++++++++++++++++" << endl;
            cout << "Bad colorings, total: " << badCanonicalColeringsToPrint.size() << endl;
            for (auto c : badCanonicalColeringsToPrint)
                cout << "[" << c << "]" << endl;
            cout << "=============================" << endl;
            cout << "Good Colorings, total: " << goodCanonicalColeringsToPrint.size() << endl;
            for (auto c : goodCanonicalColeringsToPrint)
                cout << "[" << c << "]" << endl;
            cout << "+++++++++++++++++++++++++++++++++" << endl;
            testindex++;
            colorIndex++;
            totalgcCounter += counterInfo.gcCounter;
            totalbcCounter += counterInfo.bcCounter;
            totalcbcCounter += counterInfo.cbcCounter;
            totalcCCounter += counterInfo.cCCounter;
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

