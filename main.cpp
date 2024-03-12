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

bool HasGoodVertexSubstitution(char vertex, vector<char> goodVertices, bool subsubproblem = false);


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

bool UnsafeHasCachedValue(const vector<char> &comb, pair<char, char> &costs,
                          unordered_map<vector<char>, pair<char, char>, VectorHasher> localColoringTable)
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
//  vector<char> UnsafeHasCachedValue(vector<char> comb, pair<char, char> &costs, bool& hasValue)
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
    virtual uint8_t CalculateCost(vector<char> &parentLevel, vector<char> &level) = 0;
    virtual uint8_t BetterColoring(uint8_t cost, vector<char> &leafColors, CacheInfo &cacheInf,
                                   unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable) = 0;
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
    uint8_t CalculateCost(vector<char> &parentLevel, vector<char> &level) override
    {
        return CalculateCostVertex(level);
    }
    uint8_t BetterColoring(uint8_t cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                           unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable) override;
};

class EdgeSpecificFunctions : public SpecificFunctions
{
public:
    void CanonicalOrdering(vector<char> &coloring) override
    {
        return CanonicalOrderingEdges(coloring);
        // return;
    }
    uint8_t CalculateCost(vector<char> &parentLevel, vector<char> &level) override
    {
        return CalculateCostEdges(parentLevel, level);
    }
    uint8_t BetterColoring(uint8_t cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                           unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable) override;
};

class SubproblemSpecificFunctions : public VerticeSpecificFunctions
{
private:
    vector<char> _goodVertices;

public:
    uint8_t BetterColoring(uint8_t cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                           unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable) override;
    SubproblemSpecificFunctions(vector<char> goodVertices) : _goodVertices(goodVertices)
    {
    }
};

uint8_t SubproblemSpecificFunctions::BetterColoring(uint8_t cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                                                    unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable)
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
    uint8_t minCost = nextCost;
    uint8_t curCost;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);

    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNext();
        pair<char, char> costs;
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

uint8_t EdgeSpecificFunctions::BetterColoring(uint8_t cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                                              unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable)
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
    uint8_t curCost;
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
        pair<char, char> costs;
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

uint8_t VerticeSpecificFunctions::BetterColoring(uint8_t cost, vector<char> &leafColors, CacheInfo &cacheInfo,
                                                 unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable)
{
    if (leafColors.size() == 3)
    {
        auto v = PossibleChoicesCommonNeighbours(leafColors[0], leafColors[1]);
        auto cn = VectorInstersection(v, ClebschGraphObj.adjLis[leafColors[2]].adjs);
        auto levelCost = CalculateCostVertex(leafColors);
        uint8_t minCost = cost - levelCost;
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
    uint8_t minCost = nextCost;
    uint8_t curCost;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNext();
        pair<char, char> costs;
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

void UnsafeUpdateTable(vector<char> &comb, uint8_t cost, pair<char, char> cached,
                       vector<vector<char>> &newbadColorings,
                       CacheInfo &cacheInfo, CounterInfo &ci,
                       unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable)
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

void NewTryImproveBadColoring(vector<char> comb, uint8_t cost,
                              vector<vector<char>> &newbadColorings, CacheInfo &cacheInfo,
                              CounterInfo &ci, SpecificFunctions &sf,
                              unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable)
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

void NewTryImproveBadColoringWhThreads(vector<char> comb, uint8_t cost,
                                       vector<vector<char>> &newbadColorings, CacheInfo &cacheInfo,
                                       CounterInfo &ci, SpecificFunctions &sf,
                                       unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable)
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

// Retorna todos os vértices com vizinhos em comum com v
vector<char> GetPossibleSiblings(char v)
{
    vector<char> ret;
    for (int i = 0; i < 16; i++)
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
                           unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable)
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
                pair<char, char> cached;
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
                        unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable, unsigned levelarg = 1)
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
                pair<char, char> cached;
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
        //                 unordered_map<vector<char>, pair<char, char>, VectorHasher> subProblemColoringTable;
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

void PrintCacheTable(const unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable)
{
    ofstream myfile;
    std::time_t t = std::time(0);
    string file_name = "output_table_" + to_string(t);
    myfile.open(file_name, ios::out | ios::app);
    if (!myfile.is_open())
        return;
    for (auto row : localColoringTable)
    {
        myfile << row.first << " | " << (int)row.second.first << " ," << (int)row.second.second << endl;
    }
    myfile.close();
}

void TryImproveBadColoringsWithSubProblem(const vector<vector<char>> &badColorings,
                                          unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable,
                                          SpecificFunctions &sf,
                                          CacheInfo &cacheInfo);

bool GenericTopDownTreeWhThreads(unsigned maxTreeLevel, vector<vector<char>> badColorings, SpecificFunctions &sf,
                                 unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable, unsigned levelarg = 1)
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
                pair<char, char> cached;
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
        badColorings = newbadColorings; // use pointer'
        PrintCacheTable(localColoringTable);
        cout << "level bad colorings with size:" << badColorings.size() << endl;
        TryImproveBadColoringsWithSubProblem(badColorings, localColoringTable, sf, cacheInfo);
    }
    cout << "HELLLLO" << endl;
    return level == maxTreeLevel;
}

