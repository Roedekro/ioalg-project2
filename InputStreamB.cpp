//
// Created by soren on 9/16/16.
//

#include "InputStreamB.h"
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

InputStreamB::InputStreamB() {

}

InputStreamB::~InputStreamB() {
    // TODO Auto-generated destructor stub
}

void InputStreamB::open(const char* s) {
    file = fopen(s, "rb");
}

int InputStreamB::readNext() {
    int ret;
    int bytesRead = fread(&ret , sizeof(int), 1, file);
    return bytesRead > 0 ? ret : bytesRead;
}

bool InputStreamB::endOfStream() {
    bool b = false;
    int val;
    int bytesRead = fread(&val , sizeof(int), 1, file) * sizeof(int);
    if (bytesRead == 0) b = true;
    fseek(file, -bytesRead, SEEK_CUR);
    return b;
}

void InputStreamB::close() {
    fclose(file);
}
