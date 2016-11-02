//
// Created by Martin on 11/2/16.
//

#include "ExternalHeap.h"
#include "MinHeap.h"
#include <cstdlib>

ExternalHeap::ExternalHeap(int f, int n, int b, int i, int t) {

    fanout = f;
    nodeSize = n;
    blockSize = b;
    internalMemorySize = i;
    streamType = t;
    insertBuffer = new int[internalMemorySize+1];
    insertBufferCounter = 0;
    rootPageBuffer = new int[internalMemorySize/fanout+1];
    rootNode = new Node();
    lastNode = rootNode;

}

ExternalHeap::~ExternalHeap() {}

void ExternalHeap::insert(int i) {

    if(insertBufferCounter < internalMemorySize+1) {
        insertBufferCounter++;
        insertBuffer[insertBufferCounter] = i;
    }
    else {

        // Først sorter dem i descending order med heapsort
        MinHeap min;
        min.sortDescending(insertBuffer, insertBufferCounter);
        // Split dem op i <fanout> dele.

    }
}

int ExternalHeap::deleteMin() {


    return 1;
}