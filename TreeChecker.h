//
// Created by Martin on 11/13/16.
//

#ifndef IOALG_PROJECT2_TREECHECKER_H
#define IOALG_PROJECT2_TREECHECKER_H


#include "Node.h"

class TreeChecker {
public:
    TreeChecker(int f, int p);
    virtual ~TreeChecker();
    void checkNodeRecursive(Node* node, bool print);
    int totalRecords;
    int fanout;
    int pageSize;
};


#endif //IOALG_PROJECT2_TREECHECKER_H
