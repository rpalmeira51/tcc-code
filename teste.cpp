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


int main(){
    size_t index = pow(120,8);
    size_t total = 0;
    for(size_t i =0; i< index; i++){
        total++;
    }
    cout<< "total calculated : " << total << endl;
}