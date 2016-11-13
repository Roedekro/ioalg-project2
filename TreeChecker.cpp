//
// Created by Martin on 11/13/16.
//

#include <iostream>
#include "TreeChecker.h"
#include "InputStream.h"
#include "InputStreamC.h"

TreeChecker::TreeChecker() {
    totalRecords = 0;
}

TreeChecker::~TreeChecker() {}

void TreeChecker::checkNodeRecursive(Node *node, bool print) {



    totalRecords = totalRecords + node->records;



    int main[2];
    int children[4];

    InputStreamC* inPar = new InputStreamC(8192);
    InputStreamC* inC1 = new InputStreamC(8192);
    InputStreamC* inC2 = new InputStreamC(8192);

    inPar->open(node->pages[0].c_str());
    main[0] = inPar->readNext();
    inPar = new InputStreamC(8192);
    if(node->records > 1) {
        inPar->open(node->pages[1].c_str());
        main[1] = inPar->readNext();
    }


    int total = 0;

    if(node->childrenCounter > 0) {
        inC1->open(node->children[0]->pages[0].c_str());
        children[total] = inC1->readNext();
        total++;
        inC1 = new InputStreamC(8192);
        if(node->children[0]->pageCounter > 1) {
            inC1->open(node->children[0]->pages[1].c_str());
            children[total] = inC1->readNext();
            total++;
        }

    }


    if(node->childrenCounter > 1) {
        inC2->open(node->children[1]->pages[0].c_str());
        children[total] = inC2->readNext();
        total++;
        inC2 = new InputStreamC(8192);
        if(node->children[1]->pageCounter > 1) {
            inC2->open(node->children[1]->pages[1].c_str());
            children[total] = inC2->readNext();
            total++;
        }

    }

    if(print) {
        cout << "-----\n";
        cout << "id=" << node->id << '\n';
        cout << "val1=" << main[0] << '\n';
        if(node->records > 1) {
            cout << "val2=" << main[1] << '\n';
        }
    }

    if(node->id != 1) {
        int parentID = node->parent->id;
        if(node->id / 2 != parentID) {
            cout << "Node " << node->id << " has the wrong parent " << parentID << '\n';
        }
    }

    if(node->childrenCounter > 0) {
        if(node->children[0]->id / 2 != node->id) {
            cout << "Node " << node->id << " has the wrong first child " << node->children[0]->id << '\n';
        }
    }

    if(node->childrenCounter > 1) {
        if(node->children[1]->id / 2 != node->id) {
            cout << "Node " << node->id << " has the wrong second child " << node->children[1]->id << '\n';
        }
    }


    if(node->records != node->pageCounter) {
        cout << "Node " << node->id << " has the wrong pageCounter " << node->pageCounter << '\n';
    }

    for(int i = 0; i < total; i++) {
        for(int j = 0; j < node->records; j++) {
            if(children[i] < main[j]) {
                cout << "Error in node " << node->id << " value " << main[j] << " larger than " << children[i] << '\n';
            }
        }
    }



    for(int i = 0; i < node->childrenCounter; i++) {
        checkNodeRecursive(node->children[i],print);
    }

}