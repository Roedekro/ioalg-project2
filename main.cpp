#include <iostream>
#include "MinHeap.h"
#include "ExternalHeap.h"

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

void testInsert() {
    ExternalHeap* heap = new ExternalHeap(2,1,8192,2,3);
    for(int i = 0; i < 10; i++) {
        heap->insert(i);
        cout << i << '\n';
    }
    cout << "--------------------------------------------Insert Done\n";

    for(int i = 0; i < 10; i++) {
        int ret = heap->deleteMin();
        cout << "------------------------ Deleted: " << ret << '\n';
    }

}

int main() {

    testInsert();
    //testMinHeap();

    return 0;
}