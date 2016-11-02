//
// Created by soren on 9/16/16.
//

#include "OutputStream.h"

#ifndef IOALG_PROJECT1_OUTPUTSTREAMA_H
#define IOALG_PROJECT1_OUTPUTSTREAMA_H

using namespace std;

class OutputStreamA : public OutputStream {
public:
    int filedesc;
    OutputStreamA();
    virtual ~OutputStreamA();
    void create(char* s);
    void write(int* number);
    void close();
};

#endif //IOALG_PROJECT1_OUTPUTSTREAM_H
