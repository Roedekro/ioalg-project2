/*
 * Binary.cpp
 *
 *  Created on: 28/09/2016
 *      Author: Martin
 */

// Baseret på Algorithm 232 Heapsort af Williams 1964
// OBS! Bemærk at vi pr. paperet bruger et 1-indekseret array!

#include "Binary.h"
#include "BinElement.h"
#include <cstdlib>
#include <iostream>

using namespace std;

Binary::Binary() {
    comparisons = 0;
}

Binary::~Binary() {
    // TODO Auto-generated destructor stub
}

BinElement* Binary::SWOPHeap(BinElement** a, int n, BinElement* in) {
    BinElement* out = NULL;
    if(n < 1) out = in;
    else if (in->value <= a[1]->value) {
        comparisons++; // Comparison ovenfor
        out = in;}
    else {
        comparisons++; // Comparison ovenfor
        int i = 1;
        a[n+1] = in;
        out = a[1];
        bool b = true;
        int j;
        BinElement* temp;
        BinElement* temp1;
        while(b) {
            j = i+i;
            if(j <= n) {
                temp = a[j];
                temp1 = a[j+1];
                comparisons++;
                if(temp1->value < temp->value) {
                    temp = temp1;
                    j++;
                }
                comparisons++;
                if(temp->value < in->value) {
                    a[i] = temp;
                    i = j;
                }
                else {
                    b = false;
                }
            }
            else {b=false;}
            a[i] = in;
        }
    }
    return out;
}

void Binary::inheap(BinElement** a, int n, BinElement* in) {
    n++;
    int i = n;
    int j;
    bool b = true;
    while(b) {
        if (i > 1) {
            j = i / 2;
            comparisons++;
            if(in->value < a[j]->value) {
                a[i] = a[j];
                i = j;
            }
            else {
                b = false;
            }
        }
        else {b=false;}
        a[i] = in;
    }
}

BinElement* Binary::outheap(BinElement** a, int n) {
    BinElement* in = a[n];
    n--;
    BinElement* out = SWOPHeap(a, n, in);
    return out;
}

void Binary::setheap(BinElement** a, int n) {
    int j = 1;
    bool b = true;
    if(j >= n) b = false;
    while(b) {
        inheap(a,j,a[j+1]);
        j++;
        if(j >= n) b = false;
        //if(j >= n) b = false;
    }
}



