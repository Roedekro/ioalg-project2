#include <iostream>
#include "MinHeap.h"
#include "ExternalHeap.h"
#include "TreeChecker.h"

using namespace std;

void testMinHeapRandom() {
    MinHeap* min = new MinHeap();
    int a[101];
    for(int i = 1; i <= 101; i++) {
        a[i] = rand()%10000;
    }

    min->sortDescending(a,101);
    delete(min);

    int prev = 2000000;
    for(int i = 1; i <= 101; i++) {
        int val = a[i];
        if(val > prev) {
            cout << "!!!!!!!!!!!!!!!!!!!!!!" << '\n';
        }
        prev = val;
    }




}


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

void testInsertRandom() {

    ExternalHeap* heap = new ExternalHeap(2,1,8192,2,3);
    for(int i = 0; i < 30; i++) {
        int in = rand() % 10000;
        cout << "--------------------------------------------------------- Inserting: " << in << '\n';
        heap->insert(in);

    }
    cout << "------------------------------------------------------------------ Insert Done\n";

    TreeChecker* tc = new TreeChecker();
    tc->checkNodeRecursive(heap->rootNode,true);
    cout << "Total records = " << tc->totalRecords << '\n';


    int prev = -1;
    for(int i = 0; i < 20; i++) {
        //cout << "--- " << i << '\n';
        int ret = heap->deleteMin();
        cout << "---------------------------------------------------- Deleted: " << ret << " i="<<i<< '\n';
        if(ret < prev) {
            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        }
        prev = ret;
        if(i == 14) {
            tc->checkNodeRecursive(heap->rootNode,true);
            cout << "Total records = " << tc->totalRecords << '\n';
            i = 1000;
        }
    }


}

int main() {

    //testMinHeapRandom();
    testInsertRandom();
    //testInsert();
    //testMinHeap();

    return 0;
}