bool cmp(pair<vector<char>, vector<vector<char>>> &a,
         pair<vector<char>, vector<vector<char>>> &b)
{
    return a.second.size() < b.second.size();
}

bool TryImproveWithSubProblemFromTable(int remainingBC, map<vector<char>, vector<vector<char>>> &toImproveTable, bool dosubsubproblem)
{
    cout << "Trying to solve table with " << remainingBC << " colorings" << endl;
    cout << "toImproveTable size: " << toImproveTable.size() << endl;
    while (remainingBC != 0 && toImproveTable.size() != 0)
    {
        vector<char> k;
        vector<vector<char>> coloringsToImprove;
        for (auto kv : toImproveTable)
        {
            if (kv.second.size() > coloringsToImprove.size())
            {
                k = kv.first;
                coloringsToImprove = kv.second;
            }
        }
        if (k.size() == 0)
            return false;
        auto subs = vector<char>(k.begin(), k.end() - 1);
        auto root = k[k.size() - 1];
        cout << (int)k[k.size() - 1] << " with subs " << subs << " appears: " << k.size() << endl;
        if (HasGoodVertexSubstitution(root, subs, dosubsubproblem))
        {
            for (auto kv : toImproveTable)
            {
                if (kv.first == k)
                    continue;
                auto nv = kv.second;
                for (auto bc : coloringsToImprove)
                {
                    auto it = std::find(nv.begin(), nv.end(), bc);
                    // cout << "size before" << kv.second.size() << endl;
                    if (it != nv.end())
                    {
                        nv.erase(it);
                        // cout << "Removing" << endl;
                    }
                    // cout << "size after" << kv.second.size() << endl;
                }
                toImproveTable[kv.first] = nv;
            }
            remainingBC -= coloringsToImprove.size();
            cout << "Resolved " << (int)k[k.size() - 1] << " with subs " << subs << " appears: " << coloringsToImprove.size() << endl;
        }
        else
        {
            cout << "Unable to resolve" << (int)k[k.size() - 1] << " with subs " << subs << " appears: " << coloringsToImprove.size() << endl;
        }
        toImproveTable.erase(k);
        if (!dosubsubproblem)
            cout << "Remaining BC: " << remainingBC << endl;
        else
            cout << "Real Remaining BC: " << remainingBC << endl;
        cout << "toImproveTable size: " << toImproveTable.size() << endl;
    }
    return remainingBC == 0;
}

