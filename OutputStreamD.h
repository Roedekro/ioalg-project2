//
// Created by soren on 9/20/16.
//

#include <stdio.h>
#include "OutputStream.h"

#ifndef IOALG_PROJECT1_OUTPUTSTREAMD_H
#define IOALG_PROJECT1_OUTPUTSTREAMD_H

class OutputStreamD : public OutputStream {
public:
    int index;
    int filedesc;
    int portionSize;
    int portionIndex;
    int *map;
    int n;
    OutputStreamD(int portionSize, int n);
    virtual ~OutputStreamD();
    void create(const char* s);
    void write(int* number);
    void close();

};

#endif //IOALG_PROJECT1_OUTPUTSTREAMD_H
