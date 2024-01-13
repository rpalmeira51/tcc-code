#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <utility>
#include <bits/stdc++.h>
#include "../data-structures/graph.h"

using namespace std;

ostream& operator<<(ostream& os, const vector<char>& vec);

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& vec);

vector<char> VectorInstersection(vector<char> &nv, vector<char> &nu);

void InitializeMatrix();

// calcular só uma vez
vector<char> PossibleChoicesCommonNeighbours(unsigned v, unsigned u);
void SimpleCanonical(vector<char> &colors);
#endif
