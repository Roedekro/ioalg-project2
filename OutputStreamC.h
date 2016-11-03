//
// Created by soren on 9/16/16.
//

#include "OutputStream.h"

#ifndef IOALG_PROJECT1_OUTPUTSTREAMC_H
#define IOALG_PROJECT1_OUTPUTSTREAMC_H

using namespace std;

class OutputStreamC  : public OutputStream{
public:
    int index;
    int * buffer;
    int size;
    int filedesc;
    const char * name;
    OutputStreamC(int bufferSize);
    virtual ~OutputStreamC();
    void create(const char* s);
    void write(int* number);
    void close();
};


#endif //IOALG_PROJECT1_OUTPUTSTREAMC_H
