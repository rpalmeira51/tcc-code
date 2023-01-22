#include <iostream>
#include <vector>
#include <memory>
#include <tuple> // for tuple
#include <map>
#include <bits/stdc++.h>
#include <limits.h>
#include <utility>
#include<algorithm>
using namespace std;

// 0 {1, 10, 6, 8, 13}
// 1 {0, 3, 4, 9, 15}
// 2 {3, 10, 8, 12, 15}
// 3 {1, 2, 6, 5, 11}
// 4 {1, 10, 5, 12, 14}
// 5 {3, 4, 8, 7, 13}
// 6 {0, 3, 7, 12, 14}
// 7 {10, 6, 5, 9, 15}
// 8 {0, 2, 5, 9, 14}
// 9 {1, 8, 7, 11, 12}
// 10 {0, 2, 4, 7, 11}
// 11 {3, 10, 9, 13, 14}
// 12 {2, 4, 6, 9, 13}
// 13 {0, 5, 11, 12, 15}
// 14 {4, 6, 8, 11, 15}
// 15 {1, 2, 7, 13, 14}

class Vertice
{
public:
    uint index;
    unsigned color;
    vector<char> adjs;
};

class Node
{
public:
    Node *leftChild;
    Node *rightChild;
    Node *parent;
    unsigned label;
};

class RootedTree
{
public:
    Node *root;
    Node *leftChild;
    Node *rightChild;
    Node *middleChild;
};

class Graph
{
public:
    vector<Vertice> adjLis;
};

vector<char> adjMatrix[16][16];
void CanonicalOrdering(vector<char>& coloring);
vector<char> ColoringValues(map<unsigned, char> colorings);

ostream& operator<<(ostream& os, const vector<char>& vec)
{
    for(auto e:vec){
        os << (int)e << " ,";
    }
    return os;
}


// Min TopDownCost e Min BottomUp Cost->Not for now
// Min TopDownCost <= Min BottomUp Cost -> Ruim 
// Testar com igualdade 
map<vector<char>, pair<unsigned,unsigned>> coloringTable;


Vertice fromInts(vector<int> vertices, int index)
{
    Vertice v;
    v.index = index;
    v.color = -1;
    for (int i = 0; i < vertices.size(); i++)
    {
        v.adjs.push_back(vertices[i]);
    }
    return v;
}

class ClebschGraph : public Graph
{
public:
    vector<char> badVertices = {0, 1, 2, 5, 7};
    ClebschGraph()
    {
        adjLis.push_back(fromInts({1, 10, 6, 8, 13}, 0));
        adjLis.push_back(fromInts({0, 3, 4, 9, 15}, 1));
        adjLis.push_back(fromInts({3, 10, 8, 12, 15}, 2));
        adjLis.push_back(fromInts({1, 2, 6, 5, 11}, 3));
        adjLis.push_back(fromInts({1, 10, 5, 12, 14}, 4));
        adjLis.push_back(fromInts({3, 4, 8, 7, 13}, 5));
        adjLis.push_back(fromInts({0, 3, 7, 12, 14}, 6));
        adjLis.push_back(fromInts({10, 6, 5, 9, 15}, 7));
        adjLis.push_back(fromInts({0, 2, 5, 9, 14}, 8));
        adjLis.push_back(fromInts({1, 8, 7, 11, 12}, 9));
        adjLis.push_back(fromInts({0, 2, 4, 7, 11}, 10));
        adjLis.push_back(fromInts({3, 10, 9, 13, 14}, 11));
        adjLis.push_back(fromInts({2, 4, 6, 9, 13}, 12));
        adjLis.push_back(fromInts({0, 5, 11, 12, 15}, 13));
        adjLis.push_back(fromInts({4, 6, 8, 11, 15}, 14));
        adjLis.push_back(fromInts({1, 2, 7, 13, 14}, 15));
    }
};

ClebschGraph ClebschGraphObj;

vector<char> VectorInstersection(vector<char> &nv, vector<char> &nu)
{
    vector<char> v3;
    std::sort(nv.begin(), nv.end());
    std::sort(nu.begin(), nu.end());

    std::set_intersection(nv.begin(), nv.end(),
                          nu.begin(), nu.end(),
                          back_inserter(v3));
    return v3;
}

