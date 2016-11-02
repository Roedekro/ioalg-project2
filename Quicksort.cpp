//
// Created by martin on 10/1/16.
//

#include <iostream>
#include "Quicksort.h"


Quicksort::Quicksort() {

}

Quicksort::~Quicksort() {

}

void Quicksort::sort(int *array, int p, int r) {

    if(p<r) {
        int pivot = medianOfMedians(array, p, r);
        int q = partition(array, p, r, pivot);
        sort(array, p, q-1);
        sort(array,q+1, r);
    }
}

int Quicksort::partition(int *array, int p, int r, int x) {

    int pivotIndex;
    // Ondt linear scan efter index, betyder intent i teori, men i praksis er det langsomt.
    for(int i = 0; i < r-p; i++) {
        if(array[p+i] == x) {
            pivotIndex = p+i;
            break;
        }
    }
    // Placer vores pivot længst til højre
    array[pivotIndex] = array[r];
    array[r] = x;

    int i = p-1;
    for(int j = p; j < r; j++) {
        if(array[j] <= x) {
            i++;
            int tempI = array[i];
            int tempJ = array[j];
            array[j] = tempI;
            array[i] = tempJ;
        }
    }
    int tempI = array[i+1];
    array[r] = tempI;
    array[i+1] = x;

    return i+1;
}

// Før median of medians
/*int Quicksort::partition(int *array, int p, int r) {

    int x = array[r];
    int i = p-1;
    for(int j = p; j < r; j++) {
        if(array[j] <= x) {
            i++;
            int tempI = array[i];
            int tempJ = array[j];
            array[j] = tempI;
            array[i] = tempJ;
        }
    }
    int tempI = array[i+1];
    array[r] = tempI;
    array[i+1] = x;

    return i+1;
}*/

int Quicksort::medianOfMedians(int *array, int p, int r) {
    if(r-p > 5) {
        int* a = new int[(r-p)/5];
        for(int i = 0; i < (r-p)/5; i++) {
            a[i] = insertionsort(array,p+i*5,p+i*5+4);
        }
        int ret = medianOfMedians(a,0,(r-p)/5);
        delete(a);
        return ret;
        //return medianOfMedians(a,0,(r-p)/5);
    }
    else { // Insertion sort
        return insertionsort(array, p, r);
    }
}

int Quicksort::insertionsort(int *array, int p, int r) {
    for(int i = 0; i < r-p; i++) {
        int x = array[p+i];
        int y = 0;
        for(int j = 1; j <= i; j++) {
            if(x < array[p+i-j]) {
                int temp = array[p+i-j];
                array[p+i-j] = x;
                array[p+i-y] = temp;
                //std::cout << "Replacing " << temp << " with " << x << "\n";
                y++;
            }
            else {
                break;
            }
        }
    }
    return array[p+((r-p)/2)];
}