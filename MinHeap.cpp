//
// Created by Martin on 11/2/16.
//

#include "MinHeap.h"
#include  <cstdlib>

// HUSK AT ARRAYET SKAL VÆRE 1-INDEKSERET!!!
// Lavede minheap fordi vi alligevel skal lave binary om
// til ints og sorting, og den er nu mere overskuelig.

MinHeap::MinHeap() {}

MinHeap::~MinHeap() {}

// Invariabel at arrayet efter n er en gyldig minheap.
void MinHeap::minHeapify(int *a, int n, int i) {

    int left = i*2;
    int right = i*2+1;
    int smallest = i;

    /*
    if(left <= n && a[left] < a[i]) {
        smallest = left;
    }
    if(right <= n && a[right] < a[smallest]) {
        smallest = right;
    }
    if(smallest != i) {
        int temp = a[i];
        a[i] = a[smallest];
        a[smallest] = temp;
        minHeapify(a,n,smallest);
    }*/

    // Modificeret til ikke at lave rekursivt kald, for hastighed + stack
    bool b = true;
    while(b) {
        left = i*2;
        right = i*2+1;
        smallest = i;
        if(left <= n && a[left] < a[i]) {
            smallest = left;
        }
        if(right <= n && a[right] < a[smallest]) {
            smallest = right;
        }
        if(smallest != i) {
            int temp = a[i];
            a[i] = a[smallest];
            a[smallest] = temp;
            i = smallest;
        }
        else {
            b = false;
        }
    }
}

void MinHeap::insert(int *a, int n, int in) {
    n++;
    a[n] = in;
    // Overhold heap property, bobble op
    bool b = true;
    while(b) {
        int parent = n/2;
        if(parent > 0 && a[n] < a[parent]) {
            int temp = a[n];
            a[n] = a[parent];
            a[parent] = temp;
            n = parent;
        }
        else b = false;
    }
}

int MinHeap::deleteMin(int *a, int n) {
    int ret = a[1];
    a[1] = a[n];
    // For en god ordens skyld flyt den "slettede" til enden af arrayet
    a[n] = ret;
    minHeapify(a,n-1,1);
    return ret;
}

void MinHeap::buildHeap(int *a, int n) {
    for(int i = n-2; i > 0; i--) {
        minHeapify(a,n,i);
    }
}

// Nye in-place sorting metoder for at gøre det nemmere

void MinHeap::sortDescending(int *a, int n) {
    buildHeap(a,n);
    for(int i = n; i > 1; i--) {
        // Et deletemin kald
        int temp = a[i];
        a[i] = a[1];
        a[1] = temp;
        minHeapify(a,i-1,1);
    }
}

void MinHeap::sortAscending(int *a, int n) {
    sortDescending(a,n);
    int i = 1;
    while(i < n) {
        int temp = a[n];
        a[n] = a[i];
        a[i] = temp;
        i++;
        n--;
    }
}