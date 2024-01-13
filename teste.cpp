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
#include ".main.cpp"

using namespace std;


int main(){
    size_t index = SIZE_MAX;
    double test = pow(136,8);
    cout<< "test:  " << endl;
    if(test > index){
        cout<< " bad :("<< endl;
    }
    cout << index << "   " << test << "  " <<index -test << endl;
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
        case free_reachable_flag:
        case restricted_reachable_flag:
        case free_reachable_flag | restricted_reachable_flag:
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