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
#include "./utils/canonical_ordering.h"
#include "./utils/combinations.h"
#include "./utils/utils.h"
#include <thread>
using namespace std;

// const auto processor_count = thread::hardware_concurrency() == 0 ? 8 : thread::hardware_concurrency();
const auto processor_count = 1;
//   Calcula o custo para uma cor
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
    atomic<int> CacheHit;
    atomic<int> CacheMiss;
};

struct CounterInfo
{
    atomic<unsigned> gcCounter;
    atomic<unsigned> bcCounter;
    atomic<unsigned> cgcCounter;
    atomic<unsigned> cbcCounter;
    atomic<unsigned> cCCounter;
    atomic<unsigned> cacheHit;
    atomic<unsigned> cacheMiss;
    atomic<unsigned> index;
};
mutex tableLocker;

bool UnsafeHasCachedValue(const vector<char> &comb, pair<unsigned, unsigned> &costs)
{
    if (coloringTable.find(comb) != coloringTable.end() && coloringTable[comb].first != -1)
    {
        costs = coloringTable[comb];
        return true;
    }
    return false;
}

bool UnsafeHasDirtyValue(const vector<char> &comb)
{
    if (coloringTable.find(comb) != coloringTable.end() && coloringTable[comb].first == -1)
        return true;
    return false;
}

// Checa se tem na tabela uma colocação automorfa
//  vector<char> UnsafeHasCachedValue(vector<char> comb, pair<unsigned, unsigned> &costs, bool& hasValue)
//  {
//      if (UnsafeHasCanonicalOrderingOnTable(comb, costs))
//          return comb;
//      for (int i = 0; i < 119; i++)
//      {
//          auto m_index = (i)*16;
//          for (int j = 0; j < 15; j++)
//          {
//              comb[j] = autoMorphismMatrix[m_index + comb[j]];
//          }
//          CanonicalOrdering(comb);
//          if(UnsafeHasCanonicalOrderingOnTable(comb, costs)){
//              hasValue= true;
//              return comb;
//          }
//      }
//      return comb;
//  }

// Todo remove useCache
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
        tableLocker.lock();
        if (UnsafeHasCachedValue(pc, costs))
        {
            tableLocker.unlock();
            curCost = costs.second;
            cacheInfo.CacheHit++;
        }
        else
        {
            tableLocker.unlock();
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

void UnsafeUpdateTable(vector<char> &comb, unsigned cost, pair<unsigned, unsigned> cached,
                       vector<vector<char>> &newbadColorings,
                       CacheInfo &cacheInfo, CounterInfo &ci)
{
    if (cost < cached.first)
    {
        cached = make_pair(cost, cached.second);
        coloringTable[comb] = cached;
        if (cost == cached.second)
        {
            badColoringsLocker.lock();
            newbadColorings.push_back(comb);
            badColoringsLocker.unlock();
            ci.cbcCounter++;
            ci.gcCounter--;
        }
    }
    ci.cacheHit++;
    if (cost == cached.second)
        ci.bcCounter++;
    else
        ci.gcCounter++;
}

mutex mtx;
condition_variable cv;


void TryImproveBadColoring(vector<char> comb,
                           unsigned fatherCost, vector<vector<char>> &newbadColorings,
                           CacheInfo &cacheInfo, CounterInfo &ci)
{
    auto combCost = CalculateCost(comb);
    unsigned cost = fatherCost + combCost; // Prestar atenção no custo
    pair<unsigned, unsigned> cached;
    CanonicalOrdering(comb);
    tableLocker.lock();
    if (UnsafeHasCachedValue(comb, cached))
    {
        UnsafeUpdateTable(comb, cost, cached, newbadColorings, cacheInfo, ci);
        tableLocker.unlock();
    }
    else
    {
        unsigned result;
        if(!UnsafeHasDirtyValue(comb)){
            cached = make_pair(-1, cached.second);
            coloringTable[comb] = cached;
            tableLocker.unlock();
            result = BetterColoring(cost, comb, cacheInfo);
            unique_lock<mutex> lock(mtx);
            cv.notify_all();
            lock.unlock();
        }else {
            tableLocker.unlock();
            unique_lock<mutex> lock(mtx);
            while(UnsafeHasDirtyValue(comb))
                cv.wait(lock);
            lock.unlock();
        }
        tableLocker.lock();
        if (UnsafeHasCachedValue(comb, cached))
        {
            UnsafeUpdateTable(comb, cost, cached, newbadColorings, cacheInfo, ci);
        }
        else
        {
            coloringTable[comb] = make_pair(cost, result);
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
        }
        tableLocker.unlock();
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
        // Desnecessário
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
            if (possibleColors.size() == 3)
            {
                combinationIterator = new CombinationIteratorBottomUp(possibleColors);
            }
            else
            {
                combinationIterator = new CombinationIteratorTopDown(possibleColors);
            }
            // cout << "Size: " << possibleColors.size() << " Total possibilities" << pow(5,possibleColors.size())<< endl;
            vector<vector<char>> badCanonicalColeringsToPrint;
            vector<vector<char>> goodCanonicalColeringsToPrint;
            CacheInfo cacheInfo;
            cacheInfo.CacheHit = cacheInfo.CacheMiss = 0;
            CounterInfo counterInfo;
            counterInfo.gcCounter = 0;
            counterInfo.bcCounter = 0;
            counterInfo.cgcCounter = 0;
            counterInfo.cbcCounter = 0;
            counterInfo.cCCounter = 0;
            counterInfo.cacheHit = 0;
            counterInfo.cacheMiss = 0;
            int thIndex = 0;
            vector<thread> threads;
            unsigned index = 0;
            while (true)
            {
                // cout<< index++ << endl;
                if (thIndex < processor_count)
                {
                    auto comb = combinationIterator->GetNext();
                    threads.push_back(thread(TryImproveBadColoring, comb, fatherCost, ref(newbadColorings), ref(cacheInfo), ref(counterInfo)));
                    thIndex++;
                }
                else
                {
                    thIndex = 0;
                    for (auto &th : threads)
                        th.join();
                    threads.clear();
                }
                if (combinationIterator->stop)
                {
                    for (auto &th : threads)
                        th.join();
                    threads.clear();
                    break;
                }
            }
            delete combinationIterator;
            cout << "TopDown CH: " << counterInfo.cacheHit << " CM:" << counterInfo.cacheMiss << endl;
            cout << "BetterColoring CH: " << cacheInfo.CacheHit << " CM:" << cacheInfo.CacheMiss << endl;
            // cout<< "Custo do pai: "<< fatherCost <<" Com pais: " << c << endl;
            cout << "Colorações canonicas ruins :" << counterInfo.cbcCounter << " colorações canonicas boas " << counterInfo.cgcCounter << "  colorações canonicas " << counterInfo.cCCounter << endl;
            // cout << "+++++++++++++++++++++++++++++++++" << endl;
            // cout << "Bad colorings, total: " << badCanonicalColeringsToPrint.size() << endl;
            // for (auto c : badCanonicalColeringsToPrint)
            //     cout << "[" << c << "]" << endl;
            // cout << "=============================" << endl;
            // cout << "Good Colorings, total: " << goodCanonicalColeringsToPrint.size() << endl;
            // for (auto c : goodCanonicalColeringsToPrint)
            //     cout << "[" << c << "]" << endl;
            // cout << "+++++++++++++++++++++++++++++++++" << endl;
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
    for (int i = 0; i < 15; i++)
        cout << "(" << parentPermutationMatrix[i].first << "," << parentPermutationMatrix[i].second << ")"
             << " ";
    cout << endl;
    TopDownOnTree(2);
    return 0;
}
