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
#include "Binary.h"
#include <cstdlib>
#include <sstream>

ExternalHeap::ExternalHeap(int f, int p, int b, int i, int t) {

    fanout = f;
    pageSize = p;
    blockSize = b;
    internalMemorySize = i;
    streamType = t;
    insertBuffer = new int[internalMemorySize+1];
    insertBufferCounter = 0;
    rootPageBuffer = new int[internalMemorySize/fanout+1];
    rootPageBufferCounter = 0;
    rootNode = NULL;
    lastNode = NULL;
    nodeCounter = 0;
    nodeVector = new vector<Node*>;
    mergeIntBuffer = new int[internalMemorySize+1];
    mergeBinBuffer = new BinElement*[internalMemorySize+1];

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

        // Split dem op i <fanout> dele af størrelse pageSize.
        OutputStream* out;
        if(streamType == 1) {
            out = new OutputStreamA();
        }
        else if(streamType == 2) {
            out = new OutputStreamB();
        }
        else if(streamType == 3) {
            out = new OutputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            out = new OutputStreamD(blockSize,pageSize);
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
            if(j > pageSize) {
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
                else if(streamType == 3) {
                    out = new OutputStreamC(blockSize/4);
                }
                else if(streamType == 4) {
                    out = new OutputStreamD(blockSize,pageSize);
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
        if(rootNode == NULL) {
            Node* newNode = new Node(fanout, pageSize, nodeCounter, NULL, NULL, 1);
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
            else if(streamType == 3) {
                in = new InputStreamC(blockSize/4);
            }
            else if(streamType == 4) {
                in = new InputStreamD(blockSize,pageSize);
            }
            string s = newNode->pages[fanout-1];
            const char* test = s.c_str();
            in->open(test);
            for(int i = 0; i < pageSize; i++) {
                rootPageBuffer[i] = in->readNext();
            }
            rootPageBufferCounter = pageSize;
            in->close();
            delete(in);
        }
        else {
            // Ny node i træet
            Node* prevLastNode = lastNode;
            int parentNumber = nodeCounter/2;
            Node* parent = nodeVector->at(parentNumber);
            parent->childrenCounter++;
            Node* newNode = new Node(fanout, pageSize, nodeCounter, parent, lastNode, parent->childrenCounter);
            parent->children[parent->childrenCounter] = newNode;
            parent->childrensLastPage = &newNode->pages[fanout-1];
            parent->childrensLastPageOffset = pageSize;
            newNode->pages = nodePages;
            lastNode = newNode;






            // Check om det tidliger sidste leaf overholder load condition
            // Hvis den ikke gør det skal den bytte plads med det nye leaf,
            // og potentielt skal begge sift up.

            int loadCondition = fanout*pageSize/2;
            if(fanout*pageSize%2 != 0) {
                loadCondition++;
            }
            if(prevLastNode->records < loadCondition) {
                Node* newParent = prevLastNode->parent;
                // Special case hvis den tiligere node var root
                // Så kan vi bare sifte den nye op og alting er nu balanceret
                // Dvs. vi gør ingenting.
                if(newParent != NULL) {

                    lastNode->id--;
                    prevLastNode->id++;
                    newNode->predecessor = prevLastNode->predecessor;
                    prevLastNode->predecessor = newNode;

                    // To scenarier, enten er parents den samme eller ikke
                    if(parent->id == newParent->id) { ;
                        parent->children[parent->childrenCounter-1] = prevLastNode;
                        parent->children[parent->childrenCounter-2] = newNode;
                        parent->childrensLastPage = &prevLastNode->pages[prevLastNode->pageCounter-1];
                        int offset = prevLastNode->records % pageSize;
                        if(offset == 0) offset = pageSize;
                        parent->childrensLastPageOffset = offset;
                    }
                    else {

                        newParent->children[fanout-1] = newNode;
                        newParent->childrensLastPage = &newNode->pages[fanout-1];
                        newParent->childrensLastPageOffset = pageSize;

                        parent->children[0] = prevLastNode;
                        parent->childrensLastPage = &prevLastNode->pages[prevLastNode->pageCounter-1];
                        int offset = prevLastNode->records % pageSize;
                        if(offset == 0) {
                            offset = pageSize;
                        }
                        parent->childrensLastPageOffset = offset;

                        newNode->parent = newParent;
                        prevLastNode->parent = parent;
                        siftup(prevLastNode); // <--------------------------- Vigtigt!
                    }
                }
            }

            // Sift up!
            siftup(newNode);
        }
    }
}

void ExternalHeap::siftup(Node *node) {

    Node* parent = node->parent;
    bool b = true;
    while(b && (parent != NULL)) {

        b = false; // Bliver sat true igen hvis vi henter ints fra child til parent
        // og dermed skal checke på parents parent. Hvis det ikke sker kan vi stoppe.

        // Udregn størrelsen af første til for type D.
        int pC = parent->records % pageSize;
        if(pC == 0) {
            pC = pageSize;
        }
        int cC = node->records % pageSize;
        if(cC == 0) {
            cC = pageSize;
        }

        InputStream* inPar;
        InputStream* inChild;
        OutputStream* outPar;
        OutputStream* outChild;
        if(streamType == 1) {
            inPar = new InputStreamA();
            inChild = new InputStreamA();
            outPar = new OutputStreamA();
            outChild = new OutputStreamA();
        }
        else if(streamType == 2) {
            inPar = new InputStreamB();
            inChild = new InputStreamB();
            outPar = new OutputStreamB();
            outChild = new OutputStreamB();
        }
        else if(streamType == 3) {
            inPar = new InputStreamC(blockSize/4);
            inChild = new InputStreamC(blockSize/4);
            outPar = new OutputStreamC(blockSize/4);
            outChild = new OutputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            inPar = new InputStreamD(blockSize,pageSize);
            inChild = new InputStreamD(blockSize,pageSize);
            outPar = new OutputStreamD(blockSize,pC);
            outChild = new OutputStreamD(blockSize,cC);
        }

        inPar->open(parent->pages[parent->pageCounter-1].c_str());
        inChild->open(node->pages[node->pageCounter-1].c_str());
        outPar->create("outParent");
        outChild->create("outChild");


        // Udregn hvor mange records skal til hhv. parent og child
        // Child skal minimum have fanout*pageSize / 2
        // Men vi prøver at fylde parent op så meget som muligt.
        // Flere records højere oppe i træet betyder at delete ikke
        // når så langt ned.
        int totalRecords = parent->records + node->records;
        int recordsToChild;
        if(totalRecords - fanout*pageSize > fanout*pageSize / 2) {
            recordsToChild = totalRecords - fanout*pageSize;
        }
        else {
            recordsToChild = fanout*pageSize / 2;
        }

        int recordsToParent = totalRecords - recordsToChild;

        // MEN, hvis parent nu er mindre end loadCondition må det betyde at child = lastNode
        // I det tilfælde skal parent overholde loadCondition, og child behøver ikke
        // Check om parent vil overholde loadCondition
        if(recordsToParent < fanout*pageSize / 2) {
            recordsToParent = fanout*pageSize / 2;
            recordsToChild = totalRecords - recordsToParent;
        }

        // Læs sidste page fra parent og child ind i binary
        Binary* bin = new Binary();
        int readFromParent;
        if(parent->records % pageSize == 0) {
            readFromParent = pageSize;
        }
        else {
            readFromParent = parent->records % pageSize;
        }

        for(int i = 0; i < readFromParent; i++) {
            int value = inPar->readNext();
            BinElement* binElement = new BinElement(1,value);
            bin->inheap(mergeBinBuffer, i, binElement);
        }

        int readFromChild;
        if(node->records % pageSize == 0) {
            readFromChild = pageSize;
        }
        else {
            readFromChild = node->records % pageSize;
        }

        for(int i = 0; i < readFromChild; i++) {
            int value = inChild->readNext();
            BinElement* binElement = new BinElement(2,value);
            bin->inheap(mergeBinBuffer, readFromParent+ i, binElement);
        }

        inPar->close();
        delete(inPar);
        inChild->close();
        delete(inChild);

        if(streamType == 1) {
            inPar = new InputStreamA();
            inChild = new InputStreamA();
        }
        else if(streamType == 2) {
            inPar = new InputStreamB();
            inChild = new InputStreamB();
        }
        else if(streamType == 3) {
            inPar = new InputStreamC(blockSize/4);
            inChild = new InputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            inPar = new InputStreamD(blockSize,pageSize);
            inChild = new InputStreamD(blockSize,pageSize);
        }

        int parentCounter = pageSize;
        int childCounter = pageSize;
        int parentPageCounter = parent->pageCounter-1;
        int childPageCounter = node->pageCounter-1;
        int binCounter = readFromParent + readFromChild;

        if(parentPageCounter != 0) {
            inPar->open(parent->pages[parentPageCounter-1].c_str());
        }
        if(childPageCounter != 0) {
            inChild->open(node->pages[childPageCounter-1].c_str());
        }


        // Skriv ints ud til outParent
        for(int i = 0; i < recordsToParent; i++) {
            BinElement* ele = bin->outheap(mergeBinBuffer,binCounter);
            outPar->write(&ele->value);
            binCounter--;
            if(ele->id == 1) {
                parentCounter--;
                if(parentCounter != 0) {
                    // Genbrug element
                    ele->value = inPar->readNext();
                    bin->inheap(mergeBinBuffer, binCounter, ele);
                    binCounter++;
                }
                else {
                    inPar->close();
                    delete(inPar);
                    parentPageCounter--;
                    if(parentPageCounter != 0) {
                        if(streamType == 1) {
                            inPar = new InputStreamA();;
                        }
                        else if(streamType == 2) {
                            inPar = new InputStreamB();
                        }
                        else if(streamType == 3) {
                            inPar = new InputStreamC(blockSize/4);
                        }
                        else if(streamType == 4) {
                            inPar = new InputStreamD(blockSize,pageSize);
                        }
                        inPar->open(parent->pages[parentPageCounter-1].c_str());
                        parentCounter = pageSize-1;
                        ele->value = inPar->readNext();
                        bin->inheap(mergeBinBuffer, binCounter, ele);
                        binCounter++;
                    }
                    else {
                        delete(ele);
                    }

                }

            }
            else {
                b = true; // Vi har hentet et element op fra child, og skal checke videre op i træet
                childCounter--;
                if(childCounter != 0) {
                    // Genbrug element
                    ele->value = inChild->readNext();
                    bin->inheap(mergeBinBuffer, binCounter, ele);
                }
                else {
                    inChild->close();
                    delete(inChild);
                    childPageCounter--;
                    if(childPageCounter != 0) {
                        if(streamType == 1) {
                            inChild = new InputStreamA();;
                        }
                        else if(streamType == 2) {
                            inChild = new InputStreamB();
                        }
                        else if(streamType == 3) {
                            inChild = new InputStreamC(blockSize/4);
                        }
                        else if(streamType == 4) {
                            inChild = new InputStreamD(blockSize,pageSize);
                        }
                        inChild->open(node->pages[childPageCounter-1].c_str());
                        childCounter = pageSize-1;
                        ele->value = inChild->readNext();
                        bin->inheap(mergeBinBuffer, binCounter, ele);
                        binCounter++;
                    }
                    else {
                        delete(ele);
                    }
                }
            }
        }

        outPar->close();
        delete(outPar);

        // Gør det samme, men skriv til child

        for(int i = 0; i < recordsToChild; i++) {
            BinElement* ele = bin->outheap(mergeBinBuffer,binCounter);
            outChild->write(&ele->value);
            binCounter--;
            if(ele->id == 1) {
                parentCounter--;
                if(parentCounter != 0) {
                    // Genbrug element
                    ele->value = inPar->readNext();
                    bin->inheap(mergeBinBuffer, binCounter, ele);
                    binCounter++;
                }
                else {
                    inPar->close();
                    delete(inPar);
                    parentPageCounter--;
                    if(parentPageCounter != 0) {
                        if(streamType == 1) {
                            inPar = new InputStreamA();;
                        }
                        else if(streamType == 2) {
                            inPar = new InputStreamB();
                        }
                        else if(streamType == 3) {
                            inPar = new InputStreamC(blockSize/4);
                        }
                        else if(streamType == 4) {
                            inPar = new InputStreamD(blockSize,pageSize);
                        }
                        inPar->open(parent->pages[parentPageCounter-1].c_str());
                        parentCounter = pageSize-1;
                        ele->value = inPar->readNext();
                        bin->inheap(mergeBinBuffer, binCounter, ele);
                        binCounter++;
                    }
                    else {
                        delete(ele);
                    }

                }

            }
            else {
                childCounter--;
                if(childCounter != 0) {
                    // Genbrug element
                    ele->value = inChild->readNext();
                    bin->inheap(mergeBinBuffer, binCounter, ele);
                }
                else {
                    inChild->close();
                    delete(inChild);
                    childPageCounter--;
                    if(childPageCounter != 0) {
                        if (streamType == 1) {
                            inChild = new InputStreamA();;
                        } else if (streamType == 2) {
                            inChild = new InputStreamB();
                        } else if (streamType == 3) {
                            inChild = new InputStreamC(blockSize / 4);
                        } else if (streamType == 4) {
                            inChild = new InputStreamD(blockSize, pageSize);
                        }
                        inChild->open(node->pages[childPageCounter - 1].c_str());
                        childCounter = pageSize - 1;
                        ele->value = inChild->readNext();
                        bin->inheap(mergeBinBuffer, binCounter, ele);
                        binCounter++;
                    }
                    else {
                        delete(ele);
                    }
                }
            }
        }

        // Ryd pænt op
        outChild->close();
        delete(outChild);
        inPar->close();
        delete(inPar);
        inChild->close();
        delete(inChild);

        // Indlæst ints fra outParent til mergeIntBuffer
        if(streamType == 1) {
            inPar = new InputStreamA();;
        }
        else if(streamType == 2) {
            inPar = new InputStreamB();
        }
        else if(streamType == 3) {
            inPar = new InputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            inPar = new InputStreamD(blockSize,recordsToParent);
        }
        inPar->open("outParent");
        for(int i = 1; i <= recordsToParent; i++) {
            mergeIntBuffer[i] = inPar->readNext();
        }
        inPar->close();
        delete(inPar);

        // Sorter
        MinHeap* min = new MinHeap;
        min->sortDescending(mergeIntBuffer, recordsToParent);

        // Skriv records ud til de rigtige pages i parent
        if(streamType == 1) {
            outPar = new OutputStreamA();;
        }
        else if(streamType == 2) {
            outPar = new OutputStreamB();
        }
        else if(streamType == 3) {
            outPar = new OutputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            outPar = new OutputStreamD(blockSize,pageSize);
        }
        ostringstream oss1;
        ostringstream oss2;
        oss1 << parent->id;
        oss2 << 0;
        string s = "node" + oss1.str() + "page" + oss2.str();
        const char* test = s.c_str();
        outPar->create(test);

        parentPageCounter = recordsToParent % pageSize;
        if(recordsToParent % pageSize != 0) parentPageCounter++;
        parent->pageCounter = parentPageCounter;
        int r = 0;
        int j = 0;

        for(int i = 1; i <= recordsToParent; i++) {
            j++;
            if(j <= pageSize) {
                outPar->write(&mergeIntBuffer[j]);
            }
            else {
                r++;
                outPar->close();
                delete(outPar);
                if(streamType == 1) {
                    outPar = new OutputStreamA();;
                }
                else if(streamType == 2) {
                    outPar = new OutputStreamB();
                }
                else if(streamType == 3) {
                    outPar = new OutputStreamC(blockSize/4);
                }
                else if(streamType == 4) {
                    outPar = new OutputStreamD(blockSize,pageSize);
                }
                ostringstream oss1;
                ostringstream oss2;
                oss1 << parent->id;
                oss2 << r;
                string s = "node" + oss1.str() + "page" + oss2.str();
                const char* test = s.c_str();
                outPar->create(test);
                outPar->write(&mergeIntBuffer[i]);
                j = 1;
            }

        }
        outPar->close();
        delete(outPar);

        // Gør det samme for child

        // Indlæst ints fra outChild til mergeIntBuffer
        if(streamType == 1) {
            inChild = new InputStreamA();;
        }
        else if(streamType == 2) {
            inChild = new InputStreamB();
        }
        else if(streamType == 3) {
            inChild = new InputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            inChild = new InputStreamD(blockSize,recordsToChild);
        }
        inChild->open("inChild");
        for(int i = 1; i <= recordsToChild; i++) {
            mergeIntBuffer[i] = inChild->readNext();
        }
        inChild->close();
        delete(inChild);

        // Sorter
        min->sortDescending(mergeIntBuffer, recordsToChild);

        // Skriv records ud til de rigtige pages i parent
        if(streamType == 1) {
            outChild = new OutputStreamA();;
        }
        else if(streamType == 2) {
            outChild = new OutputStreamB();
        }
        else if(streamType == 3) {
            outChild = new OutputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            outChild = new OutputStreamD(blockSize,pageSize);
        }
        ostringstream oss11;
        ostringstream oss22;
        oss1 << node->id;
        oss2 << 0;
        string s2 = "node" + oss11.str() + "page" + oss22.str();
        const char* test2 = s2.c_str();
        outPar->create(test2);

        childPageCounter = recordsToChild % pageSize;
        if(recordsToChild % pageSize != 0) childPageCounter++;
        node->pageCounter = childPageCounter;
        r = 0;
        j = 0;

        for(int i = 1; i <= recordsToParent; i++) {
            j++;
            if(j <= pageSize) {
                outChild->write(&mergeIntBuffer[j]);
            }
            else {
                r++;
                outChild->close();
                delete(outChild);
                if(streamType == 1) {
                    outChild = new OutputStreamA();;
                }
                else if(streamType == 2) {
                    outChild = new OutputStreamB();
                }
                else if(streamType == 3) {
                    outChild = new OutputStreamC(blockSize/4);
                }
                else if(streamType == 4) {
                    outChild = new OutputStreamD(blockSize,pageSize);
                }
                ostringstream oss1;
                ostringstream oss2;
                oss1 << node->id;
                oss2 << r;
                string s = "node" + oss1.str() + "page" + oss2.str();
                const char* test = s.c_str();
                outChild->create(test);
                outChild->write(&mergeIntBuffer[i]);
                j = 1;
            }

        }
        outChild->close();
        delete(outChild);
        delete(min);

        node->records = recordsToChild;
        parent->records = recordsToParent;
        if(parent->children[parent->childrenCounter-1]->id == node->id) {
            // Opdater pointers i parent
            parent->childrensLastPage = &node->pages[node->pageCounter-1];
            int offset = node->records % pageSize;
            if(offset == 0) offset = pageSize;
            parent->childrensLastPageOffset = offset;
        }

        // Setup for næste iteration
        node = parent;
        parent = node->parent;

        /*
        FEJL
        // Ingen minheap her, da vi kun laver 2way merge. <--- FEJL, skal bruge binary.
        int parInt = inPar->readNext();
        int childInt = inChild->readNext();
        b = false; // True igen hvis vi læser fra barnet til parent
        bool childOrParent = true; // Om vi har læst fra barn eller parent

        // Udregn hvor mange ints skal til parent


        // Fyld mergeBinBuffer op og giv til parent
        // FEJL - bemærk rækkefølgen, mindst er sidst
        for(int i = 1; i <= internalMemorySize+1; i++) {
            if(parInt <= childInt) {
                mergeBinBuffer[i] = parInt;
                if(!inPar->endOfStream()) {
                    parInt = inPar->readNext();
                }
                else {

                }
            }
            else {
                b = true; // Vi læste fra child.
                mergeBinBuffer[i] = parInt;
                if(!inPar->endOfStream()) {
                    parInt = inPar->readNext();
                }
                else {

                }
            }
        }*/

    }

    // Hvis vi nåede hele vejen op til root skal vi genindlæse rootBuffer
    if(parent == NULL) {

        rootPageBufferCounter = rootNode->records % pageSize;
        if(rootPageBufferCounter == 0) {
            rootPageBufferCounter = pageSize;
        }

        InputStream* inRoot;
        if(streamType == 1) {
            inRoot = new InputStreamA();;
        }
        else if(streamType == 2) {
            inRoot = new InputStreamB();
        }
        else if(streamType == 3) {
            inRoot = new InputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            inRoot = new InputStreamD(blockSize,rootPageBufferCounter);
        }
        inRoot->open(rootNode->pages[rootNode->pageCounter-1].c_str());

        for(int i = 1; i <= rootPageBufferCounter; i++) {
            rootPageBuffer[i] = inRoot->readNext();
        }
        inRoot->close();
        delete(inRoot);

    }
}

int ExternalHeap::deleteMin() {


    return 1;
}