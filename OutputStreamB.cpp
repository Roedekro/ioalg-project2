/*
 * OutputStream.cpp
 *
 *  Created on: 16/09/2016
 *      Author: Martin
 */

#include "OutputStreamB.h"
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

OutputStreamB::OutputStreamB() {

}

OutputStreamB::~OutputStreamB() {
    // TODO Auto-generated destructor stub
}

void OutputStreamB::create(char* s) {
    file = fopen(s, "wb");
    if(file == NULL) {
        perror("File StreamB");
        //exit(EXIT_FAILURE);
    }
}

void OutputStreamB::write(int* number) {
    fwrite(number, sizeof(int), 1, file);
}

void OutputStreamB::close() {
    fclose(file);
}