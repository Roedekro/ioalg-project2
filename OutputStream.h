//
// Created by martin on 9/20/16.
//

#ifndef IOALG_PROJECT1_OUTPUTSTREAM_H
#define IOALG_PROJECT1_OUTPUTSTREAM_H

class OutputStream {
public:
    virtual void create(char* s) = 0;
    virtual void write(int* number) = 0;
    virtual void close() = 0;
    //void OutputStream();
    //virtual ~OutputStream();
};

#endif //IOALG_PROJECT1_OUTPUTSTREAM_H
