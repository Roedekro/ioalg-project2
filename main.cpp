#include <iostream>
#include "MinHeap.h"

using namespace std;

void testMinHeap() {
    srand (time(NULL));
    MinHeap* min = new MinHeap();
    int a[11];
    for(int i = 1; i <= 11; i++) {
        a[i] = rand()%100;
    }

    min->sortDescending(a,11);

    for(int i = 1; i <= 11; i++) {
        cout << a[i] << '\n';
    }

    cout << "--------------------\n";

    min->sortAscending(a,11);

    for(int i = 1; i <= 11; i++) {
        cout << a[i] << '\n';
    }

    cout << "--------------------\n";

    min->deleteMin(a,11);
    min->deleteMin(a,10);
    min->deleteMin(a,9);

    min->insert(a,8,1);
    min->insert(a,9,2);
    min->insert(a,10,3);

    min->sortAscending(a,11);

    for(int i = 1; i <= 11; i++) {
        cout << a[i] << '\n';
    }
}


int main() {

    testMinHeap();

    return 0;
}