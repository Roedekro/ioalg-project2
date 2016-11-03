//
// Created by soren on 9/16/16.
//

#include "OutputStream.h"

#ifndef IOALG_PROJECT1_OUTPUTSTREAMB_H
#define IOALG_PROJECT1_OUTPUTSTREAMB_H

#include <stdio.h>

using namespace std;

class OutputStreamB : public OutputStream {
public:
    FILE* file;
    OutputStreamB();

    virtual ~OutputStreamB();
    void create(const char* s);
    void write(int* number);
    void close();
};

#endif //IOALG_PROJECT1_OUTPUTSTREAMB_H
