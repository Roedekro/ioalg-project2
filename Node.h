//
// Created by Martin on 11/2/16.
//

#ifndef IOALG_PROJECT2_NODE_H
#define IOALG_PROJECT2_NODE_H

#include <vector>
#include <string>

using namespace std;

class Node {
public:
    int id;
    Node(int m, int p, int id, Node* parent, Node* predecessor, int orderAmongSiblings);
    virtual ~Node();
    string* pages;
    int pageCounter;
    Node* parent;
    Node** children; // Array af pointers
    int childrenCounter;
    string* childrensLastPage;
    int childrensLastPageOffset;
    Node* predecessor;
    int orderAmongSiblings;
    int records;
    bool inSiftDown;
    bool haltedSiftup;
};


#endif //IOALG_PROJECT2_NODE_H
