#include <iostream>
#include <sys/time.h>
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

    //ExternalHeap* heap = new ExternalHeap(2,1,8192,2,3); // VIRKER
    ExternalHeap* heap = new ExternalHeap(4,8,8192,32,3);
    for(int i = 0; i < 300; i++) { // 30 for special case
        int in = rand() % 10000;
        cout << "--------------------------------------------------------- Inserting: " << in << '\n';
        heap->insert(in);

    }
    cout << "------------------------------------------------------------------ Insert Done\n";

    //TreeChecker* tc = new TreeChecker(2,1);
    //tc->checkNodeRecursive(heap->rootNode,true);
    //cout << "Total records = " << tc->totalRecords << '\n';


    int prev = -1;
    for(int i = 0; i < 300; i++) { // 20 for special case
        //cout << "--- " << i << '\n';
        int ret = heap->deleteMin();
        cout << "---------------------------------------------------- Deleted: " << ret << " i="<<i<< '\n';
        if(ret < prev) {
            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        }
        prev = ret;
        /*if(i == 15) { // 15 for special case
            tc->checkNodeRecursive(heap->rootNode,true);
            cout << "Total records = " << tc->totalRecords << '\n';
            i = 1000;
        }*/
        /*if(i == 0) {
            tc->checkNodeRecursive(heap->rootNode,true);
            cout << "Total records = " << tc->totalRecords << '\n';
            cout << "SpecialCounter = " << heap->specialCounter << '\n';
            i = 1000;
        }*/
    }


}

void realTest(int fanout, int pageSize, int memory, int block, int type, int runs, int n) {

    long time_total = 0;
    struct timeval te1;
    struct timeval te2;

    for(int i = 0; i < runs; i++) {
        gettimeofday(&te1,NULL);
        ExternalHeap* heap = new ExternalHeap(fanout,pageSize,block,memory,type);
        for(int j = 0; j < n; j++) {
            int in = rand() % 1000000;
            heap->insert(in);
        }
        for(int j = 0; j < n; j++) {
            heap->deleteMin();
        }
        delete(heap);
        gettimeofday(&te2,NULL);
        long time = (te2.tv_sec - te1.tv_sec) * 1000 + (te2.tv_usec - te1.tv_usec) / 1000;
        time_total = time_total + time;
        cout << "Finished test " << i+1 << " in " << time << '\n';
    }

    if(runs > 1 && time_total > 0) {
        time_total = time_total / runs;
    }

    cout << "Test finished in average time " << time_total << '\n';

}

int main(int argc, char* argv[]) {

    if(argc < 7) {
        cout << "fanout pageSize blockSize type runs N" << '\n';
    }

    int fanout, pageSize, memory,block,type,runs,n;

    if(argc <= 1) {
        fanout = 2;
        pageSize = 1;
        memory = fanout*pageSize;
        block = 32768;
        type = 4;
        runs = 10;
        n = 300;
    }
    else {
        fanout = atoi(argv[1]);
        pageSize = atoi(argv[2]);
        memory = fanout*pageSize;
        block = atoi(argv[3]);
        type = atoi(argv[4]);
        runs = atoi(argv[5]);
        n = atoi(argv[6]);
    }


    cout << "Running test with fanout = " << fanout << " pageSize = " << pageSize << " internalMemorySize = " << memory << '\n';
    cout << "blockSize = " << block << " type = " << type << " for " << runs << " runs and N = " << n << '\n';

    realTest(fanout, pageSize,memory,block,type,runs,n);

    //testMinHeapRandom();
    //testInsertRandom();
    //testInsert();
    //testMinHeap();

    return 0;
}