//
// Created by Martin on 11/2/16.
//

#include "ExternalHeap.h"
#include "MinHeap.h"
#include "OutputStream.h"
#include "OutputStreamA.h"
#include "OutputStreamB.h"
#include "OutputStreamC.h"
#include "OutputStreamD.h"
#include "InputStream.h"
#include "InputStreamA.h"
#include "InputStreamB.h"
#include "InputStreamC.h"
#include "InputStreamD.h"
#include <cstdlib>
#include <sstream>

ExternalHeap::ExternalHeap(int f, int n, int b, int i, int t) {

    fanout = f;
    nodeSize = n;
    blockSize = b;
    internalMemorySize = i;
    streamType = t;
    insertBuffer = new int[internalMemorySize+1];
    insertBufferCounter = 0;
    rootPageBuffer = new int[internalMemorySize/fanout+1];
    rootNode = NULL;
    lastNode = NULL;
    nodeCounter = 0;
    nodeVector = new vector<Node*>;
    mergeBuffer = new int[internalMemorySize+1];

}

ExternalHeap::~ExternalHeap() {}

void ExternalHeap::insert(int i) {

    if(insertBufferCounter < internalMemorySize+1) {
        insertBufferCounter++;
        insertBuffer[insertBufferCounter] = i;
    }
    else {

        nodeCounter++;
        insertBufferCounter = 1;
        insertBuffer[insertBufferCounter] = i;

        // Først sorter dem i descending order med heapsort
        MinHeap min;
        min.sortDescending(insertBuffer, insertBufferCounter);

        // Split dem op i <fanout> dele af størrelse blockSize.
        OutputStream* out;
        if(streamType == 1) {
            out = new OutputStreamA();
        }
        else if(streamType == 2) {
            out = new OutputStreamB();
        }
        else if(streamType == 2) {
            out = new OutputStreamC(8192);
        }
        else if(streamType == 2) {
            out = new OutputStreamD(32768,blockSize);
        }
        int r = 1;
        int j = 0;
        ostringstream oss1;
        ostringstream oss2;
        oss1 << nodeCounter;
        oss2 << r;
        string s = "node" + oss1.str() + "page" + oss2.str();
        const char* test = s.c_str();
        out->create(test);
        string* nodePages = new string[fanout];
        nodePages[0] = s;

        for(int i = 1; i < internalMemorySize+1; i++) {
            j++;
            if(j > blockSize) {
                // Ny page
                r++;
                out->close();
                delete(out);
                if(streamType == 1) {
                    out = new OutputStreamA();
                }
                else if(streamType == 2) {
                    out = new OutputStreamB();
                }
                else if(streamType == 2) {
                    out = new OutputStreamC(8192);
                }
                else if(streamType == 2) {
                    out = new OutputStreamD(32768,blockSize);
                }
                ostringstream oss1;
                ostringstream oss2;
                oss1 << nodeCounter;
                oss2 << r;
                string s = "node" + oss1.str() + "page" + oss2.str();
                const char* test = s.c_str();
                out->create(test);
                out->write(&insertBuffer[i]);
                j = 1;
                nodePages[r-1] = s;
            }
            else {
                out->write(&insertBuffer[i]);
            }
        }
        out->close();
        delete(out);

        // Special case root
        if(rootNode = NULL) {
            Node* newNode = new Node(fanout, blockSize, nodeCounter, NULL, NULL, 1);
            newNode->pages = nodePages;
            rootNode = newNode;
            lastNode = newNode;
            nodeVector->push_back(newNode);

            // Fyld rootpage buffer op med den sidste page i noden.
            InputStream* in;
            if(streamType == 1) {
                in = new InputStreamA();
            }
            else if(streamType == 2) {
                in = new InputStreamB();
            }
            else if(streamType == 2) {
                in = new InputStreamC(8192);
            }
            else if(streamType == 2) {
                in = new InputStreamD(32768,blockSize);
            }
            string s = newNode->pages[fanout-1];
            const char* test = s.c_str();
            in->open(test);
            for(int i = 0; i < blockSize; i++) {
                rootPageBuffer[i] = in->readNext();
            }
            in->close();
            delete(in);
        }
        else {
            // Ny node i træet
            Node* prevLastNode = lastNode;
            int parentNumber = nodeCounter/2;
            Node* parent = nodeVector->at(parentNumber);
            parent->childrenCounter++;
            Node* newNode = new Node(fanout, blockSize, nodeCounter, parent, lastNode, parent->childrenCounter);
            parent->children[parent->childrenCounter] = newNode;
            parent->childrensLastPage = &newNode->pages[fanout-1];
            parent->childrensLastPageOffset = blockSize;
            newNode->pages = nodePages;
            lastNode = newNode;





            /*
            // Check om det tidliger sidste leaf overholder load condition
            // Hvis den ikke gør det skal den bytte plads med det nye leaf,
            // og begge skal sift up.

            int loadCondition = fanout*blockSize/2;
            if(fanout*blockSize%2 != 0) loadCondition++;
            if(prevLastNode->records < loadCondition) {
                Node* newParent = prevLastNode->parent;
                // Special case hvis den tiligere node var root
                // Så kan vi bare sifte den nye op og alting er nu balanceret
                // Dvs. vi gør ingenting.
                if(newParent != NULL) {

                    // To scenarier, enten er parents den samme eller ikke
                    if(parent->id == newParent->id) {
                        lastNode->predecessor = prevLastNode->predecessor;
                        lastNode->id--;
                        prevLastNode->id++;
                        parent->ch
                    }
                    else {

                    }
                }
            }
             */









            // Sift up!
            siftup(newNode);
        }

    }
}

void ExternalHeap::siftup(Node *node) {

    Node* parent = node->parent;
    bool b = true;
    while(b && (parent != NULL)) {
        // Merge og divide
        InputStream* inPar;
        InputStream* inChild;
        if(streamType == 1) {
            inPar = new InputStreamA();
            inChild = new InputStreamA();
        }
        else if(streamType == 2) {
            inPar = new InputStreamB();
            inChild = new InputStreamB();
        }
        else if(streamType == 2) {
            inPar = new InputStreamC(8192);
            inChild = new InputStreamC(8192);
        }
        else if(streamType == 2) {
            inPar = new InputStreamD(32768,blockSize);
            inChild = new InputStreamD(32768,blockSize);
        }

        inPar->open(parent->pages[parent->pageCounter-1].c_str());
        inChild->open(node->pages[node->pageCounter-1].c_str());
        // Ingen minheap her, da vi kun laver 2way merge.
        int parInt = inPar->readNext();
        int childInt = inChild->readNext();
        b = false; // True igen hvis vi læser fra barnet til parent
        bool childOrParent = true; // Om vi har læst fra barn eller parent

        // Udregn hvor mange ints skal til parent
        // ----------------------------------------------------------------------------------------------------------------------------

        // Fyld mergeBuffer op og giv til parent
        for(int i = 1; i <= internalMemorySize+1; i++) {
            if(parInt <= childInt) {
                mergeBuffer[i] = parInt;
                if(!inPar->endOfStream()) {
                    parInt = inPar->readNext();
                }
                else {

                }
            }
            else {
                b = true; // Vi læste fra child.
                mergeBuffer[i] = parInt;
                if(!inPar->endOfStream()) {
                    parInt = inPar->readNext();
                }
                else {

                }
            }
        }




    }

    // Hvis vi nåede hele vejen op til root skal vi genindlæse rootBuffer
    if(parent == NULL) {

    }
}

int ExternalHeap::deleteMin() {


    return 1;
}