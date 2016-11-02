//
// Created by Martin on 11/2/16.
//

#ifndef IOALG_PROJECT2_EXTERNALHEAP_H
#define IOALG_PROJECT2_EXTERNALHEAP_H

#include "Node.h"

class ExternalHeap {

    ExternalHeap(int fanout, int nodesize, int blocksize, int internalmemory, int streamtype);
    virtual ~ExternalHeap();
    int fanout;
    int nodeSize;
    int blockSize;
    int internalMemorySize;
    int streamType;
    void insert(int i);
    int deleteMin();
    int* insertBuffer;
    int insertBufferCounter;
    int* rootPageBuffer;
    Node* rootNode;
    Node* lastNode;
};


#endif //IOALG_PROJECT2_EXTERNALHEAP_H
