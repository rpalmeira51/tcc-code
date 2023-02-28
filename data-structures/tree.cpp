#include "tree.h"

void printTreeColoring(TreeColoring* tc)
{
    int level = 0;
    cout << "===========" << endl;
    while (tc != NULL)
    {
        for(auto l: (*(*tc).leafColors)){
            cout<< l.second << " ";
        }
        tc = tc->nextLevel;
        cout<< endl;
    }
    cout << "===========" << endl;
}