void TryImproveBadColoringsWithSubProblem(const vector<vector<char>> &badColorings,
                                          unordered_map<vector<char>, pair<char, char>, VectorHasher> &localColoringTable,
                                          SpecificFunctions &sf,
                                          CacheInfo &cacheInfo)
{
    unsigned remainingBC = badColorings.size();
    map<vector<char>, vector<vector<char>>> toImproveTable;
    for (auto bc : badColorings)
    {
        // cout << bc << " with cost "
        //      << " first : " << (int)localColoringTable[bc].first << " second: " << (int)localColoringTable[bc].second << endl;
        auto costOfLocalImprovment = localColoringTable[bc].first;
        for (int i = 0; i < bc.size(); i++)
        {
            vector<char> goodsubs;
            // cout << "Trying to change the " << (int)bc[i] << endl;
            int bsubs = 0;
            int gsubs = 0;
            auto sibling = ((i % 2) == 0) ? bc[i + 1] : bc[i - 1];
            auto ps = GetPossibleSiblings(sibling);
            // cout << "ps" << ps << endl;
            for (auto s : ps)
            {
                // cout << (int)s << endl;
                if (s == 0)
                {
                    bsubs++;
                    continue;
                }
                vector<char> ts = bc;
                // cout << "bc:  " << bc << endl;
                ts[i] = s;
                sf.CanonicalOrdering(ts);
                // cout << "ts:  " << ts << endl;
                if (localColoringTable.count(ts) == 1)
                {
                    auto cached = localColoringTable[ts];
                    if (cached.first == cached.second)
                    {
                        bsubs++;
                    }
                    else
                    {
                        gsubs++;
                        goodsubs.push_back(s);
                    }
                }
                else
                {
                    auto free_cost = sf.BetterColoring(costOfLocalImprovment, ts, cacheInfo, localColoringTable);
                    if (free_cost >= costOfLocalImprovment)
                    {
                        // cout << "Weird coloring :(" << ts << " temp: " << temp << " costOfLocalImprovment:" << costOfLocalImprovment << endl;
                        bsubs++;
                    }
                    else
                    {
                        gsubs++;
                        goodsubs.push_back(s);
                    }
                }
            }
            if (goodsubs.size() < 7)
                continue;
            // map<char,vector<pair<vector<char>,char>>>
            auto key = goodsubs;
            key.push_back(bc[i]);
            if (toImproveTable.count(key) == 1)
            {
                if (find(toImproveTable[key].begin(), toImproveTable[key].end(), bc) == toImproveTable[key].end())
                    toImproveTable[key].push_back(bc);
            }
            else
            {
                vector<vector<char>> tempValue = {bc};
                toImproveTable[key] = tempValue;
            }
        }
    }
    cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
    TryImproveWithSubProblemFromTable(remainingBC, toImproveTable, true);
}

void ReadAndTryImprove(string filename)
{
    std::ifstream file(filename);
    CacheInfo cacheInfo;
    CounterInfo counterInfo;
    vector<vector<char>> badColorings;
    VerticeSpecificFunctions vsf;
    if (file.is_open())
    {
        std::string line;
        int index = 0;
        while (std::getline(file, line) && index < 5)
        {
            // using printf() in all tests for consistency
            printf("%s\n", line.c_str());
            vector<char> coloring;
            string tempNum = "";
            char first_num, second_num;
            bool is_coloring = true;
            // cout << "\n";
            for (auto c : line)
            {
                // cout << c << endl;
                if (is_coloring)
                {
                    if (isdigit(c))
                    {
                        tempNum += c;
                    }
                    else if (c == '|')
                    {
                        is_coloring = false;
                        tempNum = "";
                    }
                    else if (tempNum != "")
                    {
                        // cout<< "FLAAG" << tempNum << endl;
                        coloring.push_back(stoi(tempNum));
                        tempNum = "";
                    }
                }
                else
                {
                    // cout<< "FLAAG 2" << tempNum << endl;
                    // cout << "c" << c << endl;
                    if (isdigit(c))
                    {
                        tempNum += c;
                    }
                    else if (c == ',')
                    {
                        first_num = stoi(tempNum);
                        tempNum = "";
                    }
                }
            }
            second_num = stoi(tempNum);
            pair<char, char> cost_pair = make_pair(first_num, second_num);
            if (first_num == second_num && coloring.size() == 6)
                badColorings.push_back(coloring);
            coloringTable[coloring] = cost_pair;
        }
        file.close();
    }
    TryImproveBadColoringsWithSubProblem(badColorings, coloringTable, vsf, cacheInfo);
    cout << "CLOSING" << endl;
}

bool TopDownOnTreeEdge(unsigned maxTreeLevel)
{
    vector<char> initialColoring = ClebschGraphObj.badEdge;
    uint8_t cost = ClebschGraphObj.EdgeCost(ClebschGraphObj.badEdge[0], ClebschGraphObj.badEdge[1]);
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
        if (k.size() != 3)
            continue;
        auto v = pair.second;
        NewTryImproveBadColoringWhThreads(k, v.first, badColorings, cacheInfo, counterInfo, vsf, coloringTable);
    }
    return GenericTopDownTreeWhThreads(maxTreeLevel, badColorings, vsf, coloringTable);
}

// custo_r/custo_l/alca_r/alca_l

uint8_t const free_reachable_flag = 0b00000001;       // 01
uint8_t const restricted_reachable_flag = 0b00000010; // 10
uint8_t const free_cost_flag = 0b00011100;             // 00011100
uint8_t const restricted_cost_flag = 0b11100000;       // 11100000

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
            color_array[color_array_index] = (i << 4) | j;
            color_array_index++;
        }
    }
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
        ret += correspondent_pair * (pow(pair_choices, exponent));
        exponent++;
    }
    return ret;
}

