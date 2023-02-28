#include <vector>
#include <bits/stdc++.h>

using namespace std;

// 4,7,4,9, 9,7,5,7
// ^ ^      ^ ^
// Dada um vetor determina se para determinado passo o valor no índice dado é menor ou igual
// lexigograficamente que o próximo(compara primeiro o primeiro elemento, depois o segundo e assim por diante)
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


// Dada uma coloração e um passo, inverte o valor(elemento a elemento) no índice dado
// com o próximo
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


// Ordena o vetor na ordem canônica lexigograficam
void CanonicalOrdering(vector<char>& coloring){
    auto cor_size = coloring.size();
    if(cor_size ==1 ) return;
    unsigned adjSize = (cor_size/3) *2;
    unsigned levels =  log2(adjSize);
    unsigned pace_size =1;
    while (levels > 1){
        unsigned index = 0;
        while(index + pace_size < cor_size){
            if(isLexisGE(coloring,pace_size,index) ){
                swapPartsOfTrees(coloring,pace_size,index);
            }
            index+=  2*pace_size;
        }  
        levels--;
        pace_size*=2;
    }
    unsigned findex = 0;
    unsigned sindex = findex+pace_size;
    if(isLexisGE(coloring,pace_size, findex)){
        swapPartsOfTrees(coloring,pace_size,findex);
    }
    if(isLexisGE(coloring,pace_size, sindex)){
        swapPartsOfTrees(coloring,pace_size,sindex);
    }
    if(isLexisGE(coloring,pace_size, findex)){
        swapPartsOfTrees(coloring,pace_size,findex);
    }
}