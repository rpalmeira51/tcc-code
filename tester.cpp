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



char CalculateCost(vector<vector<char>> levels){
    char ret =0;
    for(auto level:levels){
        ret += CalculateCostVertex(level);
    }
    return ret;
}

vector<vector<char>> SubproblemBetterColoring(vector<char> &leaf_colors, vector<char> good_vertices)
{
    //cout << "FLAAAG"<< endl;
    vector<vector<char>> ret;
    vector<char> best_choices;
    uint8_t level_cost = CalculateCostVertex(leaf_colors);
    if (leaf_colors.size() == 2)
    {
        auto possibles = adjMatrix[leaf_colors[0]][leaf_colors[1]];
        for(auto v: possibles){
        	if(find(good_vertices.begin(), good_vertices.end(), v) != good_vertices.end()){
                //cout<< "|" <<+v <<"|" << endl;
                best_choices.push_back(v);
                ret.push_back(best_choices);
                return ret;
            }
        }
        return ret;
    }
    vector<vector<char>> possibleColors;
    //cout << "FLAAAG 2"<< endl;
    //cout << leaf_colors << endl;
    //cout << adjMatrix[leaf_colors[0]][leaf_colors[1]] << endl;
    //cout << adjMatrix[leaf_colors[1]][leaf_colors[2]] << endl;
    if (!HasPossibleParentColors(leaf_colors, possibleColors)){
        return ret;
    }
    uint8_t min_cost = 255;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    //cout << "FLAAAG"<< endl;
    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNext();
        SimpleCanonical(pc);
        vector<vector<char>> base_level = SubproblemBetterColoring(pc, good_vertices);
        uint8_t cur_cost = CalculateCost(base_level)+ CalculateCostVertex(pc);
        // cout << pc <<endl;
        // cout << cur_cost << "is viable ?" << base_level.size() << endl;
        if(base_level.size() >0 && cur_cost < min_cost){
            min_cost = cur_cost;
            best_choices = pc;
            ret = base_level;
        }
    }
    //cout << "|" << best_choices << "|" << endl;
    if(best_choices.size()>0){
        ret.push_back(best_choices);
    }else{
        //cout << "AAAAAAAAAAAAAAA" << endl;
    }
    return ret;
}

void SubBColoringWrapper(vector<char> &leaf_colors, vector<char> good_vertices){
    cout << "==================================================="<< endl;
    auto ret = SubproblemBetterColoring(leaf_colors,good_vertices);
    for(auto level:ret){
        cout << "|" << level << "|" << endl;
    }
    cout << "|" << leaf_colors << "|" << endl;
    cout << "==================================================="<< endl;
}


// ===================================================
// 6, 8, 9, 15, 10, 14  first cost1   second cost1
// ===================================================
// |10|
// |0, 7, 4|
// |6, 8, 9, 15, 10, 14|

vector<vector<char>> TesterBetterColoring(vector<char> &leaf_colors, vector<char> good_vertices)
{
    //cout << "FLAAAG"<< endl;
    vector<vector<char>> ret;
    vector<char> best_choices;  
    uint8_t level_cost = CalculateCostVertex(leaf_colors);
    if (leaf_colors.size() == 3)
    {
        auto adjs = adjMatrix[leaf_colors[0]][leaf_colors[1]];
        auto possibles = VectorInstersection(adjs, ClebschGraphObj.adjLis[leaf_colors[2]].adjs);
        for(auto v: possibles){
            if(find(good_vertices.begin(), good_vertices.end(), v) != good_vertices.end()){
                //cout<< "|" <<+v <<"|" << endl;
                best_choices.push_back(v);
                ret.push_back(best_choices);
                return ret;
            }
        }
        return ret;
    }
    vector<vector<char>> possibleColors;
    if (!HasPossibleParentColors(leaf_colors, possibleColors)){
        return ret;
    }
    uint8_t min_cost = 255;
    auto combinationIterator = CombinationIteratorBottomUp(possibleColors);
    while (!combinationIterator.stop)
    {
        auto pc = combinationIterator.GetNext();
        SimpleCanonical(pc);
        vector<vector<char>> base_level = TesterBetterColoring(pc, good_vertices);
        uint8_t cur_cost = CalculateCost(base_level)+ CalculateCostVertex(pc);
        // cout << pc <<endl;
        // cout << cur_cost << "is viable ?" << base_level.size() << endl;
        if(base_level.size() >0 && cur_cost < min_cost){
            min_cost = cur_cost;
            best_choices = pc;
            ret = base_level;
        }
    }
    if(best_choices.size()>0){
        ret.push_back(best_choices);
    }else{
        //cout << "AAAAAAAAAAAAAAA" << endl;
    }
    return ret;
}

vector<vector<char>> TesterColoringWrapper(vector<char> &leaf_colors, vector<char> good_vertices, bool print = false){
    if(print)
        cout << "==================================================="<< endl;
    auto ret = TesterBetterColoring(leaf_colors,good_vertices);
    for(auto level:ret){
        if(print)
            cout << "|" << level << "|" << endl;
    }
    ret.insert(ret.begin(),leaf_colors);
    if(print){
        cout << "|" << leaf_colors << "|" << endl;
        cout << "==================================================="<< endl;    
    }
    return ret;
}


void ReadToTable(string filename,unordered_map<vector<char>, pair<char, char>, VectorHasher>& localColoringTable)
{
    std::ifstream file(filename);
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
            localColoringTable[coloring] = cost_pair;
        }
        file.close();
    }
    cout << "CLOSING" << endl;
}


int main(){
    InitializeMatrix();
    InitializeParentPermutationMatrix();
    //cout << "FLAAAAAAAAAAG" << endl;
    unordered_map<vector<char>, pair<char, char>, VectorHasher> localColoringTable;
    vector<char>  good_choices = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    ReadToTable("output_table_1692733877", localColoringTable);
    for(auto item: localColoringTable){
        if(item.first.size() != 6) continue;
        auto cor = item.first;
        auto ret = TesterColoringWrapper(cor,good_choices);
        if(item.second.first == item.second.second){
            cout << "Bad Coloring" << endl;
            //TesterColoringWrapper(cor,good_choices);
        }else if(CalculateCost(ret) - CalculateCostVertex(cor) >0) {
            cout << "Weird Coloring" << cor << endl;
        }else if(CalculateCost(ret) < item.second.second){
            cout << "Super Weird Coloring" << cor << "with cost: " << +CalculateCost(ret) << endl;
        }
        cout << item.first << "  " << "first cost" << +item.second.first << "   second cost" << +item.second.second << endl;
        TesterColoringWrapper(cor,good_choices,true);
        
    } 
    //vector<char> parent_good_choices = {1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 15};
        
    // vector<char> base_leafs = {0,13,6,13};
    // for(auto gc: good_choices){
    //     base_leafs[1] = gc;   
    //     SubBColoringWrapper(base_leafs,parent_good_choices);
    // }
    return 0;
}