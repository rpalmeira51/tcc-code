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

// Decições
// Custo infeliz(na raiz) = 100
// Custo vértice 0 = 1
// Custo de outro = 0
// Trocar custo de unsigned para char /4 a memória utilizada

const char subProblemVertexCost = 100;

// const auto processor_count = thread::hardware_concurrency() == 0 ? 8 : thread::hardware_concurrency();
const auto processor_count = 1;

bool HasGoodVertexSubstitution(char vertex, vector<char> goodVertices);

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

bool UnsafeHasCachedValue(const vector<char> &comb, pair<unsigned, unsigned> &costs,
                          unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> localColoringTable)
{
    if (localColoringTable.find(comb) != localColoringTable.end())
    {
        costs = localColoringTable[comb];
        return true;
    }
    return false;
}

// bool UnsafeHasDirtyValue(const vector<char> &comb)
// {
//     if (coloringTable.find(comb) != coloringTable.end() && coloringTable[comb].first == -1)
//         return true;
//     return false;
// }

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
    virtual unsigned BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInf,
                                    unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable) = 0;
};

class VerticeSpecificFunctions : public SpecificFunctions
{
public:
    void CanonicalOrdering(vector<char> &coloring) override
    {
        // return LexicographicOrderingSubProblem(coloring);
        return VertexLexicographicOrdering(coloring);
        // return LexicographicOrdering;
    }
    unsigned CalculateCost(vector<char> &parentLevel, vector<char> &level) override
    {
        return CalculateCostVertex(level);
    }
    unsigned BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                            unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable) override;
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
    unsigned BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                            unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable) override;
};

class SubproblemSpecificFunctions : public VerticeSpecificFunctions
{
private:
    vector<char> _goodVertices;

public:
    unsigned BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                            unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable) override;
    SubproblemSpecificFunctions(vector<char> goodVertices) : _goodVertices(goodVertices)
    {
    }
};

unsigned SubproblemSpecificFunctions::BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                                                     unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable)
{
    // cout << "COSSST test" << cost << endl;
    if (leafColors.size() == 2)
    {
        auto possibles = adjMatrix[leafColors[0]][leafColors[1]];
        for (auto gc : _goodVertices)
        {
            if (find(possibles.begin(), possibles.end(), gc) != possibles.end())
            {
                // cout << "aaaaaaaaaaaaaa " << endl;
                return 0;
            }
        }
        return subProblemVertexCost;
    }
    vector<vector<char>> possibleColors;
    if (!HasPossibleParentColors(leafColors, possibleColors))
        return cost;
    auto levelCost = CalculateCostVertex(leafColors);
    // Subproblema seu Otário
    //  > ao invés de >= não precisamos de menos vértices 0s
    if (levelCost > cost)
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
        if (UnsafeHasCachedValue(pc, costs, localColoringTable))
        {
            tableLocker.unlock();
            curCost = costs.second;
            cacheInfo.CacheHit++;
        }
        else
        {
            tableLocker.unlock();
            curCost = BetterColoring(nextCost, pc, cacheInfo, localColoringTable);
            cacheInfo.CacheMiss++;
        }
        minCost = min(minCost, curCost);
    }
    return minCost + levelCost;
}

unsigned EdgeSpecificFunctions::BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                                               unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable)
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
    unsigned curCost;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    // COUT << "###############################" << endl;
    // COUT << "COST:  " << cost << endl;
    // COUT << leafColors << endl;
    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNext();
        auto levelCost = CalculateCostEdges(pc, leafColors);
        int nextCost = cost - levelCost;
        if (nextCost <= 0)
            continue;
        pair<unsigned, unsigned> costs;
        CanonicalOrdering(pc);
        ////COUT << pc << endl;
        tableLocker.lock();
        auto ret = UnsafeHasCachedValue(pc, costs, localColoringTable);
        tableLocker.unlock();
        if (ret)
        {
            curCost = costs.second;
            cacheInfo.CacheHit++;
        }
        else
        {
            curCost = BetterColoring(nextCost, pc, cacheInfo, localColoringTable);
            tableLocker.lock();
            auto toCache = make_pair(nextCost, curCost);
            localColoringTable[pc] = toCache;
            tableLocker.unlock();
            cacheInfo.CacheMiss++;
        }
        auto calcCost = curCost + levelCost;
        if (calcCost < cost)
            return calcCost;
    }
    return cost;
}

unsigned VerticeSpecificFunctions::BetterColoring(unsigned cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                                                  unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable)
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
    auto levelCost = CalculateCostVertex(leafColors);
    if (levelCost >= cost)
        return cost;
    if (!HasPossibleParentColors(leafColors, possibleColors))
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
        if (UnsafeHasCachedValue(pc, costs, localColoringTable))
        {
            tableLocker.unlock();
            curCost = costs.second;
            cacheInfo.CacheHit++;
        }
        else
        {
            tableLocker.unlock();
            curCost = BetterColoring(nextCost, pc, cacheInfo, localColoringTable);
            cacheInfo.CacheMiss++;
        }
        minCost = min(minCost, curCost);
        if (minCost < nextCost)
            return minCost + levelCost;
    }
    return cost;
}

mutex badColoringsLocker;

