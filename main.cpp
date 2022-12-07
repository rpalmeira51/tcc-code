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
    vector<unsigned> adjs;
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

vector<unsigned> adjMatrix[16][16];

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
    vector<unsigned> badVertices = {0, 1, 2, 5, 7};
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

vector<unsigned> VectorInstersection(vector<unsigned> &nv, vector<unsigned> &nu)
{
    vector<unsigned> v3;
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
            // //cout<< i << "   " << j << endl;
            auto nv = ClebschGraphObj.adjLis[i].adjs;
            auto nu = ClebschGraphObj.adjLis[j].adjs;
            adjMatrix[i][j] = VectorInstersection(nv, nu);
        }
    }
}

// calcular só uma vez
vector<unsigned> PossibleChoicesCommonNeighbours(unsigned v, unsigned u)
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
    map<unsigned, vector<unsigned>> &possibleColorsByVertex;
    bool stop = false;
    CombinationIterator(map<unsigned, vector<unsigned>> &possibleColorsByVertexArg) : possibleColorsByVertex(possibleColorsByVertexArg)
    {
        n = possibleColorsByVertex.size();
        permutation.resize(n);
        for (auto c : possibleColorsByVertex)
        {
            numberOfChoices.push_back(c.second.size());
        }
    }

    map<unsigned, unsigned> GetNextBottomUp()
    {
        map<unsigned, unsigned> leafColoring;
        int i = 0;
        for (auto ps : possibleColorsByVertex)
        {
            leafColoring[ps.first] = ps.second[permutation[i]];
            // ////////cout << "colorindo :" << ps.first  << " com a cor " << ps.second[permutation[i]]<< " e i: " << i << "e permutation[i]: "<<  (unsigned) permutation[i] << endl;
            i++;
        }
        if (!GetNextPermutation(permutation, numberOfChoices))
        {
            stop = true;
        }
        return leafColoring;
    }

    bool IsGoodLeafColoring(map<unsigned, unsigned> &leafColoring)
    {
        //            return true;
        auto size = leafColoring.size();
        // //cout<< "-----------"<< endl;
        // for(auto lc: leafColoring){
        //         //cout << lc.second <<"  ";
        //     }
        // //cout<< endl;
        if (size == 3)
        {
            // if((leafColoring[1] <= leafColoring[2]) && (leafColoring[2]<= leafColoring[3]))
            //     //cout << "Coloração boa" << endl;
            // else
            //     //cout << "Coloração ruim" << endl;
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

    map<unsigned, unsigned> GetNext()
    {
        map<unsigned, unsigned> leafColoring;
        //////cout << "size " << n << endl;
        while (!stop)
        {
            int i = 0;
            for (auto ps : possibleColorsByVertex)
            {
                leafColoring[ps.first] = ps.second[permutation[i]];
                // ////////cout << "colorindo :" << ps.first  << " com a cor " << ps.second[permutation[i]]<< " e i: " << i << "e permutation[i]: "<<  (unsigned) permutation[i] << endl;
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
    ////////cout << "all combinations" << endl;
    vector<map<unsigned, unsigned>> ret;
    unsigned n = possibleColorsByVertex.size();
    vector<char> permutation(n);
    vector<char> numberOfChoices;
    // transformar em um for loop
    for (auto c : possibleColorsByVertex)
    {
        numberOfChoices.push_back(c.second.size());
    }
    //////////cout << "numberOfChoices"<<"with size n:"<< n << endl;
    for (auto i : numberOfChoices)
    {
        //////////cout << unsigned(i) << ", "<< endl;
    }
    do
    {
        map<unsigned, unsigned> leafColoring;
        int i = 0;
        for (auto ps : possibleColorsByVertex)
        {
            leafColoring[ps.first] = ps.second[permutation[i]];
            // ////////cout << "colorindo :" << ps.first  << " com a cor " << ps.second[permutation[i]]<< " e i: " << i << "e permutation[i]: "<<  (unsigned) permutation[i] << endl;
            i++;
        }
        ret.push_back(leafColoring);
    } while (GetNextPermutation(permutation, numberOfChoices));
    return ret;
}

unsigned calculateCost(map<unsigned, unsigned> &levelColors)
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
    map<unsigned, unsigned> *leafColors;
    TreeColoring *nextLevel;
};

void printTreeColoring(TreeColoring* tc)
{
    int level = 0;
    //cout << "===========" << endl;
    while (tc != NULL)
    {
        for(auto l: (*(*tc).leafColors)){
            //cout<< l.second << " ";
        }
        tc = tc->nextLevel;
        //cout<< endl;
    }
    //cout << "===========" << endl;
}

// escrever as cores na arvore ?
//  arvore como vetor ?
//  quando o custo for negativo retorna
bool betterColoring(unsigned cost, map<unsigned, unsigned> &leafColors, vector<Node *> &leafs, TreeColoring *tc)
{
    // se o leaf size for 3 último vértice -> caso base
    //////////cout << "inicio betterColoring" << endl;
    TreeColoring newTc;
    newTc.leafColors = &leafColors;
    newTc.nextLevel = tc;
    if (leafs.size() == 3)
    {
        auto v = PossibleChoicesCommonNeighbours(leafColors[(*leafs[0]).label], leafColors[(*leafs[1]).label]);
        auto cn = VectorInstersection(v, ClebschGraphObj.adjLis[leafColors[(*leafs[2]).label]].adjs);
        //////cout << leafColors[(*leafs[0]).label] << " " << leafColors[(*leafs[1]).label] << " "<< leafColors[(*leafs[2]).label] << "-> " << endl;
        //////cout<< "possible colors: ";
        // for(auto p :cn ){
        //     ////cout<< p << " ";
        // }
        for (auto p : cn)
        {
            // //cout<< p << " ";
            if (cost > calculateCost(p))
            {
                ////////cout << "end base betterColoring" << endl;
                // //cout << "custo remanecente" << cost<< endl;
                // //cout << p;
                // //cout<< endl;
                TreeColoring rootT;
                map<unsigned, unsigned> rootColor;
                rootColor[0] = p;
                rootT.leafColors = &rootColor;
                rootT.nextLevel = &newTc;
                printTreeColoring(&rootT);
                return true;
            }
        }
        // //cout<< endl;
        ////////cout << "end base betterColoring" << endl;
        return false;
    }
    vector<Node *> newLeafs;
    map<unsigned, vector<unsigned>> possibleColors;
    for (int i = 0; i < leafs.size(); i += 2)
    {
        ////////cout << "test 1" << endl;
        auto parent = (*(leafs[i])).parent;
        auto v = leafColors[(*(leafs[i])).label];
        auto u = leafColors[(*(leafs[i + 1])).label];
        newLeafs.push_back(parent);
        ////////cout << "test 2" << endl;
        // testar se está vazio e retorna
        //  se um possible colors, retorna direto false
        ////////cout << "cor v: " << v << "  cor u "<< u << endl;
        auto cn = PossibleChoicesCommonNeighbours(v, u);
        if (cn.size() == 0)
            return false;
        possibleColors[parent->label] = cn;
    }
    ////////cout << "test 5" << endl;
    auto combinationIterator = CombinationIterator(possibleColors);
    ////////cout << "test 6" << endl;
    auto levelCost = calculateCost(leafColors);
    ////////cout << "test 7" << endl;
    auto nextCost = cost - levelCost;
    if (levelCost > cost)
        return false;
    int index = 0;

    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNextBottomUp();
        if (betterColoring(nextCost, pc, newLeafs, &newTc))
        {
            ////////cout << "end betterColoring" << endl;
            //////cout << "custo remanecente" << cost<< endl;
            for (auto l : leafs)
            {
                // //cout << leafColors[l->label] << " ";
            }
            // //cout << endl;
            return true;
        }
    }
    ////////cout << "end betterColoring" << endl;
    return false;
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
            //////cout << c->label<< ":" << comb[c->label]<< " parent :"<< c->parent->label << "  |  ";
            // //cout << comb[c->label] << " ";
            //  if(c->parent != NULL)
            //      nextLevel.push_back(c->parent);
        }
        // //cout << endl;
        currLevel = nextLevel;
    }
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
    vector<pair<unsigned, map<unsigned, unsigned>>> badCollorings;

    for (auto c : ClebschGraphObj.badVertices)
    {
        map<unsigned, unsigned> initialColoring;
        initialColoring[rtree.root->label] = c;
        badCollorings.push_back(make_pair(calculateCost(c), initialColoring));
    }

    unsigned level = 0;
    bool isFirst = true;
    while (badCollorings.size() > 0 && level < maxTreeLevel)
    {
        //////cout << "TEste 3"<< endl;
        if (isFirst)
        {
        }
        vector<pair<unsigned, map<unsigned, unsigned>>> newBadCollorings; // colocar custo
        vector<vector<unsigned>> fbtest;
        // vector<unsigned> newCosts;
        unsigned colorIndex = 0;
        int testindex = 0;
        for (auto bcPair : badCollorings)
        {
            map<unsigned, vector<unsigned>> possibleColors;
            ////////cout << "Print the children" << endl;
            // //cout << "index " <<testindex <<" ";
            auto c = bcPair.second;
            auto fatherCost = bcPair.first;
            for (auto child : children)
            {
                ////////cout << child->label << " on level " << level << endl;
                auto parentColor = c[child->parent->label];
                possibleColors[child->label] = ClebschGraphObj.adjLis[parentColor].adjs;
            }
            // ////////cout << " possible colors " << endl  ;
            // for(auto pc: possibleColors){
            //     for(auto c: pc.second){
            //         ////////cout << c << " ," << endl;
            //     }
            // }
            //////cout << "==========" << endl;
            // otimizar com irmãos
            auto combinationIterator = CombinationIterator(possibleColors);
            // auto combinations = AllCombinations(possibleColors);
            //////cout << "combinations on level : " << level << endl;
            // for(auto c: combinations){
            //     for (auto t: c){
            //         ////////cout << t.second << ",";
            //     }
            //     ////////cout << endl;
            // }

            //////cout<< "================="<< endl;
            unsigned gcCounter = 0;
            unsigned bcCounter = 0;
            // //cout << "Teste " << endl;
            while (!combinationIterator.stop)
            {
                auto comb = combinationIterator.GetNext();
                unsigned cost = fatherCost + calculateCost(comb); // Prestar atenção no custo
                // //cout<< "Coloração das folhas: ";
                // for(auto c: comb){
                //     //cout<< c.second << " ";
                // }
                if (!betterColoring(fatherCost, comb, children, NULL))
                {
                    if(newBadCollorings.find( ) == newBadCollorings.end())
                    newBadCollorings.push_back(make_pair(cost, comb));
                    bcCounter++;
                }
                else
                {
                    gcCounter++;
                }
                print_tree(children, comb);
                // //cout<< "custo: " << cost << " fcost " << fatherCost;
                // //cout  << endl;
            }
            // //cout<< combinationIterator.retPerm << "   " << combinationIterator.totalPerm<< endl;
            //cout << "Colorações ruins : " << bcCounter << "Colarações boas :" << gcCounter << " total: " << bcCounter + gcCounter << endl;
            testindex++;
            colorIndex++;
        }
        //cout << newBadCollorings.size() << " bad coloring on level : " << level << endl;
        //cout << "=========================" << endl;
        badCollorings = newBadCollorings; // use pointer
        children = growTree(children, index);
        level++;
    }
    return level == maxTreeLevel;
}




