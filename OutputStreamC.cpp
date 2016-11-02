//
// Created by soren on 9/16/16.
//

#include "OutputStreamC.h"
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <initializer_list>
#include <iostream>
#include <sys/stat.h>

using namespace std;

OutputStreamC::OutputStreamC(int bufferSize) {
    index = 0;
    buffer = new int[bufferSize];
    size = bufferSize;
}

OutputStreamC::~OutputStreamC() {
    // TODO Auto-generated destructor stub
}

void OutputStreamC::create(const char* s) {
    filedesc = ::open(s, O_CREAT|O_RDWR);
    name = s;
}

void OutputStreamC::write(int* number) {
    if (index >= size) {
        int tmp = ::write(filedesc, buffer, sizeof(int)*size);
        if(tmp == -1) {
            cout << "Error writing to file in OutC " << name << "\n";
        }
        index = 0;
    }

    buffer[index] = *number;
    index++;
}

void OutputStreamC::close() {
    if (index > 0) {
        ::write(filedesc, buffer, index * sizeof(int));
    }

    int tmp = ::close(filedesc);
    if(tmp == -1) cout << "Error closing file in OutC\n";
    delete(buffer);
    chmod(name, 0666);
    index = 0;
}