void InitializeMatrix()
{
    for (int i = 0; i < ClebschGraphObj.adjLis.size(); i++)
    {
        for (int j = 0; j < ClebschGraphObj.adjLis.size(); j++)
        {
            // ////cout<< i << "   " << j << endl;
            auto nv = ClebschGraphObj.adjLis[i].adjs;
            auto nu = ClebschGraphObj.adjLis[j].adjs;
            adjMatrix[i][j] = VectorInstersection(nv, nu);
        }
    }
}

// calcular só uma vez
vector<char> PossibleChoicesCommonNeighbours(unsigned v, unsigned u)
{
    return adjMatrix[v][u];
}



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

void IncrecementNextPermutation(vector<char> &vet, vector<char> &nc)
{
}

class CombinationIterator
{
public:
    vector<char> permutation;
    vector<char> numberOfChoices;
    unsigned totalPerm = 0;
    unsigned retPerm = 0;
    unsigned goodComb = 0;
    unsigned badComb = 0;
    unsigned n;
    map<unsigned, vector<char>> &possibleColorsByVertex;
    bool stop = false;
    CombinationIterator(map<unsigned, vector<char>> &possibleColorsByVertexArg) : possibleColorsByVertex(possibleColorsByVertexArg)
    {
        n = possibleColorsByVertex.size();
        permutation.resize(n);
        for (auto c : possibleColorsByVertex)
        {
            numberOfChoices.push_back(c.second.size());
        }
    }

    map<unsigned, char> GetNextBottomUp()
    {
        map<unsigned, char> leafColoring;
        int i = 0;
        for (auto ps : possibleColorsByVertex)
        {
            leafColoring[ps.first] = ps.second[permutation[i]];
            i++;
        }
        if (!GetNextPermutation(permutation, numberOfChoices))
        {
            stop = true;
        }
        return leafColoring;
    }

    bool IsGoodLeafColoring(map<unsigned, char> &leafColoring)
    {
        auto size = leafColoring.size();
        if (size == 3)
        {
            return (leafColoring[1] <= leafColoring[2]) && (leafColoring[2] <= leafColoring[3]);
        }
        for (auto itr = leafColoring.begin(); itr != leafColoring.end(); advance(itr, 2))
        {
            auto leftPC = *itr;
            auto rightPc = *next(itr, 1);
            if (leftPC.second > rightPc.second)
                return false;
        }
        return true;
    }

    map<unsigned, char> GetNext()
    {
        map<unsigned, char> leafColoring;
        while (!stop)
        {
            int i = 0;
            for (auto ps : possibleColorsByVertex)
            {
                leafColoring[ps.first] = ps.second[permutation[i]];
                // //////////cout << "colorindo :" << ps.first  << " com a cor " << ps.second[permutation[i]]<< " e i: " << i << "e permutation[i]: "<<  (unsigned) permutation[i] << endl;
                i++;
            }
            if (!GetNextPermutation(permutation, numberOfChoices))
            {
                stop = true;
            }
            if (IsGoodLeafColoring(leafColoring))
            {
                goodComb++;
                break;
            }
            else
            {
                badComb++;
            }
        }
        return leafColoring;
    }
};

vector<map<unsigned, unsigned>> AllCombinations(map<unsigned, vector<unsigned>> &possibleColorsByVertex)
{
    //////////cout << "all combinations" << endl;
    vector<map<unsigned, unsigned>> ret;
    unsigned n = possibleColorsByVertex.size();
    vector<char> permutation(n);
    vector<char> numberOfChoices;
    // transformar em um for loop
    for (auto c : possibleColorsByVertex)
    {
        numberOfChoices.push_back(c.second.size());
    }
    ////////////cout << "numberOfChoices"<<"with size n:"<< n << endl;
    for (auto i : numberOfChoices)
    {
        ////////////cout << unsigned(i) << ", "<< endl;
    }
    do
    {
        map<unsigned, unsigned> leafColoring;
        int i = 0;
        for (auto ps : possibleColorsByVertex)
        {
            leafColoring[ps.first] = ps.second[permutation[i]];
            // //////////cout << "colorindo :" << ps.first  << " com a cor " << ps.second[permutation[i]]<< " e i: " << i << "e permutation[i]: "<<  (unsigned) permutation[i] << endl;
            i++;
        }
        ret.push_back(leafColoring);
    } while (GetNextPermutation(permutation, numberOfChoices));
    return ret;
}

