//
// Created by Martin on 11/2/16.
//

#include "Node.h"

Node::Node(int m, int p, int id, Node *par, Node *pred, int ord) {

    pageCounter = m;
    parent = par;
    children = new Node*[m];
    childrenCounter = 0;
    childrensLastPage = NULL;
    childrensLastPageOffset = NULL;
    predecessor = pred;
    orderAmongSiblings = ord;
    pages = NULL;
    records = m*p;
}

Node::~Node() {}