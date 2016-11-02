//
// Created by Martin on 11/2/16.
//

#include "ExternalHeapsort.h"

ExternalHeapsort::ExternalHeapsort(int f, int n, int b, int i) {

    fanout = f;
    nodeSize = n;
    blockSize = b;
    internalMemorySize = i;

}

ExternalHeapsort::~ExternalHeapsort() {}