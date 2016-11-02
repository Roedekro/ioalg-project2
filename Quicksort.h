//
// Created by martin on 10/1/16.
//

#ifndef IOALG_PROJECT1_QUICKSORT_H
#define IOALG_PROJECT1_QUICKSORT_H


class Quicksort {

public:
    Quicksort();
    virtual ~Quicksort();
    void sort(int * array, int p, int r);
    int partition(int * array, int p, int r, int pivot);
    int medianOfMedians(int * array, int p, int r);
    int insertionsort(int * array, int p, int r);
};


#endif //IOALG_PROJECT1_QUICKSORT_H
