//
// Created by soren on 9/16/16.
//

#include "InputStream.h"

#ifndef IOALG_PROJECT1_INPUTSTREAMC_H
#define IOALG_PROJECT1_INPUTSTREAMC_H

using namespace std;

class InputStreamC : public InputStream{
public:
    int filedesc;
    int size;
    int * buffer;
    int index;
    int endoffileIndex;
    InputStreamC(int bufferSize);
    virtual ~InputStreamC();
    void open(const char* s);
    int readNext();
    bool endOfStream();
    void close();
};

#endif //IOALG_PROJECT1_INPUTSTREAMC_H
