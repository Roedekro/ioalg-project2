//
// Created by Martin on 11/2/16.
//

#ifndef IOPROJECT2_EXSTERNALHEAPSORT_H
#define IOPROJECT2_EXSTERNALHEAPSORT_H


class ExternalHeapsort {

    ExternalHeapsort(int fanout, int nodesize, int blocksize, int internalmemory);
    virtual ~ExternalHeapsort();
    int fanout;
    int nodeSize;
    int blockSize;
    int internalMemorySize;
};


#endif //IOPROJECT2_EKSTERNALHEAPSORT_H