// 4,7,4,9, 9,7,5,7
// ^ ^      ^ ^   
bool isLexisGE(vector<unsigned>& coloring, unsigned pace_size, int fele_index){
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

void swapPartsOfTrees(vector<unsigned>& coloring, unsigned pace_size, int fele_index){
    int index = 0;
    while(pace_size > index){
        auto current_ele_i = fele_index+index;
        auto temp = coloring[current_ele_i];
        coloring[current_ele_i] = coloring[current_ele_i+pace_size];
        coloring[current_ele_i+pace_size] = temp;
        index++;
    }
}

void CanonicalOrdering(vector<unsigned>& coloring){
    auto cor_size = coloring.size();
    unsigned adjSize = (cor_size/3) *2;
    unsigned levels =  log2(adjSize);
    unsigned pace_size =1;
    cout << "levels :" <<levels << endl;
    while (levels > 1){
        unsigned index = 0;
        while(index + pace_size < cor_size){
            if(isLexisGE(coloring,pace_size,index) ){
                cout<< "swaping with index:" << index << "and ps" << pace_size << endl;
                swapPartsOfTrees(coloring,pace_size,index);
            }
            index+=  2*pace_size;
        }
        for(auto c: coloring){
            cout<< c << " ";
        }
        cout<< endl;    
        levels--;
        pace_size*=2;
    }
    unsigned findex = 0;
    unsigned sindex = findex+pace_size;
    cout<< "ps: "<< pace_size << endl;
    if(isLexisGE(coloring,pace_size, findex)){
        cout<< "swaping with index:" << findex << "and ps" << pace_size << endl;
        swapPartsOfTrees(coloring,pace_size,findex);
    }
    if(isLexisGE(coloring,pace_size, sindex)){
        cout<< "swaping with index:" << sindex << "and ps" << pace_size << endl;
        swapPartsOfTrees(coloring,pace_size,sindex);
    }
    if(isLexisGE(coloring,pace_size, findex)){
        cout<< "swaping with index:" << findex << "and ps" << pace_size << endl;
        swapPartsOfTrees(coloring,pace_size,findex);
    }
    cout<< endl;
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



int main(){
    vector<unsigned> coloring{ 16,12,5,7 ,15,3,1,0, 16,5,7,3};
    CanonicalOrdering(coloring);
    // for(auto c: coloring){
    //     cout<< c << " ";
    // }
    cout<< endl;

}


// int main()
// {

//     //////cout << "TEste " << endl;
//     InitializeMatrix();
//     //////cout << "TEste 2"<< endl;
//     TopDownOnTree(2);

//     return 0;
// }