void UnsafeUpdateTable(vector<char> &comb, unsigned cost, pair<unsigned, unsigned> cached,
                       vector<vector<char>> &newbadColorings,
                       CacheInfo &cacheInfo, CounterInfo &ci,
                       unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable)
{
    if (cost < cached.first)
    {
        cached = make_pair(cost, cached.second);
        localColoringTable[comb] = cached;
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

void NewTryImproveBadColoring(vector<char> comb, unsigned cost,
                              vector<vector<char>> &newbadColorings, CacheInfo &cacheInfo,
                              CounterInfo &ci, SpecificFunctions &sf,
                              unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable)
{

    unsigned result = sf.BetterColoring(cost, comb, cacheInfo, localColoringTable);
    // cout << "Cost " << cost << "result " << result << " " << endl;
    tableLocker.lock();
    auto cached = localColoringTable[comb];
    localColoringTable[comb] = make_pair(cost, result);
    if (result < cost)
    {
        ci.cgcCounter++;
    }
    else
    {
        badColoringsLocker.lock();
        newbadColorings.push_back(comb);
        badColoringsLocker.unlock();
        ci.cbcCounter++;
        // cout << "[" << comb << "], ";
    }
    tableLocker.unlock();
}

void NewTryImproveBadColoringWhThreads(vector<char> comb, unsigned cost,
                                       vector<vector<char>> &newbadColorings, CacheInfo &cacheInfo,
                                       CounterInfo &ci, SpecificFunctions &sf,
                                       unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable)
{

    unsigned result = sf.BetterColoring(cost, comb, cacheInfo, localColoringTable);
    localColoringTable[comb] = make_pair(cost, result);
    if (result < cost)
    {
        ci.cgcCounter++;
    }
    else
    {
        newbadColorings.push_back(comb);
        ci.cbcCounter++;
    }
}

// void TryImproveBadColoring(vector<char> comb,
//                            vector<char> &parentComb,
//                            unsigned fatherCost, vector<vector<char>> &newbadColorings,
//                            CacheInfo &cacheInfo, CounterInfo &ci, SpecificFunctions &sf)
// {
//     auto combCost = sf.CalculateCost(parentComb, comb);
//     unsigned cost = fatherCost + combCost; // Prestar atenção no custo
//     pair<unsigned, unsigned> cached;
//     sf.CanonicalOrdering(comb);
//     tableLocker.lock();
//     if (UnsafeHasCachedValue(comb, cached))
//     {
//         UnsafeUpdateTable(comb, cost, cached, newbadColorings, cacheInfo, ci);
//         tableLocker.unlock();
//     }
//     else
//     {
//         unsigned result;
//         // Condições de corrida
//         if (!UnsafeHasDirtyValue(comb))
//         {
//             cached = make_pair(-1, cached.second);
//             coloringTable[comb] = cached;
//             tableLocker.unlock();
//             result = sf.BetterColoring(cost, comb, cacheInfo);
//             unique_lock<mutex> lock(mtx);
//             tableLocker.lock();
//             lock.unlock();
//             cv.notify_all();
//         }
//         else
//         {
//             tableLocker.unlock();
//             unique_lock<mutex> lock(mtx);
//             while (true)
//             {
//                 cv.wait(lock);
//                 tableLocker.lock();
//                 auto stop = !UnsafeHasDirtyValue(comb);
//                 tableLocker.unlock();
//                 if (stop)
//                     break;
//             }
//             lock.unlock();
//             tableLocker.lock();
//         }
//         if (UnsafeHasCachedValue(comb, cached))
//         {
//             UnsafeUpdateTable(comb, cost, cached, newbadColorings, cacheInfo, ci);
//         }
//         else
//         {
//             coloringTable[comb] = make_pair(cost, result);
//             ci.cacheMiss++;
//             ci.cCCounter++;
//             if (cost == result)
//             {
//                 ci.bcCounter++;
//                 ci.cbcCounter++;
//                 badColoringsLocker.lock();
//                 newbadColorings.push_back(comb);
//                 badColoringsLocker.unlock();
//             }
//             else
//             {
//                 ci.gcCounter++;
//                 ci.cgcCounter++;
//             }
//         }
//         tableLocker.unlock();
//     }
// }

vector<char> GetPossibleSiblings(char v)
{
    vector<char> ret;
    for (int i = 0; i < 15; i++)
    {
        if (adjMatrix[v][i].size() == 0)
        {
            // cout << "v: " << (int)v << " i: " << i << " size: " << adjMatrix[v][i].size() << endl;
            continue;
        }
        // cout << "v: " << (int)v << " i: " << i << " size: " << adjMatrix[v][i].size() << endl;
        ret.push_back(i);
    }
    return ret;
}

bool SubProblemTopDownTree(unsigned maxTreeLevel, vector<vector<char>> badColorings, SpecificFunctions &sf,
                           unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable)
{
    unsigned level = 0;
    while (badColorings.size() > 0 && level < maxTreeLevel)
    {
        vector<vector<char>> newbadColorings;
#pragma region
        unsigned totalgcCounter = 0;
        unsigned totalbcCounter = 0;
        unsigned totalcbcCounter = 0;
        unsigned totalcCCounter = 0;
#pragma endregion
        for (auto c : badColorings)
        {
            vector<vector<char>> possibleColors;
            // cout << "c :  " << c << endl;
            sf.CanonicalOrdering(c);
            auto fatherCost = localColoringTable[c].first;
            // cout << "c :  " << c << " cost:" << fatherCost << endl;
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

            unsigned number_of_combinations = pow(15, c.size());
            while (!combinationIterator->stop)
            {
                auto comb = combinationIterator->GetNext();
                auto leafCost = sf.CalculateCost(c, comb);
                auto cost = fatherCost + leafCost;
                sf.CanonicalOrdering(comb);
                pair<unsigned, unsigned> cached;
                if (UnsafeHasCachedValue(comb, cached, localColoringTable))
                {
                    if (cost >= cached.first)
                        continue;
                    if (cached.first > cached.second)
                        counterInfo.cgcCounter--;
                }
                NewTryImproveBadColoring(comb, cost, newbadColorings, cacheInfo, counterInfo, sf, localColoringTable);
            }
            delete combinationIterator;
            for (auto bc : newbadColorings)
            {
                cout << "bad comb: " << bc << endl;
            }
            // cout << "BetterColoring CH: " << cacheInfo.CacheHit << " CM:" << cacheInfo.CacheMiss << endl;
            cout << "Colorações canonicas ruins :" << counterInfo.cbcCounter << " colorações canonicas boas " << counterInfo.cgcCounter << "  colorações canonicas " << counterInfo.cCCounter << endl;
            // for(auto c: newbadColorings){
            //     cout<< c<< endl;
            // }
            totalgcCounter += counterInfo.gcCounter;
            totalbcCounter += counterInfo.bcCounter;
            totalcbcCounter += counterInfo.cbcCounter;
            totalcCCounter += counterInfo.cCCounter;
        }
        badColorings = newbadColorings; // use pointer
        level++;
    }
    return level == maxTreeLevel;
}

bool GenericTopDownTree(unsigned maxTreeLevel, vector<vector<char>> badColorings, SpecificFunctions &sf,
                        unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable, unsigned levelarg = 1)
{
    unsigned level = levelarg;
    cout << "bad colorings " << badColorings.size() << endl;
    for (auto bc : badColorings)
    {
        cout << bc << endl;
    }
    cout << "end colorings" << endl;
    while (badColorings.size() > 0 && level < maxTreeLevel)
    {
        vector<vector<char>> newbadColorings;
#pragma region
        unsigned totalgcCounter = 0;
        unsigned totalbcCounter = 0;
        unsigned totalcbcCounter = 0;
        unsigned totalcCCounter = 0;
#pragma endregion
        for (auto c : badColorings)
        {
            vector<vector<char>> possibleColors;
            sf.CanonicalOrdering(c);
            // tableLocker.lock();
            auto fatherCost = localColoringTable[c].first;
            // tableLocker.unlock();
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

            vector<thread> threads;
            unsigned index = 0;
            unsigned number_of_combinations = pow(15, c.size());
            while (!combinationIterator->stop)
            {
                index++;
                auto comb = combinationIterator->GetNext();
                auto leafCost = sf.CalculateCost(c, comb);
                auto cost = fatherCost + leafCost;
                sf.CanonicalOrdering(comb);
                pair<unsigned, unsigned> cached;
                if (UnsafeHasCachedValue(comb, cached, localColoringTable))
                {
                    if (cost < cached.first)
                    {
                        if (cached.second != -1 && cached.first > cached.second)
                        {
                            counterInfo.cgcCounter--;
                            // counterInfo.cbcCounter++;
                        }
                        cached = make_pair(cost, -1);
                        localColoringTable[comb] = cached;
                    }
                }
                else
                {
                    cached = make_pair(cost, -1);
                    localColoringTable[comb] = cached;
                }
            }
            // if(level==3){
            //     cout << "Max index:  " << index << endl;
            //     return false;
            // }
            auto targetSize = possibleColors.size();
            int thIndex = 0;
            for (auto pair : localColoringTable)
            {
                auto k = pair.first;
                auto v = pair.second;
                if (k.size() != targetSize || v.second != -1)
                {
                    continue;
                }
                counterInfo.cCCounter++;
                // NewTryImproveBadColoring(k, v.first, ref(newbadColorings), ref(cacheInfo), ref(counterInfo), ref(sf));
                if (thIndex < processor_count)
                {
                    threads.push_back(thread(NewTryImproveBadColoring, k, v.first, ref(newbadColorings), ref(cacheInfo),
                                             ref(counterInfo), ref(sf), ref(localColoringTable)));
                    thIndex++;
                    continue;
                }
                else
                {
                    thIndex = 0;
                    for (auto &th : threads)
                        th.join();
                    threads.clear();
                }
                threads.push_back(thread(NewTryImproveBadColoring, k, v.first, ref(newbadColorings), ref(cacheInfo),
                                         ref(counterInfo), ref(sf), ref(localColoringTable)));
            }
            if (threads.size() > 0)
            {
                for (auto &th : threads)
                    th.join();
                threads.clear();
            }
            // COUT << endl;
            delete combinationIterator;
            // cout << "====================="<< endl;
            cout << "level :  " << level << endl;

            // cout << "====================="<< endl;
            // cout << "TopDown CH: " << counterInfo.cacheHit << " CM:" << counterInfo.cacheMiss << endl;
            cout << endl;
            // cout << "BetterColoring CH: " << cacheInfo.CacheHit << " CM:" << cacheInfo.CacheMiss << endl;
            cout << "Colorações canonicas ruins :" << counterInfo.cbcCounter << " colorações canonicas boas " << counterInfo.cgcCounter << "  colorações canonicas " << counterInfo.cCCounter << endl;
            if (level == 3)
            {
                cout << "Stoped for breath  " << index << endl;
                return false;
            }
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
        // if (level == 1 && levelarg != 0)
        // {
        //     for (auto bc : newbadColorings)
        //     {
        //         cout << "bad coloring" << bc << endl;
        //         int badChanges = 0;
        //         int goodChanges = 0;
        //         for (int i = 0; i < bc.size(); i++)
        //         {
        //             char temp, c;
        //             if (i % 2 == 0)
        //             {
        //                 c = bc[i + 1];
        //             }
        //             else
        //             {
        //                 c = bc[i - 1];
        //             }
        //             auto possibleChoices = GetPossibleSiblings(c);
        //             vector<char> goodChoices;
        //             cout << "possibleChoices for c: " << (int)c << endl;
        //             cout << possibleChoices << endl;
        //             temp = bc[i];
        //             for (auto v : possibleChoices)
        //             {
        //                 bc[i] = v;
        //                 auto comb = bc;
        //                 sf.CanonicalOrdering(comb);
        //                 if (localColoringTable.count(comb) == 0)
        //                 {
        //                     goodChanges++;
        //                     goodChoices.push_back(v);
        //                 }
        //                 else
        //                 {
        //                     auto cached = localColoringTable[comb];
        //                     if (cached.first == cached.second)
        //                     {
        //                         // cout << "=============" << endl;
        //                         // cout << localColoringTable.count(comb) << endl;
        //                         // cout << comb << endl;
        //                         // cout << "cached data " << cached.first << "  " << cached.second << endl;
        //                         badChanges++;
        //                     }
        //                     else
        //                     {
        //                         goodChanges++;
        //                         goodChoices.push_back(v);
        //                     }
        //                 }
        //             }
        //             bc[i] = temp;
        //             if (goodChoices.size() > 1)
        //             {
        //                 unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> subProblemColoringTable;
        //                 vector<char> subProblemBC = {temp};
        //                 SubproblemSpecificFunctions ssf(goodChoices);
        //                 vector<vector<char>> newBc = {subProblemBC};
        //                 subProblemColoringTable[subProblemBC] = make_pair(100, 1);
        //                 cout << "temp: " << (int)temp << " test ::" << endl;
        //                 cout << "goodChoides " << goodChoices << endl;
        //                 cout << "bad colorings" << subProblemBC << endl;
        //                 auto ret = SubProblemTopDownTree(2, newBc, ssf, subProblemColoringTable);
        //                 cout << "ret ::" << ret << endl;
        //             }
        //             // return false;
        //         }
        //         cout << "bc : " << badChanges << " gc: " << goodChanges << endl;
        //         break;
        //     }
        //     return false;
        // }

        badColorings = newbadColorings; // use pointer
        level++;
    }
    cout << "HELLLLO" << endl;
    return level == maxTreeLevel;
}

bool GenericTopDownTreeWhThreads(unsigned maxTreeLevel, vector<vector<char>> badColorings, SpecificFunctions &sf,
                                 unordered_map<vector<char>, pair<unsigned, unsigned>, VectorHasher> &localColoringTable, unsigned levelarg = 1)
{
    unsigned level = levelarg;
    cout << "bad colorings " << badColorings.size() << endl;
    for (auto bc : badColorings)
    {
        cout << bc << endl;
    }
    cout << "end colorings" << endl;
    while (badColorings.size() > 0 && level < maxTreeLevel)
    {
        vector<vector<char>> newbadColorings;
#pragma region
        CounterInfo counterInfo;
        counterInfo.gcCounter = 0;
        counterInfo.bcCounter = 0;
        counterInfo.cgcCounter = 0;
        counterInfo.cbcCounter = 0;
        counterInfo.cCCounter = 0;
        CacheInfo cacheInfo;
        cacheInfo.CacheHit = cacheInfo.CacheMiss = 0;
#pragma endregion
        for (auto c : badColorings)
        {
            vector<vector<char>> possibleColors;
            sf.CanonicalOrdering(c);
            auto fatherCost = localColoringTable[c].first;
            for (int i = 0; i < c.size() * 2; i++)
            {
                auto parentColor = c[i / 2];
                possibleColors.push_back(ClebschGraphObj.adjLis[parentColor].adjs);
                // cout << "{" <<ClebschGraphObj.adjLis[parentColor].adjs << "}, " ;
            }
            // cout << endl << "+++++++"<< endl;
            CombinationIterator *combinationIterator;
            combinationIterator = new CombinationIteratorBottomUp(possibleColors);
            unsigned index = 0;
            unsigned number_of_combinations = pow(15, c.size());
            while (!combinationIterator->stop)
            {
                index++;
                auto comb = combinationIterator->GetNext();
                auto leafCost = sf.CalculateCost(c, comb);
                auto cost = fatherCost + leafCost;
                sf.CanonicalOrdering(comb);
                pair<unsigned, unsigned> cached;
                if (UnsafeHasCachedValue(comb, cached, localColoringTable))
                {
                    if (cost < cached.first)
                    {
                        if (cached.second != -1 && cached.first > cached.second)
                        {
                            counterInfo.cgcCounter--;
                            // counterInfo.cbcCounter++;
                        }
                        cached = make_pair(cost, -1);
                        localColoringTable[comb] = cached;
                    }
                }
                else
                {
                    cached = make_pair(cost, -1);
                    localColoringTable[comb] = cached;
                }
            }
            cout << "==================================" << endl;
            auto targetSize = possibleColors.size();
            int thIndex = 0;
            delete combinationIterator;
            cout << "level :  " << level << endl;
            cout << endl;
            level++;
        }
        auto targetSize = badColorings[0].size() * 2;
        int index = 0;

        for (auto pair : localColoringTable)
        {
            auto k = pair.first;
            auto v = pair.second;
            if (index % (localColoringTable.size() / 100) == 0)
                cout << "done i: " << index << " iterations, " << index / (localColoringTable.size() / 100) << "\% completed" << endl;
            if (k.size() != targetSize || v.second != -1)
            {
                continue;
            }
            counterInfo.cCCounter++;
            NewTryImproveBadColoringWhThreads(k, v.first, newbadColorings, cacheInfo, counterInfo, sf, localColoringTable);
            index++;
        }
        cout << "Colorações canonicas ruins :" << counterInfo.cbcCounter << " colorações canonicas boas " << counterInfo.cgcCounter << "  colorações canonicas " << counterInfo.cCCounter << endl;
        badColorings = newbadColorings; // use pointer
        cout << "level bad colorings with size:" << badColorings.size() << endl;
        unsigned remainingBC = badColorings.size();
        for (auto bc : badColorings)
        {
            cout << bc << " with cost "
                 << " first : " << localColoringTable[bc].first << " second: " << localColoringTable[bc].second << endl;
            bool improved = false;
            auto costOfLocalImprovment = localColoringTable[bc].first;
            for (int i = 0; i < bc.size(); i++)
            {
                vector<char> goodsubs;
                cout << "Trying to change the " << (int)bc[i] << endl;
                int bsubs = 0;
                int gsubs = 0;
                auto sibling = ((i % 2) == 0) ? bc[i + 1] : bc[i - 1];
                auto ps = GetPossibleSiblings(sibling);
                for (auto s : ps)
                {
                    if (s == 0)
                    {
                        bsubs++;
                        continue;
                    }
                    vector<char> ts = bc;
                    ts[i] = s;
                    sf.CanonicalOrdering(ts);
                    if (localColoringTable.find(ts) != localColoringTable.end())
                    {
                        auto cached = localColoringTable[ts];
                        if (cached.first == cached.second)
                            bsubs++;
                        else
                        {
                            gsubs++;
                            goodsubs.push_back(s);
                        }
                    }
                    else
                    { 

                        auto temp  = sf.BetterColoring(costOfLocalImprovment , ts, cacheInfo, localColoringTable);
                        if(temp >= costOfLocalImprovment){
                            cout << "Weird coloring :(" << ts << " temp: " << temp <<  " costOfLocalImprovment:" << costOfLocalImprovment <<  endl;
                            bsubs++;
                        }else{
                            gsubs++;
                            goodsubs.push_back(s);
                        }
                    }
                }
                cout << " good subs substitution: " << goodsubs << endl;
                cout << "bad subs: " << bsubs << "   good subs " << gsubs << endl;
                if (HasGoodVertexSubstitution(bc[i], goodsubs))
                {
                    improved = true;
                    remainingBC--;
                    break;
                }
                if (improved)
                {
                    cout << "GOOTTTT BETTER" << endl;
                }
                cout << "=+++++++++++++++=" << endl;
            }
        }
        cout << "Remaining bad colorings: " << remainingBC << endl;
        cout << "===================" << endl;
    }
    cout << "HELLLLO" << endl;
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
    return GenericTopDownTree(maxTreeLevel, badColorings, esf, coloringTable);
}

// AnyBadColoringsOnLevel ?
bool TopDownOnTreeVertex(unsigned maxTreeLevel)
{
    for (auto bc : ClebschGraphObj.badVertices)
    {
        vector<char> initialColoring{bc};
        auto cost = CalculateCostVertex(bc);
        coloringTable[initialColoring] = make_pair(cost, cost);
        auto possibleChildren = ClebschGraphObj.adjLis[bc].adjs;
        vector<vector<char>> possibleColors = {possibleChildren, possibleChildren, possibleChildren};
        CombinationIterator *combinationIterator;
        combinationIterator = new CombinationIteratorBottomUp(possibleColors);
        while (!combinationIterator->stop)
        {
            auto comb = combinationIterator->GetNext();
            VertexLexicographicOrdering(comb);
            cout << "comb test 0" << comb << endl;
            auto totalCost = CalculateCostVertex(comb) + cost;
            coloringTable[comb] = make_pair(totalCost, totalCost);
        }
    }
    CacheInfo cacheInfo;
    CounterInfo counterInfo;
    vector<vector<char>> badColorings;
    VerticeSpecificFunctions vsf;
    for (auto pair : coloringTable)
    {
        auto k = pair.first;
        //cout << k.size() << endl;
        if (k.size() != 3)
            continue;
        auto v = pair.second;
        NewTryImproveBadColoringWhThreads(k, v.first, badColorings, cacheInfo, counterInfo, vsf, coloringTable);
    }
    return GenericTopDownTreeWhThreads(maxTreeLevel, badColorings, vsf, coloringTable);
}

char const good_flag = 1 << 0;      // 01
char const reachable_flag = 1 << 1; // 10

uint8_t const left_color = 0xF << 4;
uint8_t const right_color = 0xF << 0;
const uint8_t pair_choices = 136;
uint8_t color_array[pair_choices];

void InitializeColorArray()
{
    int color_array_index = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int j = i; j < 16; j++)
        {
            // cout << "pipi: " <<((i <<4) | j) << endl;
            color_array[color_array_index] = (i << 4) | j;
            color_array_index++;
            cout << color_array_index << endl;
        }
    }
    // cout<< "aaaaaaaaaaa aaaaaaaaaa" << endl;
}

pair<char, char> GetColors(uint16_t pair)
{
    char c1 = (pair & left_color) >> 4;
    char c2 = pair & right_color;
    return make_pair(c1, c2);
}

// index 22 ->  1 , 6
vector<char> GetColors(uint8_t *pairs, int size)
{
    vector<char> ret;
    for (int i = 0; i < size; i++)
    {
        auto pair = pairs[i];
        // left color 1111 0000
        //  right color 0000 1111
        char c1 = (pair & left_color) >> 4;
        char c2 = pair & right_color;
        ret.push_back(c1);
        ret.push_back(c2);
    }
    return ret;
}

// TODO teste de bijeção
size_t GetIndex(vector<char> colors)
{
    int exponent = 0;
    size_t ret = 0;
    for (int i = 0; i < colors.size(); i += 2)
    {
        u_int8_t u = colors[i];
        u_int8_t v = colors[i + 1];
        u_int8_t correspondent_pair = ((16 + (16 - u + 1)) * u) / 2 + (v - u);
        // cout<< "u: " << (int) u <<" v: " << (int )v << endl;
        // cout << "pair" << (int) correspondent_pair << endl;
        ret += correspondent_pair * (pow(pair_choices, exponent));
        exponent++;
    }
    return ret;
}

int HasBadVertex(vector<char> colors)
{
    int number_of_zeros = 0;
    for (auto c : colors)
        if (c == 0)
            number_of_zeros++;
    return number_of_zeros;
}

void SimpleCanonical(vector<char> &colors)
{
    for (int i = 0; i < colors.size(); i += 2)
    {
        auto u = colors[i];
        auto v = colors[i + 1];
        if (u <= v)
            continue;
        colors[i] = v;
        colors[i + 1] = u;
    }
}

void Test()
{
    // size_t pair_choices_exponents[5] = {1, pair_choices, (size_t)pow(pair_choices, 2), (size_t)pow(pair_choices, 3), (size_t)pow(pair_choices, 4)};
    // size_t i = 5438513;
    // uint8_t first_index = i % pair_choices;
    // uint8_t second_index = (i % pair_choices_exponents[2]) / pair_choices;
    // uint8_t third_index = (i % pair_choices_exponents[3]) / pair_choices_exponents[2];
    // uint8_t fourth_index = i / pair_choices_exponents[3];
    // cout << "index : " << i << endl;
    // cout << "bc index " << (int)first_index << "   " << (int)color_array[first_index] << endl;
    // cout << "bc index " << (int)second_index << "   " << (int)color_array[second_index] << endl;
    // cout << "bc index " << (int)third_index << "   " << (int)color_array[third_index] << endl;
    // cout << "bc index " << (int)fourth_index << "   " << (int)color_array[fourth_index] << endl;
    // uint8_t color_pairs[4] = {color_array[first_index], color_array[second_index], color_array[third_index], color_array[fourth_index]};
    // auto translate_colors = GetColors(color_pairs, 4);
    // cout << translate_colors << endl;
    // cout << "reverse" << GetIndex(translate_colors) << endl;
    vector<char> test{13, 13};
    auto index = GetIndex(test);
    cout << "Test index: " << index << endl;
}

void SanityCheckForLevel(int level, uint8_t *value_array)
{
    size_t size = (size_t)pow(pair_choices, level);
    for (size_t i = 0; i < size; i++)
    {
        auto ret = value_array[i];
        switch (ret)
        {
        case 0:
        case good_flag:
        case reachable_flag:
        case good_flag | reachable_flag:
            break;
        default:
            cout << "Sanity check failed for index: " << dec << i << " with flag: " << hex << ret << endl;
        }
    }
}

void TestTableIndexing()
{
    for (int level = 1; level <= 3; level++)
    {
        size_t level_size = (size_t)pow(pair_choices, pow(2, level - 1));
        // uint8_t *level_array = (uint8_t *)malloc(level_size * sizeof(uint8_t));
        cout << "testing for level: " << level << " with level size: " << level_size << endl;
        for (size_t i = 0; i < level_size; i++)
        {
            uint8_t number_of_indexes = pow(2, level - 1);
            uint8_t *index_array = (uint8_t *)malloc(number_of_indexes * sizeof(uint8_t));
            for (uint8_t index_position = 1; index_position <= number_of_indexes; index_position++)
            {
                u_int8_t index = (i % (size_t)pow(pair_choices, index_position)) / (size_t)pow(pair_choices, index_position - 1);
                index_array[index_position - 1] = color_array[index];
                // cout << "index: " << (int) index <<" t: " << (int)color_array[index] << endl;
            }
            auto translate_colors = GetColors(index_array, number_of_indexes);
            auto indexOfGenerateColoring = GetIndex(translate_colors);
            if (i != indexOfGenerateColoring)
            {
                cout << "Wrong Index ambiguity for index: " << dec << i << " got coloring" << translate_colors << " that generate index: " << indexOfGenerateColoring << endl;
                return;
            }
            free(index_array);
        }
        vector<char> coloring(pow(2, level));
        cout << "Testing for coloring of size: " << pow(2, level) << " on level: " << level << endl;
        for (size_t j = 0; j < level_size; j++)
        {
            // cout << "Testing for coloring: " << coloring << endl;
            auto index_from_coloring = GetIndex(coloring);
            // cout << "Index from coloring: " << index_from_coloring << endl;
            if (index_from_coloring < 0 || index_from_coloring > level_size)
            {
                cout << "Wrong index: " << index_from_coloring << " for coloring: " << coloring << " on level: " << level << " level size: " << level_size << endl;
                return;
            }
            uint8_t number_of_indexes = pow(2, level - 1);
            uint8_t *index_array = (uint8_t *)malloc(number_of_indexes * sizeof(uint8_t));
            for (uint8_t index_position = 1; index_position <= number_of_indexes; index_position++)
            {
                u_int8_t index = (index_from_coloring % (size_t)pow(pair_choices, index_position)) / (size_t)pow(pair_choices, index_position - 1);
                index_array[index_position - 1] = color_array[index];
                // cout << "index from coloring"<< index_from_coloring <<"index: " << (int) index <<" t: " << (int)color_array[index] << endl;
            }
            auto translate_colors = GetColors(index_array, number_of_indexes);
            if (coloring != translate_colors)
            {
                cout << "Wrong Color ambiguity for index: " << dec << index_from_coloring << " got coloring" << translate_colors << " from index of coloring: " << coloring << endl;
                return;
            }
            free(index_array);
            for (int ci = coloring.size() - 1; ci >= 0; ci--)
            {
                if (coloring[ci] != 15)
                {
                    coloring[ci]++;
                    if (ci % 2 == 0 && coloring[ci + 1] < coloring[ci])
                        coloring[ci + 1] = coloring[ci];
                    break;
                }
                coloring[ci] = 0;
            }
        }
    }
}

bool HasGoodVertexSubstitution(char vertex, vector<char> goodVertices)
{
    size_t pair_choices_exponents[5] = {1, pair_choices, (size_t)pow(pair_choices, 2), (size_t)pow(pair_choices, 3), (size_t)pow(pair_choices, 4)};
    size_t size_1 = pair_choices;
    uint8_t level_1[pair_choices];
    size_t size_2 = pair_choices_exponents[2];
    uint8_t level_2[size_2];
    size_t size_3 = pair_choices_exponents[4];
    uint8_t *level_3 = (uint8_t *)malloc(size_3 * sizeof(uint8_t));
    size_t size_4 = (size_t)pow(pair_choices, 8);

    for (size_t i = 0; i < size_1; i++)
    {
        uint8_t ret = 0;
        auto colors = GetColors(color_array[i]);
        char u = colors.first;
        char v = colors.second;
        // cout << "u: " << (int)u << " v: "<< (int)v<< endl;
        auto possibles = adjMatrix[u][v];
        for (auto possible_parent : possibles)
        {
            if (possible_parent == vertex)
            {
                ret = ret | reachable_flag;
                continue;
            }
            if (find(goodVertices.begin(), goodVertices.end(), possible_parent) != goodVertices.end())
            {
                ret = ret | good_flag;
                cout << "Good possible_parent" << (int)possible_parent << endl; 
            }
        }
        level_1[i] = ret;
    }
    unsigned badColorings1 = 0;
    for (size_t i = 0; i < size_1; i++)
    {
        auto ret = level_1[i];
        if (ret != 0 && ((ret & (reachable_flag | good_flag)) != ret))
        {
            cout << "weird ret" << hex << (int)ret << endl;
            cout << "index" << dec << i << endl;
        }
        if ((ret & reachable_flag) != ret)
            continue;
        if ((ret & good_flag) != ret)
            badColorings1++;
    }
    cout << badColorings1 << " bad colorings on level 1" << endl;
    cout << "test 2" << endl;
    if (badColorings1 == 0)
        return true;
    SanityCheckForLevel(1, level_1);
    for (size_t i = 0; i < size_2; i++)
    {
        uint8_t ret = 0;
        uint8_t first_index = i % pair_choices;
        uint8_t second_index = i / pair_choices;
        uint8_t color_pairs[2] = {color_array[first_index], color_array[second_index]};
        auto translate_colors = GetColors(color_pairs, 2);
        vector<vector<char>> possibleColors;
        if (!HasPossibleParentColors(translate_colors, possibleColors))
        {
            level_2[i] = ret;
            continue;
        }
        int min_number_of_zeros_for_reachable_parent = INT_MAX;
        auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
        while (!combinationIterator.stop)
        {
            auto pc = combinationIterator.GetNext();
            SimpleCanonical(pc);
            auto index = GetIndex(pc);
            auto parent_ret = level_1[index];
            int number_of_zeros = HasBadVertex(pc);
            if (number_of_zeros)
            {
                if ((parent_ret & reachable_flag) == reachable_flag)
                    min_number_of_zeros_for_reachable_parent = min(min_number_of_zeros_for_reachable_parent, number_of_zeros);
                ret = ret | (parent_ret & reachable_flag);
            }
            else
            {
                ret = ret | parent_ret;
            }
        }
        if (ret == reachable_flag)
        {
            auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
            while (!combinationIterator.stop)
            {
                auto pc = combinationIterator.GetNext();
                SimpleCanonical(pc);
                auto index = GetIndex(pc);
                auto parent_ret = level_1[index];
                int number_of_zeros = HasBadVertex(pc);
                if (number_of_zeros > min_number_of_zeros_for_reachable_parent)
                {
                    ret = ret | (parent_ret & reachable_flag);
                }
                else
                {
                    ret = ret | parent_ret;
                }
            }
        }
        level_2[i] = ret;
    }
    cout << "============ level 2" << endl;
    unsigned badColorings2 = 0;
    for (size_t i = 0; i < size_2; i++)
    {
        auto ret = level_2[i];
        if ((ret & reachable_flag) != reachable_flag)
            continue;
        if ((ret & good_flag) != good_flag)
            badColorings2++;
    }
    cout << dec << badColorings2 << " bad colorings on level 2" << endl;
    cout << dec << (double)badColorings2 / (double)size_2 << " bad colorings proprotion on level 2" << endl;
    if (badColorings2 == 0)
        return true;
    SanityCheckForLevel(2, level_2);
    cout << "test 3" << endl;
    for (size_t i = 0; i < size_3; i++)
    {
        if (i % (size_3 / 100) == 0)
            cout << "done i: " << i << " iterations, " << i / (size_3 / 100) << "\% completed" << endl;
        uint8_t ret = 0;
        uint8_t first_index = i % pair_choices;
        uint8_t second_index = (i % pair_choices_exponents[2]) / pair_choices;
        uint8_t third_index = (i % pair_choices_exponents[3]) / pair_choices_exponents[2];
        uint8_t fourth_index = i / pair_choices_exponents[3];

        uint8_t color_pairs[4] = {color_array[first_index], color_array[second_index], color_array[third_index], color_array[fourth_index]};
        auto translate_colors = GetColors(color_pairs, 4);
        vector<vector<char>> possibleColors;
        if (!HasPossibleParentColors(translate_colors, possibleColors))
        {
            level_3[i] = ret;
            continue;
        }
        int min_number_of_zeros_for_reachable_parent = INT_MAX;
        auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
        while (!combinationIterator.stop)
        {
            auto pc = combinationIterator.GetNext();
            SimpleCanonical(pc);
            auto index = GetIndex(pc);
            auto parent_ret = level_2[index];
            int number_of_zeros = HasBadVertex(pc);
            if (number_of_zeros)
            {
                if ((parent_ret & reachable_flag) == reachable_flag)
                    min_number_of_zeros_for_reachable_parent = min(min_number_of_zeros_for_reachable_parent, number_of_zeros);
                ret = ret | (parent_ret & reachable_flag);
            }
            else
            {
                ret = ret | parent_ret;
            }
        }
        if (ret == reachable_flag)
        {
            auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
            while (!combinationIterator.stop)
            {
                auto pc = combinationIterator.GetNext();
                SimpleCanonical(pc);
                auto index = GetIndex(pc);
                auto parent_ret = level_2[index];
                int number_of_zeros = HasBadVertex(pc);
                if (number_of_zeros > min_number_of_zeros_for_reachable_parent)
                {
                    ret = ret | (parent_ret & reachable_flag);
                }
                else
                {
                    ret = ret | parent_ret;
                }
            }
        }
        level_3[i] = ret;
    }
    cout << "FLAAAAG" << endl;
    unsigned badColorings3 = 0;
    for (size_t i = 0; i < size_3; i++)
    {
        auto ret = level_3[i];
        if ((ret & reachable_flag) != reachable_flag)
            continue;
        if ((ret & good_flag) != good_flag)
            badColorings3++;
    }
    cout << dec << badColorings3 << " bad colorings on level 3" << endl;
    cout << dec << (double)badColorings3 / (double)size_3 << " bad colorings proportion on level 3" << endl;
    SanityCheckForLevel(3, level_3);
    if (badColorings3 == 0)
        return true;
    return false;
    for (size_t i = 0; i < size_3; i++)
    {
        auto ret = level_3[i];
        if ((ret & reachable_flag) != reachable_flag)
            continue;
        int bad_subs = 0;
        int good_subs = 0;
        if ((ret & good_flag) != good_flag)
        {
            cout << "FLAAAG" << endl;
            uint8_t first_index = i % pair_choices;
            uint8_t second_index = (i % pair_choices_exponents[2]) / pair_choices;
            uint8_t third_index = (i % pair_choices_exponents[3]) / pair_choices_exponents[2];
            uint8_t fourth_index = i / pair_choices_exponents[3];

            uint8_t color_pairs[4] = {color_array[first_index], color_array[second_index], color_array[third_index], color_array[fourth_index]};
            auto translate_colors = GetColors(color_pairs, 4);
            // cout << "Doing test for coloring: " << translate_colors << "with index: " << i << endl;
            // cout << hex << "ret:  " << ret << endl;
            for (int i = 0; i < translate_colors.size(); i++)
            {
                auto sibling = i % 2 == 0 ? translate_colors[i + 1] : translate_colors[i - 1];
                vector<char> good_subs;
                auto possible_subs = GetPossibleSiblings(sibling);
                cout << dec << "pss: " << possible_subs << endl;
                for (auto ps : possible_subs)
                {
                    vector<char> temp_colors = translate_colors;
                    // cout << dec << "before temp_colors: " << temp_colors << endl;
                    temp_colors[i] = ps;
                    SimpleCanonical(temp_colors);
                    // cout << dec << "after temp_colors: " << temp_colors << endl;
                    auto index = GetIndex(temp_colors);
                    // cout << "index" << index << endl;
                    auto cur_ret = level_3[index];
                    // cout << hex << " flaag" << (int)cur_ret << endl;
                    if ((cur_ret & good_flag) == good_flag)
                    {
                        // cout << "FLAAAG" << endl;
                        good_subs.push_back(ps);
                    }
                }
                cout << dec << "good subs number: " << good_subs.size() << endl;
                if (good_subs.size() != 0)
                    cout << "good subs: " << good_subs << endl;
            }
            // cout << "==============================================" << endl;
        }
    }
    // #pragma region level 4
    // size_t badColorings4 = 0;
    // for (size_t i = 0; i < size_4; i++)
    // {
    //     if (i % (size_4 / 100) == 0)
    //         cout << "done i: " << i << " iterations, " << i / (size_4 / 100) << "\% completed" << endl;
    //     uint8_t ret = 0;
    //     uint8_t first_index = i % pair_choices;
    //     uint8_t second_index = (i % pair_choices_exponents[2]) / pair_choices;
    //     uint8_t third_index = (i % pair_choices_exponents[3]) / pair_choices_exponents[2];
    //     uint8_t fourth_index = (i % pair_choices_exponents[4]) / pair_choices_exponents[3];
    //     uint8_t fifth_index = (i % pair_choices_exponents[5]) / pair_choices_exponents[4];
    //     uint8_t sixth_index = (i % pair_choices_exponents[6]) / pair_choices_exponents[5];
    //     uint8_t seventh_index = (i % pair_choices_exponents[7]) / pair_choices_exponents[6];
    //     uint8_t eighth_index = i / pair_choices_exponents[7];

    //     uint8_t color_pairs[8] = {color_array[first_index], color_array[second_index], color_array[third_index], color_array[fourth_index],
    //                             color_array[fifth_index], color_array[sixth_index], color_array[seventh_index], color_array[eighth_index]};
    //     auto translate_colors = GetColors(color_pairs, 8);
    //     vector<vector<char>> possibleColors;
    //     if (!HasPossibleParentColors(translate_colors, possibleColors))
    //         continue;
    //     int min_number_of_zeros_for_reachable_parent = INT_MAX;
    //     auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    //     while (!combinationIterator.stop)
    //     {
    //         auto pc = combinationIterator.GetNext();
    //         auto index = GetIndex(pc);
    //         auto parent_ret = level_3[index];
    //         int number_of_zeros = HasBadVertex(pc);
    //         if (number_of_zeros)
    //         {
    //             if ((parent_ret & reachable_flag) == reachable_flag)
    //                 min_number_of_zeros_for_reachable_parent = min(min_number_of_zeros_for_reachable_parent, number_of_zeros);
    //             ret = ret | (parent_ret & reachable_flag);
    //         }
    //         else
    //         {
    //             ret = ret | parent_ret;
    //         }
    //     }
    //     if (ret == reachable_flag)
    //     {
    //         auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    //         while (!combinationIterator.stop)
    //         {
    //             auto pc = combinationIterator.GetNext();
    //             SimpleCanonical(pc);
    //             auto index = GetIndex(pc);
    //             auto parent_ret = level_3[index];
    //             int number_of_zeros = HasBadVertex(pc);
    //             if (number_of_zeros > min_number_of_zeros_for_reachable_parent)
    //             {
    //                 ret = ret | (parent_ret & reachable_flag);
    //             }
    //             else
    //             {
    //                 ret = ret | parent_ret;
    //             }
    //         }
    //     }
    //     if (ret == reachable_flag)
    //     {
    //         cout << translate_colors << endl;
    //         cout << "bad coloring :(" << endl;
    //         badColorings4++;
    //     }
    // }
    // cout << dec << badColorings4 << " bad colorings on level 4" << endl;
    // cout << dec << (double)badColorings4 / (double)size_4 << " bad colorings proportion on level 4" << endl;
    // cout << "yeeeey" << endl;
    // #pragma endregion level 4
    //
    free(level_3);
}

// Opt Teste
// int main()
// {
//     InitializeMatrix();
//     InitializeParentPermutationMatrix();
//     InitializeColorArray();
//     char vertex = 8;
//     vector<char> goodVertices = {2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14};
//     HasGoodVertexSubstitution(vertex, goodVertices);
//     // char vertex = 1;
//     // vector<char> goodVertices = {1, 2, 3, 5, 7, 9, 10, 12, 13, 14};
//     // HasGoodVertexSubstitution(vertex, goodVertices);
//     // Test();
//     TestTableIndexing();
//     return 0;
// }

// Fazer essa otm
// Revisão do código e otimizações gerais
// Utilizar o banco de dados -> para guardar
int main()
{
    InitializeMatrix();
    //TODO MAtar
    //InitializeParentPermutationMatrix();
    TopDownOnTreeVertex(2); 
    // TopDownOnTreeEdge(2);
    return 0;
}
