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

vector<char> coloringValues(map<unsigned, char> colorings);

// Calcula o custo para uma cor
unsigned calculateCost(unsigned vertex)
{
    if (std::find(ClebschGraphObj.badVertices.begin(), ClebschGraphObj.badVertices.end(), vertex) != ClebschGraphObj.badVertices.end())
        return 1;
    else
        return 0;
}

// Calcula o custo para um mapa de indíce para -> cores 
unsigned calculateCost(map<unsigned, char> &levelColors)
{
    unsigned cost = 0;
    for (auto c : levelColors)
        cost += calculateCost(c.second);
    return cost;
}

unsigned betterColoring(unsigned cost, map<unsigned, char> &leafColors, vector<Node *> &leafs)
{    
    if (leafs.size() == 3)
    {
        auto v = PossibleChoicesCommonNeighbours(leafColors[(*leafs[0]).label], leafColors[(*leafs[1]).label]);
        auto cn = VectorInstersection(v, ClebschGraphObj.adjLis[leafColors[(*leafs[2]).label]].adjs);
        auto levelCost = calculateCost(leafColors);
        auto rootCost = cost-levelCost;
        unsigned minCost = rootCost;
        for (auto p : cn)
        {
            minCost = min(minCost,calculateCost(p));
        }
        return minCost+levelCost;
    }
    vector<Node*> newLeafs;
    map<unsigned, vector<char>> possibleColors;
    for (int i = 0; i < leafs.size(); i += 2)
    {
        auto parent = (*(leafs[i])).parent;
        auto v = leafColors[(*(leafs[i])).label];
        auto u = leafColors[(*(leafs[i + 1])).label];
        newLeafs.push_back(parent);
        auto cn = PossibleChoicesCommonNeighbours(v, u);
        if (cn.size() == 0)
            return cost;
        possibleColors[parent->label] = cn;
    }
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    auto levelCost = calculateCost(leafColors);
    auto nextCost = cost - levelCost;
    if (levelCost > cost)
        return cost;
    int index = 0;
    unsigned minCost = nextCost;
    map<unsigned, char> beestColoring;
    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNext();
        //TODO checar a tabela
        // Coloração canonica
        auto cv = coloringValues(pc);
        CanonicalOrdering(cv);
        // if(coloringTable.find(cv) != coloringTable.end()){
        //     //cout<< "Cached    !!!!"<< endl;
        //     auto cached = coloringTable[cv];
        //     minCost = min(minCost,cached.second);
        // }else{
            auto curCost = betterColoring(nextCost, pc, newLeafs);
            minCost = min(minCost,curCost);
        //}
        //cout<< coloringValues(pc)<< endl;
        // if(curCost< minCost){
        //     beestColoring = pc;
        //     minCost = curCost;
        // }
        
    }
    return minCost + levelCost;
}

vector<Node *> growTree(vector<Node *> children, unsigned &lastIndex)
{
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

void print_tree(vector<Node *> children, map<unsigned, unsigned> &comb)
{
    vector<Node *> currLevel = children;
    while (currLevel.size() > 0)
    {
        vector<Node *> nextLevel;
        for (auto c : children)
        {
            ////////cout << c->label<< ":" << comb[c->label]<< " parent :"<< c->parent->label << "  |  ";
            // ////cout << comb[c->label] << " ";
            //  if(c->parent != NULL)
            //      nextLevel.push_back(c->parent);
        }
        // ////cout << endl;
        currLevel = nextLevel;
    }
}

vector<char> coloringValues(map<unsigned, char> colorings){
    vector<char> ret;
    for(auto cl: colorings){
        ret.push_back(cl.second);
    }
    return ret;
}

// AnyBadColoringsOnLevel ?
bool TopDownOnTree(unsigned maxTreeLevel)
{
    unsigned index = 0;
    RootedTree rtree;
    rtree.root = new Node();
    rtree.root->label = index++;

    rtree.leftChild = new Node();
    rtree.middleChild = new Node();
    rtree.rightChild = new Node();
    vector<Node *> children;
    children.push_back(rtree.leftChild);
    children.push_back(rtree.middleChild);
    children.push_back(rtree.rightChild);

    for (auto children : children)
    {
        children->label = index++;
        children->parent = rtree.root;
    }
    vector<vector<char>> badColorings;

    for (auto c : ClebschGraphObj.badVertices)
    {
        vector<char> initialColoring{c};
        initialColoring = {c};
        badColorings.push_back(initialColoring);
        auto cost = calculateCost(c);
        coloringTable[initialColoring] = make_pair(cost,cost);
    }

    unsigned level = 0;
    bool isFirst = true;
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
            map<unsigned, vector<char>> possibleColors;
            //auto cv =  coloringValues(c);
            //cout<< "cv: " << cv << " fatherCost " << coloringTable[cv].first<< endl;
            CanonicalOrdering(c);
            //cout<< "cv: " << cv << " fatherCost " << coloringTable[cv].first<< endl;
            auto fatherCost = coloringTable[c].first;
            if(children.size() ==3){
                for(int i =0; i< children.size(); i++){
                    auto child = children[i];
                    auto parentColor = c[0];
                    possibleColors[child->label] = ClebschGraphObj.adjLis[parentColor].adjs;
                }
            }else {
                for(int i =0; i< children.size(); i++){
                    auto child = children[i];
                    auto parentColor = c[i/2];
                    possibleColors[child->label] = ClebschGraphObj.adjLis[parentColor].adjs;
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
                auto combCost = calculateCost(comb);
                unsigned cost = fatherCost + combCost ;// Prestar atenção no custo
                //cout<< "Calculating cost, fatherCost:  " << fatherCost << "  combCost  " << combCost << endl; 
                vector<char> coloringVector;
                coloringVector = coloringValues(comb);
                CanonicalOrdering(coloringVector); 
                if(coloringTable.find(coloringVector) != coloringTable.end()){
                    auto cached = coloringTable[coloringVector];
                    bool alreadyBadColoring = cached.first == cached.second;
                    if(cost < cached.first ){
                        cached = make_pair(cost, cached.second);
                        coloringTable[coloringVector] = cached;
                        if(cost == cached.second){
                            //cbcCounter++;
                            newbadColorings.push_back(coloringVector);
                        }
                    }
                    if( cost == cached.second){
                        bcCounter++;
                    }    
                    else 
                        gcCounter++;  
                }else {
                    auto result = betterColoring(cost, comb, children); 
                    //cout << "cost: " << cost << result 
                    cCCounter++;
                    coloringTable[coloringVector] = make_pair(cost, result);
                    if( cost == result){
                        bcCounter++;
                        cbcCounter++;
                        newbadColorings.push_back(coloringVector);
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
        children = growTree(children, index);
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