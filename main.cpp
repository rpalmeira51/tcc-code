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
unsigned CalculateCostVertex(unsigned vertex)
{
    if (std::find(ClebschGraphObj.badVertices.begin(), ClebschGraphObj.badVertices.end(), vertex) != ClebschGraphObj.badVertices.end())
        return 1;
    else
        return 0;
}

// 0 ,6
// 1 ,1 ,0 ,0
unsigned CalculateCostEdges(vector<char> const &parentColors, vector<char> const &colors)
{
    ////COUT << "parent" << parentColors << endl;
    ////COUT << "colors" << colors << endl;
    unsigned cost = 0;
    for (int i = 0; i < colors.size(); i++)
    {
        cost += ClebschGraphObj.EdgeCost(parentColors[i / 2], colors[i]);
    }
    ////COUT << "T cost: " << cost << endl;
    return cost;
}

// Calcula o custo para um vetor de cores
unsigned CalculateCostVertex(vector<char> const &colors)
{
    unsigned cost = 0;
    for (auto c : colors)
        cost += CalculateCostVertex(c);
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

class SpecificFunctions
{
public:
    virtual void CanonicalOrdering(vector<char> &coloring) = 0;
    virtual unsigned CalculateCost(vector<char> &parentLevel, vector<char> &level) = 0;
    virtual unsigned BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInf) = 0;
};

class VerticeSpecificFunctions : public SpecificFunctions
{
public:
    void CanonicalOrdering(vector<char> &coloring) override
    {
        return CanonicalOrderingVertices(coloring);
        // return;
    }
    unsigned CalculateCost(vector<char> &parentLevel, vector<char> &level) override
    {
        return CalculateCostVertex(level);
    }
    unsigned BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo) override;
};

class EdgeSpecificFunctions : public SpecificFunctions
{
public:
    void CanonicalOrdering(vector<char> &coloring) override
    {
        return CanonicalOrderingEdges(coloring);
        // return;
    }
    unsigned CalculateCost(vector<char> &parentLevel, vector<char> &level) override
    {
        return CalculateCostEdges(parentLevel, level);
    }
    unsigned BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo) override;
};

unsigned EdgeSpecificFunctions::BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo)
{
    if (leafColors.size() == 2)
    {
        if (!ClebschGraphObj.AreAdjacent(leafColors[0], leafColors[1]))
        {
            return cost;
        }
        return ClebschGraphObj.EdgeCost(leafColors[0], leafColors[1]);
    }
    vector<vector<char>> possibleColors;
    if (!HasPossibleParentColors(leafColors, possibleColors))
        return cost;
    unsigned minCost = 10000;
    unsigned curCost;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    // COUT << "###############################" << endl;
    // COUT << "COST:  " << cost << endl;
    // COUT << leafColors << endl;
    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNext();
        auto levelCost = CalculateCostEdges(pc, leafColors);
        auto nextCost = cost - levelCost;
        pair<unsigned, unsigned> costs;
        CanonicalOrdering(pc);
        ////COUT << pc << endl;
        tableLocker.lock();
        auto ret = UnsafeHasCachedValue(pc, costs);
        tableLocker.unlock();
        if (ret)
        {
            curCost = costs.second;
            cacheInfo.CacheHit++;
        }
        else
        {
            curCost = BetterColoring(nextCost, pc, cacheInfo);
            cacheInfo.CacheMiss++;
        }
        minCost = min(minCost, curCost + levelCost);
    }
    return minCost;
}

unsigned VerticeSpecificFunctions::BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo)
{
    if (leafColors.size() == 3)
    {
        auto v = PossibleChoicesCommonNeighbours(leafColors[0], leafColors[1]);
        auto cn = VectorInstersection(v, ClebschGraphObj.adjLis[leafColors[2]].adjs);
        auto levelCost = CalculateCostVertex(leafColors);
        unsigned minCost = cost - levelCost;
        for (auto p : cn)
        {
            minCost = min(minCost, CalculateCostVertex(p));
        }
        return minCost + levelCost;
    }
    vector<vector<char>> possibleColors;
    if (!HasPossibleParentColors(leafColors, possibleColors))
        return cost;
    auto levelCost = CalculateCostVertex(leafColors);
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
            curCost = BetterColoring(nextCost, pc, cacheInfo);
            cacheInfo.CacheMiss++;
        }
        minCost = min(minCost, curCost);
    }
    return minCost + levelCost;
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
                           vector<char> &parentComb,
                           unsigned fatherCost, vector<vector<char>> &newbadColorings,
                           CacheInfo &cacheInfo, CounterInfo &ci, SpecificFunctions &sf)
{
    auto combCost = sf.CalculateCost(parentComb, comb);
    unsigned cost = fatherCost + combCost; // Prestar atenção no custo
    pair<unsigned, unsigned> cached;
    sf.CanonicalOrdering(comb);
    tableLocker.lock();
    if (UnsafeHasCachedValue(comb, cached))
    {
        UnsafeUpdateTable(comb, cost, cached, newbadColorings, cacheInfo, ci);
        tableLocker.unlock();
    }
    else
    {
        unsigned result;
        // Condições de corrida
        if (!UnsafeHasDirtyValue(comb))
        {
            cached = make_pair(-1, cached.second);
            coloringTable[comb] = cached;
            tableLocker.unlock();
            result = sf.BetterColoring(cost, comb, cacheInfo);
            unique_lock<mutex> lock(mtx);
            tableLocker.lock();
            lock.unlock();
            cv.notify_all();
        }
        else
        {
            tableLocker.unlock();
            unique_lock<mutex> lock(mtx);
            while (true)
            {
                cv.wait(lock);
                tableLocker.lock();
                auto stop = !UnsafeHasDirtyValue(comb);
                tableLocker.unlock();
                if (stop)
                    break;
            }
            lock.unlock();
            tableLocker.lock();
        }
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
                badColoringsLocker.lock();
                newbadColorings.push_back(comb);
                badColoringsLocker.unlock();
            }
            else
            {
                ci.gcCounter++;
                ci.cgcCounter++;
            }
        }
        tableLocker.unlock();
    }
}