int HasBadVertex(pair<char, char> colors)
{
    return (colors.first == 0) + (colors.second == 0);
}

int HasBadVertex(vector<char> colors)
{
    int number_of_zeros = 0;
    for (auto c : colors)
        if (c == 0)
            number_of_zeros++;
    return number_of_zeros;
}


uint8_t ExtractRestricetedReacheble(uint8_t flag)
{
    return (flag & restricted_reachable_flag);
}

uint8_t ExtractFreeReacheble(uint8_t flag)
{
    return (flag & free_reachable_flag);
}

uint8_t ExtractFreeCost(uint8_t flag)
{
    return (flag & free_cost_flag) >> 2;
}

uint8_t ExtractRestricetedCost(uint8_t flag)
{
    return (flag & restricted_cost_flag) >> 5;
}

uint8_t FreeCostToFlag(uint8_t flag)
{
    return flag << 2;
}

uint8_t RestricetedCostToFlag(uint8_t flag)
{
    return flag << 5;
}

bool IsBad(uint8_t flag)
{
    return ExtractRestricetedReacheble(flag) && (!ExtractFreeReacheble(flag) || (ExtractFreeCost(flag) > ExtractRestricetedCost(flag)));
}

bool IsGood(uint8_t flag)
{
    return ExtractFreeReacheble(flag) && ((ExtractFreeCost(flag) <= ExtractRestricetedCost(flag)));
}

