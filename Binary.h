/*
 * Binary.h
 *
 *  Created on: 28/09/2016
 *      Author: Martin
 */


#include "BinElement.h"

#ifndef BINARY_H_
#define BINARY_H_

class Binary {
public:
    Binary();
    virtual ~Binary();
    int comparisons;
    BinElement* SWOPHeap(BinElement** a, int n, BinElement* in);
    void inheap(BinElement** a, int n, BinElement* in);
    BinElement* outheap(BinElement** a, int n);
    void setheap(BinElement** a, int n);
};

#endif /* BINARY_H_ */
