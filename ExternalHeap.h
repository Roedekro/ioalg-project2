//
// Created by Martin on 11/2/16.
//

#ifndef IOALG_PROJECT2_EXTERNALHEAP_H
#define IOALG_PROJECT2_EXTERNALHEAP_H

#include "Node.h"
#include "BinElement.h"

class ExternalHeap {
public:
    ExternalHeap(int fanout, int pagesize, int blocksize, int internalmemory, int streamtype);
    virtual ~ExternalHeap();
    int fanout;
    int pageSize;
    int blockSize;
    int internalMemorySize;
    int streamType;
    void insert(int i);
    void siftup(Node* node);
    int deleteMin();
    int* insertBuffer;
    int insertBufferCounter;
    int* rootPageBuffer;
    int rootPageBufferCounter;
    Node* rootNode;
    Node* lastNode;
    int nodeCounter;
    vector<Node*>* nodeVector;
    int* mergeIntBuffer;
    BinElement** mergeBinBuffer;
    void deleteFromRoot();
    void siftdown(Node* node);
    void siftdownLeaf(Node* node, bool b);
};


#endif //IOALG_PROJECT2_EXTERNALHEAP_H
