//
// Created by martin on 9/20/16.
//

#ifndef IOALG_PROJECT1_INPUTSTREAM_H
#define IOALG_PROJECT1_INPUTSTREAM_H

class InputStream {
public:
    virtual void open(char* s) =0;
    virtual int readNext() =0;
    virtual bool endOfStream() =0;
    virtual void close() =0;
};

#endif //IOALG_PROJECT1_INPUTSTREAM_H