// TODO Remove zeros
//  bbb/bbb b b
//  custo_r/custo_l/alca_r/alca_l
bool HasGoodVertexSubstitution(char undesirable, vector<char> goodVertices, bool dosubsubproblem)
{
    string file_name = "";
    if(dosubsubproblem)
        file_name = "./results/";
    else
        file_name = "./results/subsub/";
    for (auto c : goodVertices)
    {
        file_name += to_string((int)c);
        file_name += "-";
    }
    file_name += "|-";
    file_name += to_string((int)undesirable);
    file_name += ".log";
    cout << "FILE NAME" << file_name << endl;
    ofstream myfile;
    myfile.open(file_name, ios::out | ios::app);
    ofstream ofile(file_name);
    size_t pair_choices_exponents[5] = {1, pair_choices, (size_t)pow(pair_choices, 2), (size_t)pow(pair_choices, 3), (size_t)pow(pair_choices, 4)};
    size_t size_1 = pair_choices;
    uint8_t level_1[pair_choices];
    size_t size_2 = pair_choices_exponents[2];
    uint8_t level_2[size_2];
    myfile << "Trying substitutions to: " << (int)undesirable << endl;
    myfile << "with goodVertices " << goodVertices << endl;
    uint8_t free_cost, restricted_cost;
    for (size_t i = 0; i < size_1; i++)
    {
        uint8_t ret = 0;
        auto colors = GetColors(color_array[i]);
        char u = colors.first;
        char v = colors.second;
        // cout << "i: " << i << endl;
        auto possibles = adjMatrix[u][v];
        free_cost = restricted_cost = HasBadVertex(colors);
        for (auto possible_parent : possibles)
        {
            if (possible_parent == undesirable)
            {
                ret = ret | restricted_reachable_flag;
                if (possible_parent == 0)
                    restricted_cost++;
            }
            else if (find(goodVertices.begin(), goodVertices.end(), possible_parent) != goodVertices.end())
            {
                ret = ret | free_reachable_flag;
                // cout << "Good possible_parent" << (int)possible_parent << endl;
            }
        }
        ret = ret | FreeCostToFlag(free_cost) | RestricetedCostToFlag(restricted_cost);
        level_1[i] = ret;
    }
    unsigned badColorings1 = 0;
    map<size_t, vector<pair<size_t,size_t>>> bad_configs_map;
    for (size_t i = 0; i < size_1; i++)
    {
        auto ret = level_1[i];
        auto colors = GetColors(color_array[i]);
        if (IsBad(ret))
        {
            myfile << "bad color to improve: " << +colors.first << "  " << +colors.second << "  with index " << i << endl;
            bad_configs_map[i]= {};
            badColorings1++;
        }
    }
    myfile << badColorings1 << " bad colorings on level 1" << endl;
    if (badColorings1 == 0)
    {
        myfile.close();
        return true;
    }
    // SanityCheckForLevel(1, level_1);
    //Chegamos até aqui 
    for (size_t i = 0; i < size_2; i++)
    {
        uint8_t ret = 0b11111100;
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
        // cout << "Checking coloring " << translate_colors << endl;
        int min_number_of_zeros_for_reachable_parent = INT_MAX;
        auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
        uint8_t level_cost = HasBadVertex(translate_colors);
        vector<size_t> parent_bad_colorings;
        size_t best_free_parent_i = SIZE_MAX;
        while (!combinationIterator.stop)
        {
            auto pc = combinationIterator.GetNext();
            SimpleCanonical(pc);
            auto index = GetIndex(pc);
            auto parent_ret = level_1[index];
            /*if (ExtractFreeReacheble(parent_ret) || ExtractRestricetedReacheble(parent_ret))
            {
                cout << "Trying to solve parent  " << pc << endl;
                cout << "Costs free cost << " << +ExtractFreeCost(parent_ret) << " restricted cost: " << +ExtractRestricetedCost(parent_ret) << endl;
                cout << "With flag free reachable: " << (ExtractFreeReacheble(parent_ret) ? "Y" : "N") << " restricted reachable: " << (ExtractRestricetedReacheble(parent_ret) ? "Y" : "N") << endl;
            }
            else
            {
                cout << "Unreachable parent:: " << bitset<8>(parent_ret) << endl;
            }*/
            if (IsBad(parent_ret))
                parent_bad_colorings.push_back(index);
            if (ExtractFreeReacheble(parent_ret))
            {
                // cout << "freeee" << endl;
                ret |= free_reachable_flag;
                uint8_t current_best_parent_free_cost = ExtractFreeCost(ret) - level_cost;
                // cout << "ret: " << bitset<8>(ret) << " current_best_parent_free_cost: " << +current_best_parent_free_cost << endl;
                uint8_t parent_free_cost = ExtractFreeCost(parent_ret);
                // cout << "parent_ret: " << bitset<8>(parent_ret) << " parent_free_cost: " << +parent_free_cost << endl;
                if (current_best_parent_free_cost > parent_free_cost)
                {
                    ret = ((ret & (~free_cost_flag)) | FreeCostToFlag(parent_free_cost + level_cost));
                    best_free_parent_i = index;
                }
                // cout << "new ret: " << bitset<8>(ret)<< endl;
            }
            else if (ExtractRestricetedReacheble(parent_ret))
            {
                // cout << "restricted" << endl;
                ret |= restricted_reachable_flag;
                uint8_t current_best_parent_r_cost = ExtractRestricetedCost(ret) - level_cost;
                // cout << "ret: " << bitset<8>(ret) << " current_best_parent_r_cost: " << +current_best_parent_r_cost << endl;
                uint8_t parent_r_cost = ExtractRestricetedCost(parent_ret);
                // cout << "parent_ret: " << bitset<8>(parent_ret) << " parent_r_cost: " << +parent_r_cost << endl;
                ret = ((ret & (~restricted_cost_flag)) | ((current_best_parent_r_cost > parent_r_cost) ? RestricetedCostToFlag(parent_r_cost + level_cost) : ret));
                // cout << "new ret: " << bitset<8>(ret) << endl;
            }
            // cout << "============" << endl;
        }
        // cout << "Final Result for " << translate_colors << endl;
        // cout << "Costs free cost << " << +ExtractFreeCost(ret) << " restricted cost: " << +ExtractRestricetedCost(ret) << endl;
        // cout << "With flag free reachable: " << (ExtractFreeReacheble(ret) ? "Y" : "N") << " restricted reachable: " << (ExtractRestricetedReacheble(ret) ? "Y" : "N") << endl;
        // cout << "================================================================" << endl;

        // if(parent_bad_colorings.size() > 0 && !IsBad(ret)){
        //     myfile<< "Solved this bad configurations:"<< endl;
        //     for(auto pi: parent_bad_colorings){
        //         auto colors = GetColors(color_array[pi]);
        //         auto possibles = adjMatrix[colors.first][colors.second];
        //         myfile<< "index: "<< +colors.first << "," << +colors.second<< "  possibles: " << possibles << endl;
        //     }
        //     auto colors = GetColors(color_array[best_free_parent_i]);
        //     auto possibles = adjMatrix[colors.first][colors.second];
        //     myfile <<"With this configuration " << +colors.first << "," << +colors.second << "  possibles: " << possibles << endl;
        //     myfile << "============"<< endl;
        // }
        if (parent_bad_colorings.size() > 0)
        {
            if (IsBad(ret))
            {
                for (auto pi : parent_bad_colorings)
                {
                    bad_configs_map.erase(pi);
                }
            }
            else
            {
                for (auto pi : parent_bad_colorings)
                {
                    if (bad_configs_map.count(pi) == 1)
                    {
                        auto p = make_pair(i, best_free_parent_i);
                        bad_configs_map[pi].push_back(p);
                    }
                }
            }
        }
        level_2[i] = ret;
    }
    for (auto bc : bad_configs_map)
    {
        auto config = GetColors(color_array[bc.first]);
        myfile << "Solved config"
               << "  |" << +config.first << ", " << +config.second << "|"
               << " descendents: " << endl;
        myfile << "|" << ClebschGraphObj.adjLis[config.first].adjs << " |---| " << ClebschGraphObj.adjLis[config.second].adjs << "|" << endl;
        ;
        auto indexes = bc.second;
        sort(indexes.begin(), indexes.end(), [](const pair<size_t, size_t> &a, const pair<size_t, size_t> &b)
             { return a.first < b.first; });
        for (auto dc_p : indexes)
        {
            uint8_t first_index = dc_p.first % pair_choices;
            uint8_t second_index = dc_p.first / pair_choices;
            uint8_t color_pairs[2] = {color_array[first_index], color_array[second_index]};
            auto d_config = GetColors(color_pairs, 2);
            auto parent_config = GetColors(color_array[dc_p.second]);
            auto possibles = adjMatrix[parent_config.first][parent_config.second];
            myfile << "|" << d_config << "|"
                   << "---> |" << +parent_config.first << ", " << +parent_config.second << "| possibles  |" << possibles << "|" << endl;
        }
        myfile << "+++++++++++++++++++++++++++++++++++++++" << endl;
    }
    unsigned badColorings2 = 0;
    for (size_t i = 0; i < size_2; i++)
    {
        auto ret = level_2[i];
        if (IsBad(ret))
        {
            badColorings2++;
            uint8_t first_index = i % pair_choices;
            uint8_t second_index = i / pair_choices;
            uint8_t color_pairs[2] = {color_array[first_index], color_array[second_index]};
            auto translate_colors = GetColors(color_pairs, 2);
            myfile << "bad coloring on level 2  |" << translate_colors << "|" << endl;
        }
    }
    myfile << endl;
    myfile << badColorings2 << " bad colorings on level 2" << endl;
    if (badColorings2 == 0)
    {
        myfile.close();
        return true;
    }
    // for (size_t i = 0; i < size_2; i++)
    // {
    //     if (level_2[i] == reachable_flag)
    //     {
    //         badColorings2++;
    //         uint8_t first_index = i % pair_choices;
    //         uint8_t second_index = i / pair_choices;
    //         uint8_t color_pairs[2] = {color_array[first_index], color_array[second_index]};
    //         auto translate_colors = GetColors(color_pairs, 2);
    //         vector<vector<char>> possibleColors;
    //         if (!HasPossibleParentColors(translate_colors, possibleColors))
    //         {
    //             continue;
    //         }
    //         int min_number_of_zeros_for_reachable_parent = INT_MAX;
    //         auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    //         //cout << "Coloring: " << translate_colors << "with index: "<< i << endl;
    //         while (!combinationIterator.stop)
    //         {
    //             auto pc = combinationIterator.GetNext();
    //             SimpleCanonical(pc);
    //             auto index = GetIndex(pc);
    //             auto parent_ret = level_1[index];
    //             int number_of_zeros = HasBadVertex(pc);
    //             cout << "===============================" << endl;
    //             cout << "Trying to solve parent" << pc << endl;
    //             cout << "number of zeros" << number_of_zeros << endl;
    //             cout << "possibles: " << adjMatrix[pc[0]][pc[1]] << endl;
    //             cout << "With flag reachable: " << (IsReachable(parent_ret) ? "Y" : "N") << "  good: " << (IsGood(parent_ret) ? "Y" : "N") << endl;
    //             if ((parent_ret & reachable_flag) == reachable_flag)
    //                 min_number_of_zeros_for_reachable_parent = min(min_number_of_zeros_for_reachable_parent, number_of_zeros);
    //         }
    //         cout << "+++++++++++++++++++++++" << endl;
    //     }
    // }
    if (dosubsubproblem)
    {
        cout << "Preparing SubProblem" << endl;
        map<vector<char>, vector<vector<char>>> toImproveTable;
        map<vector<char>, vector<vector<char>>> to_print_table;
        for (size_t i = 0; i < size_2; i++)
        {
            auto ret = level_2[i]; 
            if (IsBad(ret))
            {
                uint8_t first_index = i % pair_choices;
                uint8_t second_index = i / pair_choices;
                uint8_t color_pairs[2] = {color_array[first_index], color_array[second_index]};
                auto bc = GetColors(color_pairs, 2);
                for (int i = 0; i < bc.size(); i++)
                {
                    vector<char> goodsubs;
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
                        SimpleCanonical(ts);
                        // cout << "ts:  " << ts << endl;
                        auto index = GetIndex(ts);
                        auto sub_ret = level_2[index];

                        //&&
                        //&
                        // 110 & 010 = 010
                        // 00
                        // 01 -> good flag
                        // 10 -> reachable_flag
                        // 11
                        if (IsGood(sub_ret) && ( ExtractFreeCost(sub_ret) <= ExtractRestricetedCost(ret)))
                        {
                            goodsubs.push_back(s);
                        }
                    }
                    if (goodsubs.size() < 7)
                        continue;
                    // map<char,vector<pair<vector<char>,char>>>
                    auto key = goodsubs;
                    // cout << "pushing to solver root " << (int)bc[i] << "from" << bc << " with subs" << goodsubs << endl;
                    key.push_back(bc[i]);
                    if (toImproveTable.count(key) == 1)
                    {
                        if (find(toImproveTable[key].begin(), toImproveTable[key].end(), bc) == toImproveTable[key].end())
                            toImproveTable[key].push_back(bc);
                    }
                    else
                    {
                        vector<vector<char>> tempValue = {bc};
                        toImproveTable[key] = tempValue;
                    }
                    if (to_print_table.count(bc) == 1)
                    {
                        to_print_table[bc].push_back(key);
                    }
                    else
                    {
                        vector<vector<char>> tempValue = {key};
                        to_print_table[bc] = tempValue;
                    }
                }
            }
        }
        myfile << "Sub sub problem :" << endl;
        for (auto te : to_print_table)
        {
            myfile << "Trying improving " << te.first << " with subs:" << endl;
            for (auto k : te.second)
            {
                auto subs = vector<char>(k.begin(), k.end() - 1);
                auto root = k[k.size() - 1];
                myfile << "Root to change : " << +root << " with colors |" << subs << "|" << endl;
            }
            myfile << "++++++++++++++++++++++" << endl;
        }

        cout << "Starting subsubproblem" << endl;
        auto ret = TryImproveWithSubProblemFromTable(badColorings2, toImproveTable, false);
        if (!ret)
            myfile << "Unable to be solved with SubSubProblem" << endl;
        else
            myfile << "Solved with SubSubProblem" << endl;
        myfile.close();
        return ret;
    }
    cout << dec << badColorings2 << " bad colorings on level 2" << endl;
    cout << dec << (double)badColorings2 / (double)size_2 << " bad colorings proportion on level 2" << endl;
    myfile.close();
    if (badColorings2 == 0)
        return true;
    else
        return false;
    // SanityCheckForLevel(2, level_2);
    cout << "test 3" << endl;
    size_t size_3 = pair_choices_exponents[4];
    uint8_t *level_3 = (uint8_t *)malloc(size_3 * sizeof(uint8_t));
    size_t size_4 = (size_t)pow(pair_choices, 8);
    for (size_t i = 0; i < size_3; i++)
    {
        // if (i % (size_3 / 100) == 0)
        //     cout << "done i: " << i << " iterations, " << i / (size_3 / 100) << "\% completed" << endl;
        uint8_t ret = 0;
        uint8_t first_index = i % pair_choices;
        uint8_t second_index = (i % pair_choices_exponents[2]) / pair_choices;
        uint8_t third_index = (i % pair_choices_exponents[3]) / pair_choices_exponents[2];
        uint8_t fourth_index = i / pair_choices_exponents[3];

        uint8_t color_pairs[4] = {color_array[first_index], color_array[second_index], color_array[third_index], color_array[fourth_index]};
        auto translate_colors = GetColors(color_pairs, 4);
        vector<vector<char>> possibleColors;
        // cout << "Coloring: " << translate_colors << "with index: "<< i << endl;
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
            if ((parent_ret & restricted_reachable_flag) == restricted_reachable_flag)
                min_number_of_zeros_for_reachable_parent = min(min_number_of_zeros_for_reachable_parent, number_of_zeros);
            if (number_of_zeros)
            {
                ret = ret | (parent_ret & restricted_reachable_flag);
            }
            else
            {
                ret = ret | parent_ret;
            }
        }
        if ((ret & free_reachable_flag) != free_reachable_flag)
        {
            auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
            while (!combinationIterator.stop)
            {
                auto pc = combinationIterator.GetNext();
                SimpleCanonical(pc);
                auto index = GetIndex(pc);
                auto parent_ret = level_2[index];
                int number_of_zeros = HasBadVertex(pc);
                if (number_of_zeros <= min_number_of_zeros_for_reachable_parent)
                {
                    ret = ret | parent_ret;
                }
            }
        }
        level_3[i] = ret;
    }
    unsigned badColorings3 = 0;
    for (size_t i = 0; i < size_3; i++)
    {
        if (level_3[i] == restricted_reachable_flag)
        {
            badColorings3++;
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
                continue;
            }

            int min_number_of_zeros_for_reachable_parent = INT_MAX;
            auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
            cout << "Bad Coloring: " << translate_colors << "with index: " << i << endl;
            while (!combinationIterator.stop)
            {
                auto pc = combinationIterator.GetNext();
                SimpleCanonical(pc);
                auto index = GetIndex(pc);
                auto parent_ret = level_2[index];
                int number_of_zeros = HasBadVertex(pc);
                cout << "===============================" << endl;
                cout << "Trying to solve parent" << pc << endl;
                cout << "number of zeros" << number_of_zeros << endl;
                cout << "possibles: ";
                for (auto i = 0; i < pc.size(); i += 2)
                {
                    cout << adjMatrix[pc[i]][pc[i + 1]] << "--|--";
                }
                cout << endl;
                // cout << "With flag reachable: " << (IsReachable(parent_ret) ? "Y" : "N") << "  good: " << (IsGood(parent_ret) ? "Y" : "N") << endl;
                if ((parent_ret & restricted_reachable_flag) == restricted_reachable_flag)
                    min_number_of_zeros_for_reachable_parent = min(min_number_of_zeros_for_reachable_parent, number_of_zeros);
            }
            cout << "+++++++++++++++++++++++" << endl;
        }
    }
    cout << dec << badColorings3 << " bad colorings on level 3" << endl;
    cout << dec << (double)badColorings3 / (double)size_3 << " bad colorings proportion on level 3" << endl;
    // SanityCheckForLevel(3, level_3);
    free(level_3);
    if (badColorings3 == 0)
        return true;
    return false;
}

