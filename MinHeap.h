//
// Created by Martin on 11/2/16.
//

#ifndef IOPROJECT2_MINHEAP_H
#define IOPROJECT2_MINHEAP_H


class MinHeap {
public:
    MinHeap();
    virtual ~MinHeap();
    void minHeapify(int* a, int n, int i);
    void insert(int* a, int n, int in);
    int deleteMin(int*a, int n);
    void buildHeap(int* a, int n);
    void sortDescending(int* a, int n);
    void sortAscending(int* a, int n);
};


#endif //IOPROJECT2_MINHEAP_H