unsigned calculateCost(map<unsigned, char> &levelColors)
{
    unsigned cost = 0;
    for (auto c : levelColors)
    {
        if (std::find(ClebschGraphObj.badVertices.begin(), ClebschGraphObj.badVertices.end(), c.second) != ClebschGraphObj.badVertices.end())
        {
            cost++;
        }
    }
    return cost;
}

unsigned calculateCost(unsigned vertex)
{
    if (std::find(ClebschGraphObj.badVertices.begin(), ClebschGraphObj.badVertices.end(), vertex) != ClebschGraphObj.badVertices.end())
        return 1;
    else
        return 0;
}

class TreeColoring
{
public:
    map<unsigned, char> *leafColors;
    TreeColoring *nextLevel;
};

void printTreeColoring(TreeColoring* tc)
{
    int level = 0;
    ////cout << "===========" << endl;
    while (tc != NULL)
    {
        for(auto l: (*(*tc).leafColors)){
            ////cout<< l.second << " ";
        }
        tc = tc->nextLevel;
        ////cout<< endl;
    }
    ////cout << "===========" << endl;
}

//Checar a tabela vs calcular ->>>>>>>>>>>>>>>>>
//Custo considera folhas 
unsigned betterColoring(unsigned cost, map<unsigned, char> &leafColors, vector<Node *> &leafs, TreeColoring *tc)
{
    TreeColoring newTc;
    newTc.leafColors = &leafColors;
    newTc.nextLevel = tc;
    
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
    vector<Node *> newLeafs;
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
    auto combinationIterator = CombinationIterator(possibleColors);
    auto levelCost = calculateCost(leafColors);
    auto nextCost = cost - levelCost;
    if (levelCost > cost)
        return cost;
    int index = 0;
    unsigned minCost = nextCost;
    map<unsigned, char> beestColoring;
    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNextBottomUp();
        //TODO checar a tabela
        // Coloração canonica
        auto cv = ColoringValues(pc);
        CanonicalOrdering(cv);
        // if(coloringTable.find(cv) != coloringTable.end()){
        //     //cout<< "Cached    !!!!"<< endl;
        //     auto cached = coloringTable[cv];
        //     minCost = min(minCost,cached.second);
        // }else{
            auto curCost = betterColoring(nextCost, pc, newLeafs, &newTc);
            minCost = min(minCost,curCost);
        //}
        //cout<< ColoringValues(pc)<< endl;
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



vector<char> ColoringValues(map<unsigned, char> colorings){
    vector<char> ret;
    for(auto cl: colorings){
        ret.push_back(cl.second);
    }
    return ret;
}





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
        for (auto c : badColorings)
        {
            map<unsigned, vector<char>> possibleColors;
            //auto cv =  ColoringValues(c);
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
            auto combinationIterator = CombinationIterator(possibleColors);
            unsigned gcCounter = 0;
            unsigned bcCounter = 0;
            unsigned cbcCounter =0;
            while (!combinationIterator.stop)
            {
                auto comb = combinationIterator.GetNextBottomUp();
                auto combCost = calculateCost(comb);
                unsigned cost = fatherCost + combCost ;// Prestar atenção no custo
                //cout<< "Calculating cost, fatherCost:  " << fatherCost << "  combCost  " << combCost << endl; 
                vector<char> coloringVector;
                coloringVector = ColoringValues(comb);
                CanonicalOrdering(coloringVector); 
                if(coloringTable.find(coloringVector) != coloringTable.end()){
                    auto cached = coloringTable[coloringVector];
                    bool alreadyBadColoring = cached.first == cached.second;
                    //TODO remove debug
                    if(cached.first < cached.second){
                        cout<< "PROBLEMA DOIDO _-------------------------"<< endl;
                    }
                    if(cost < cached.first ){
                        cached = make_pair(cost, cached.second);
                        coloringTable[coloringVector] = cached;
                        if(cost == cached.second){
                            cbcCounter++;
                            newbadColorings.push_back(coloringVector);
                        }
                    }
                    if( cost == cached.second){
                        bcCounter++;
                    }    
                    else 
                        gcCounter++;  
                }else {
                    auto result = betterColoring(cost, comb, children, NULL); 
                    //cout << "cost: " << cost << result 
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
            //cout<< "Custo do pai: "<< fatherCost <<" Com pais: " << c << endl;
            cout <<"Colorações canonicas ruins :" << cbcCounter << " Colorações ruins : "<< bcCounter <<" Colarações boas : "<< gcCounter <<  " total:"<< gcCounter + bcCounter <<endl;
            testindex++;
            colorIndex++;
            totalgcCounter += gcCounter;
            totalbcCounter += bcCounter;
            totalcbcCounter+= cbcCounter;
        }
        cout <<"Colorações totais canonicas ruins :" << totalcbcCounter << " Colorações ruins : "<< totalbcCounter <<" Colarações boas : "<< totalgcCounter <<  " total:"<< totalgcCounter + totalbcCounter <<endl;
        badColorings = newbadColorings; // use pointer
        children = growTree(children, index);
        level++;
        cout << level << endl;
        if(level == 3){
         return level == maxTreeLevel;
        }
    }
    return level == maxTreeLevel;
}




// 4,7,4,9, 9,7,5,7
// ^ ^      ^ ^   
bool isLexisGE(vector<char>& coloring, unsigned pace_size, int fele_index){
    int index = 0;
    while(pace_size > index){
        auto temp = fele_index+index;
        if(coloring[temp] < coloring[temp+pace_size] ){
            return false;
        }else if(coloring[temp] > coloring[temp+pace_size]) {
            return true;
        }
        index++;
    }
    return false;
}

void swapPartsOfTrees(vector<char>& coloring, unsigned pace_size, int fele_index){
    int index = 0;
    while(pace_size > index){
        auto current_ele_i = fele_index+index;
        auto temp = coloring[current_ele_i];
        coloring[current_ele_i] = coloring[current_ele_i+pace_size];
        coloring[current_ele_i+pace_size] = temp;
        index++;
    }
}

void CanonicalOrdering(vector<char>& coloring){
    auto cor_size = coloring.size();
    if(cor_size ==1 ) return;
    unsigned adjSize = (cor_size/3) *2;
    unsigned levels =  log2(adjSize);
    unsigned pace_size =1;
    //cout << "levels :" <<levels << endl;
    while (levels > 1){
        unsigned index = 0;
        while(index + pace_size < cor_size){
            if(isLexisGE(coloring,pace_size,index) ){
                //cout<< "swaping with index:" << index << "and ps" << pace_size << endl;
                swapPartsOfTrees(coloring,pace_size,index);
            }
            index+=  2*pace_size;
        }
        for(auto c: coloring){
            //cout<< c << " ";
        }
        //cout<< endl;    
        levels--;
        pace_size*=2;
    }
    unsigned findex = 0;
    unsigned sindex = findex+pace_size;
    //cout<< "ps: "<< pace_size << endl;
    if(isLexisGE(coloring,pace_size, findex)){
        //cout<< "swaping with index:" << findex << "and ps" << pace_size << endl;
        swapPartsOfTrees(coloring,pace_size,findex);
    }
    if(isLexisGE(coloring,pace_size, sindex)){
        //cout<< "swaping with index:" << sindex << "and ps" << pace_size << endl;
        swapPartsOfTrees(coloring,pace_size,sindex);
    }
    if(isLexisGE(coloring,pace_size, findex)){
        //cout<< "swaping with index:" << findex << "and ps" << pace_size << endl;
        swapPartsOfTrees(coloring,pace_size,findex);
    }
    //cout<< endl;
}


class TreeColoringNode{
    public:
        unsigned color;
        TreeColoringNode* parent;
    private:
        vector<TreeColoringNode*> nextColors;

    //Coloca um método para achar vizinho
    // Por vetor, outro por ponteiro  

};


class FinalColoringNode: TreeColoringNode{
    public:
        unsigned worstCost;
        unsigned betterColoringCost;
};

class ColoringTree{
    public: 
        vector<TreeColoringNode*> nextColors;
};



// int main(){
//     vector<unsigned> coloring{ 16,12,5,7 ,15,3,1,0, 16,5,7,3};
//     CanonicalOrdering(coloring);
//     // for(auto c: coloring){
//     //     //cout<< c << " ";
//     // }
//     //cout<< endl;

// }


int main()
{

    ////////cout << "TEste " << endl;
    InitializeMatrix();
    ////////cout << "TEste 2"<< endl;
    TopDownOnTree(3);

    return 0;
}