bool GenericTopDownTree(unsigned maxTreeLevel, vector<vector<char>> badColorings, SpecificFunctions &sf)
{
    unsigned level = 1;
    while (badColorings.size() > 0 && level < maxTreeLevel)
    {
        vector<vector<char>> newbadColorings;
#pragma region
        unsigned totalgcCounter = 0;
        unsigned totalbcCounter = 0;
        unsigned totalcbcCounter = 0;
        unsigned totalcCCounter = 0;
#pragma endregion
        // for (auto c : badColorings)
        // {
        //     cout << c << endl;
        // }
        for (auto c : badColorings)
        {
            vector<vector<char>> possibleColors;
            sf.CanonicalOrdering(c);
            tableLocker.lock();
            auto fatherCost = coloringTable[c].first;
            tableLocker.unlock();
            for (int i = 0; i < c.size() * 2; i++)
            {
                auto parentColor = c[i / 2];
                possibleColors.push_back(ClebschGraphObj.adjLis[parentColor].adjs);
            }
            CombinationIterator *combinationIterator;
            combinationIterator = new CombinationIteratorTopDown(possibleColors);
#pragma region
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
#pragma endregion

            int thIndex = 0;
            vector<thread> threads;
            unsigned index = 0;
            unsigned number_of_combinations = pow(15,c.size());
            while (true)
            {
                if(index % 1000000 ==0 ){
                    cout << "\t Parcial colorações canonicas ruins :" << counterInfo.cbcCounter << " colorações canonicas boas " << counterInfo.cgcCounter << "  colorações canonicas " << counterInfo.cCCounter << endl;
                }
                if (thIndex < processor_count)
                {
                    auto comb = combinationIterator->GetNext();
                    threads.push_back(thread(TryImproveBadColoring, comb, ref(c), fatherCost, ref(newbadColorings), ref(cacheInfo), ref(counterInfo), ref(sf)));
                    thIndex++;
                    index++;
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
            // COUT << endl;
            delete combinationIterator;
            cout << "TopDown CH: " << counterInfo.cacheHit << " CM:" << counterInfo.cacheMiss << endl;
            cout << "BetterColoring CH: " << cacheInfo.CacheHit << " CM:" << cacheInfo.CacheMiss << endl;
            cout << "Colorações canonicas ruins :" << counterInfo.cbcCounter << " colorações canonicas boas " << counterInfo.cgcCounter << "  colorações canonicas " << counterInfo.cCCounter << endl;
            // for(auto c: newbadColorings){
            //     cout<< c<< endl;
            // }
            totalgcCounter += counterInfo.gcCounter;
            totalbcCounter += counterInfo.bcCounter;
            totalcbcCounter += counterInfo.cbcCounter;
            totalcCCounter += counterInfo.cCCounter;
            // if(level == 2){
            //     return level == maxTreeLevel;
            // }
        }
        badColorings = newbadColorings; // use pointer
        level++;
    }
    return level == maxTreeLevel;
}

bool TopDownOnTreeEdge(unsigned maxTreeLevel)
{
    vector<char> initialColoring = ClebschGraphObj.badEdge;
    auto cost = ClebschGraphObj.EdgeCost(ClebschGraphObj.badEdge[0], ClebschGraphObj.badEdge[1]);
    coloringTable[initialColoring] = make_pair(cost, cost);
    vector<vector<char>> badColorings = {initialColoring};
    CacheInfo cacheInfo;
    CounterInfo counterInfo;
    EdgeSpecificFunctions esf;
    return GenericTopDownTree(maxTreeLevel, badColorings, esf);
}

// AnyBadColoringsOnLevel ?
bool TopDownOnTreeVertex(unsigned maxTreeLevel)
{
    vector<char> initialColoring{ClebschGraphObj.badVertice};
    auto cost = CalculateCostVertex(ClebschGraphObj.badVertice);
    coloringTable[initialColoring] = make_pair(cost, cost);
    vector<vector<char>> possibleColors = {{1}, {6}, {10}};
    CombinationIterator *combinationIterator;
    combinationIterator = new CombinationIteratorBottomUp(possibleColors);
    vector<vector<char>> badColorings;
    CacheInfo cacheInfo;
    CounterInfo counterInfo;
    VerticeSpecificFunctions vsf;
    while (!combinationIterator->stop)
    {
        auto comb = combinationIterator->GetNext();
        TryImproveBadColoring(comb, ref(initialColoring), cost, ref(badColorings), ref(cacheInfo), ref(counterInfo), vsf);
    }
    return GenericTopDownTree(maxTreeLevel, badColorings, vsf);
}

// Fazer essa otm
// Revisão do código e otimizações gerais
// Utilizar o banco de dados -> para guardar
int main()
{
    InitializeMatrix();
    InitializeParentPermutationMatrix();
    TopDownOnTreeEdge(4);
    return 0;
}
