/*
 * InputStream.cpp
 *
 *  Created on: 16/09/2016
 *      Author: Martin
 */

#include "InputStreamD.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

using namespace std;

InputStreamD::InputStreamD(int portionSize, int n) {
    this->n = n;
    index = 0;
    filedesc = -1;
    fileSize = 0;
    int temp = portionSize / getpagesize();
    if(temp == 0) {
        this->portionSize = getpagesize();
    }
    else {
        this->portionSize = temp * getpagesize();
    }
    portionIndex = 0;
}

InputStreamD::~InputStreamD() {
    // TODO Auto-generated destructor stub
}

void InputStreamD::open(char* s) {
    file = fopen(s, "r+");
    if(file == NULL) {
        perror("File is NULL");
    }
    filedesc = fileno(file);
    //filedesc = ::open(s, O_RDWR);
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    map = (int *) mmap(0, portionSize, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, 0);
    if (map == MAP_FAILED) {
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
}

int InputStreamD::readNext() {
    if (index == getpagesize() / sizeof(int)) {
        munmap(map, portionSize);
        portionIndex++;
        map = (int *) mmap(0, portionSize, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, portionIndex*getpagesize());
        //map = (int *) mmap(0, portionSize, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, 0);
        index = 0;
    }

    int elm = map[index];
    index++;
    return elm;
}

bool InputStreamD::endOfStream() {
    return (index >= fileSize / sizeof(int));
}

void InputStreamD::close() {
    fclose(file);
    munmap(map, portionSize);
}