void Initialize(){
    InitializeMatrix();
    InitializeColorArray();
    InitializeParentPermutationMatrix();
} 


set<vector<char>> perm_table;

bool helper(vector<char> permutation, int index, unsigned vertex, ofstream& stream){
    if(permutation.size() >5 ){
        perm_table.insert(permutation);
        if(HasGoodVertexSubstitution(vertex, permutation, false)) {
            stream << "good replacement for vertex: " << vertex <<  " <-> perm: " << permutation << endl;
            return true;
        }
        //return false;
    }
    if(index == 16) return false;
    if(helper(permutation, index+1, vertex, stream)) return true;
    cout << permutation << endl;
    if(index == vertex)
        return false;
    permutation.push_back(index);
    cout << permutation << endl;
    return helper(permutation, index+1, vertex, stream); 
}

void ProduceSubProblemTable(){
    vector<char> permutation_helper;
    ofstream myfile;
    myfile.open("sub_problem_table", ios::out | ios::trunc);
    for(int i =0; i< 16; i++){
        helper(permutation_helper,1,i, myfile);    
    }
    auto max = pow(2,16);
    cout << "====================" << endl;
    cout << "Test" << endl;
    for(unsigned i=0 ; i< max ; i++ ){
        //cout << hex <<"i: "<< i << endl;
        vector<char> permutation;
        for(int j =0; j< 16; j++){
            //cout <<"j: " << j << " (i >> j) " << (i >> j) << endl; 
            if(((i >> j) &1) == 1){
                permutation.push_back(j);
            }
        } 
        if(permutation.size() > 5){
            //cout << "Testing perm  " << permutation << endl;
            if( permutation[0] !=0  && perm_table.find(permutation) == perm_table.end()){
                cout << "missing perm  " << permutation << endl;
            }
        }
    }
}

// Fazer essa otm
// Revisão do código e otimizações gerais
// Utilizar o banco de dados -> para guardar
int main()
{    
    Initialize();
    //TopDownOnTreeVertex(3);
    ProduceSubProblemTable();
    cout << "end"<< endl;
    //ReadAndTryImprove("output_table_1692733877");
     return 0;
}
