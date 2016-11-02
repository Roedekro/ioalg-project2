//
// Created by soren on 9/20/16.
//

#include <stdio.h>
#include "InputStream.h"


#ifndef IOALG_PROJECT1_INPUTSTREAMD_H
#define IOALG_PROJECT1_INPUTSTREAMD_H



using namespace std;

class InputStreamD : public InputStream {
public:
    FILE* file;
    int filedesc;
    int * map;
    int index;
    int fileSize;
    int portionSize;
    int portionIndex;
    int n;
    InputStreamD(int portionSize, int n);
    virtual ~InputStreamD();
    void open(char* s);
    int readNext();
    bool endOfStream();
    void close();
};


#endif //IOALG_PROJECT1_INPUTSTREAMD_H
