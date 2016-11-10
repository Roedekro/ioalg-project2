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
#include <iostream>

ExternalHeap::ExternalHeap(int f, int p, int b, int i, int t) {

    fanout = f;
    pageSize = p;
    blockSize = b;
    internalMemorySize = i;
    streamType = t;
    insertBuffer = new int[internalMemorySize+1];
    insertBufferCounter = 0;
    rootPageBuffer = new int[pageSize+1];
    rootPageBufferCounter = 0;
    rootNode = NULL;
    lastNode = NULL;
    nodeCounter = 0;
    nodeVector = new vector<Node*>;
    mergeIntBuffer = new int[(fanout+1)*pageSize+1];
    mergeBinBuffer = new BinElement*[(fanout+1)*pageSize+1];

}

ExternalHeap::~ExternalHeap() {}

void ExternalHeap::insert(int i) {

    if(insertBufferCounter+2 < internalMemorySize+1) { // +1 for 1 indeksering, +1 for nyt element
        insertBufferCounter++;
        insertBuffer[insertBufferCounter] = i;
    }
    else {

        nodeCounter++;
        insertBufferCounter++;
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
        int r = 0;
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
                nodePages[r] = s;
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
            cout << s << '\n';
            in->open(test);
            for(int i = 0; i < pageSize; i++) {
                int ret = in->readNext();
                rootPageBuffer[i+1] = ret;
                cout << "Create node: Inserted into RootPageBuffer " << ret << '\n';
                //cout << "Inserted in RootPageBuffer " <<
            }
            rootPageBufferCounter = pageSize;
            in->close();
            delete(in);

            cout << "New rootNode with id = " << newNode->id << '\n';
        }
        else {
            // Ny node i træet
            Node* prevLastNode = lastNode;
            int parentNumber = nodeCounter/2;
            Node* parent = nodeVector->at(parentNumber-1);
            parent->childrenCounter++;
            Node* newNode = new Node(fanout, pageSize, nodeCounter, parent, lastNode, parent->childrenCounter);
            parent->children[parent->childrenCounter-1] = newNode;
            parent->childrensLastPage = &newNode->pages[fanout-1];
            parent->childrensLastPageOffset = pageSize;
            newNode->pages = nodePages;
            lastNode = newNode;

            nodeVector->push_back(newNode);




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
                    lastNode = prevLastNode;

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

            cout << "New Node with id = " << newNode->id << '\n';
            // Sift up!
            siftup(newNode);
        }
        insertBufferCounter = 0;
    }
}

void ExternalHeap::siftup(Node *node) {

    Node* parent = node->parent;
    bool b = true;
    while(b && (parent != NULL)) {

        b = false; // Bliver sat true igen hvis vi henter ints fra child til parent
        // og dermed skal checke på parents parent. Hvis det ikke sker kan vi stoppe.

        // Udregn størrelsen af første page for type D.
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
        int parentPageCounter = parent->pageCounter;
        int childPageCounter = node->pageCounter;
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
            cout << "To parent: " << ele->value << '\n';
            cout << "ID = " << ele->id << '\n';
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
                        parentCounter = pageSize;
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
                    binCounter++;
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
                        childCounter = pageSize;
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
            cout << "To child: " << ele->value << '\n';
            cout << "ID = " << ele->id << '\n';
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

        // De er udkommenteret da de er lukkede og deletet ovenfor
        //inPar->close();
        //delete(inPar);
        //inChild->close();
        //delete(inChild);


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

        /* Gøres til sidst
        parentPageCounter = recordsToParent % pageSize;
        if(recordsToParent % pageSize != 0) parentPageCounter++;
        parent->pageCounter = parentPageCounter;*/
        int r = 0;
        int j = 0;

        for(int i = 1; i <= recordsToParent; i++) {
            j++;
            if(j <= pageSize) {
                outPar->write(&mergeIntBuffer[i]);
                cout << "Wrote int = " << mergeIntBuffer[i] << " to parent page = " << r << '\n';
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
        inChild->open("outChild");
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

        /* Gøres til sidst
        childPageCounter = recordsToChild % pageSize;
        if(recordsToChild % pageSize != 0) childPageCounter++;
        node->pageCounter = childPageCounter;*/
        r = 0;
        j = 0;

        for(int i = 1; i <= recordsToChild; i++) {
            j++;
            if(j <= pageSize) {
                outChild->write(&mergeIntBuffer[i]);
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

        // Opdater records og pageCounter
        node->records = recordsToChild;
        parent->records = recordsToParent;
        if(parent->children[parent->childrenCounter-1]->id == node->id) {
            // Opdater pointers i parent
            parent->childrensLastPage = &node->pages[node->pageCounter-1];
            int offset = node->records % pageSize;
            if(offset == 0) offset = pageSize;
            parent->childrensLastPageOffset = offset;
        }

        // Opdater pages
        int pages = node->records / pageSize;
        if(node->records % pageSize != 0) {
            pages++;
        }
        node->pageCounter = pages;

        pages = parent->records / pageSize;
        if(parent->records % pageSize != 0) {
            pages++;
        }
        parent->pageCounter = pages;

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
        cout << "Updating RootPageBuffer " <<'\n';
        cout << "Root ID = " << rootNode->id << '\n';
        cout << "PageCounter = " << rootNode->pageCounter << '\n';



        inRoot->open(rootNode->pages[rootNode->pageCounter-1].c_str());

        for(int i = 1; i <= rootPageBufferCounter; i++) {
            int ret = inRoot->readNext();
            rootPageBuffer[i] = ret;
            cout << "Inserted into RootPageBuffer " << ret << '\n';
        }
        inRoot->close();
        delete(inRoot);

    }
}

int ExternalHeap::deleteMin() {

    cout << "DeleteMin content of rootBuffer " << rootPageBuffer[1] << '\n';
    cout << "RootPageBufferCouter = " << rootPageBufferCounter << '\n';

    // Sorter både insertBuffer og rootBuffer
    // Brug descending da den er hurtigere and ascending
    int ret;
    int insertInt = -1;
    int rootInt = -1;

    MinHeap* min = new MinHeap();
    if(insertBufferCounter > 0) {
        min->sortDescending(insertBuffer, insertBufferCounter);
        insertInt = insertBuffer[insertBufferCounter];
    }
    if(rootPageBufferCounter > 0) {
        //min->sortDescending(rootPageBuffer, rootPageBufferCounter); // <--- Nødvendig? Fjern i heapsort for mere speed
        rootInt = rootPageBuffer[rootPageBufferCounter];
    }

    // Sammenlign og find mindste
    if(insertInt != -1 && rootInt != -1) {
        if(insertInt < rootInt) {
            ret = insertInt;
            insertBufferCounter--;
        }
        else {
            ret = rootInt;
            rootPageBufferCounter--;
            deleteFromRoot();
        }
    }
    else if(insertInt != -1) {
        ret = insertInt;
        insertBufferCounter--;
    }
    else {
        ret = rootInt;
        rootPageBufferCounter--;
        deleteFromRoot();
    }

    cout << "DeleteMin returning " << ret << '\n';
    cout << "DeleteMin insertInt " << insertInt << '\n';
    cout << "DeleteMin rootInt " << rootInt << '\n';
    cout << "DeleteMin insertBufferCounter " << insertBufferCounter << '\n';
    cout << "DeleteMin rootPageBufferCounter " << rootPageBufferCounter << '\n';

    return ret;
}

void ExternalHeap::deleteFromRoot() {

    // Vi er i denne her nødt til at slette med det samme, da der kan komme siftups
    // Ellers kan vi vente med at slette

    // Men vi kan lave et "trick" ved bare at tælle records ned med mindre det skaber et
    // page-shift eller at vi ikke længere overholder loadCondition.

    bool read = false;

    rootNode->records--;
    if(rootNode->records % pageSize != 0) {
        //rootNode->records--;
        cout << "Decreased root records" << '\n';
    }
    else {
        // Læs ny side ind
        //rootNode->records--;
        rootNode->pageCounter--;
        read = true;
    }

    if(rootNode->records < fanout*pageSize/2) {
        siftdown(rootNode);
        read = true;
    }

    if(read) {

        int toRead = rootNode->records % pageSize;
        if(toRead == 0) {
            toRead = pageSize;
        }

        InputStream* in;
        if(streamType == 1) {
            in = new InputStreamA();;
        }
        else if(streamType == 2) {
            in = new InputStreamB();
        }
        else if(streamType == 3) {
            in = new InputStreamC(blockSize/4);
        }
        else if(streamType == 4) {
            in = new InputStreamD(blockSize,toRead);
        }
        in->open(rootNode->pages[rootNode->pageCounter-1].c_str());

        for(int i = 1; i <= toRead; i++) {
            int ret = in->readNext();
            rootPageBuffer[i] = ret;
            cout << "DeleteFromRoot inserted into rootbuffer " << ret << '\n';
        }
        rootPageBufferCounter = toRead;
        in->close();
        delete(in);
    }
}

void ExternalHeap::siftdown(Node* node) {

    cout << "siftdown on node " << node->id << '\n';
    cout << "#records = " << node->records << " #children = " << node->childrenCounter << '\n';


    if(node->childrenCounter == 0) {
        // Special case
        siftdownLeaf(node);
    }
    else {

        int inputCounter[node->childrenCounter +
                         1]; // <------------------------------------ OBS! Skal det news? Eller er det så småt at det kan være op stack?
        InputStream *inputStreams[node->childrenCounter + 1];

        int toread = 0;
        InputStream *in;
        if (node->records != 0) {
            // Først en inputStream til node

            toread = node->records % pageSize;
            if (toread == 0 && node->records != 0) {
                toread = pageSize;
            }

            if (streamType == 1) {
                in = new InputStreamA();
            } else if (streamType == 2) {
                in = new InputStreamB();
            } else if (streamType == 3) {
                in = new InputStreamC(blockSize / 4);
            } else if (streamType == 4) {
                in = new InputStreamD(blockSize, toread);
            }
            in->open(node->pages[node->pageCounter - 1].c_str());
            for (int i = 1; i <= toread; i++) {
                mergeBinBuffer[i] = new BinElement(0, in->readNext());
            }
            inputStreams[0] = in;
            node->pageCounter--;
        }


        int total = toread;

        inputCounter[0] = 1;

        // Gør det samme for børnene
        for (int i = 1; i <= node->childrenCounter; i++) {
            InputStream *in;
            int toread = node->children[i - 1]->records % pageSize;
            if (toread == 0) {
                toread = pageSize;
            }

            inputCounter[i] = 1;

            if (streamType == 1) {
                in = new InputStreamA();;
            } else if (streamType == 2) {
                in = new InputStreamB();
            } else if (streamType == 3) {
                in = new InputStreamC(blockSize / 4);
            } else if (streamType == 4) {
                in = new InputStreamD(blockSize, toread);
            }
            cout << "Nodes child id = " << node->children[i - 1]->id << '\n';
            //cout << "Opening: " << node->children[i - 1]->pages[node->children[i - 1]->pageCounter - 1].c_str() << '\n';
            in->open(node->children[i - 1]->pages[node->children[i - 1]->pageCounter - 1].c_str());
            for (int j = 1; j <= toread; j++) {
                mergeBinBuffer[total + j] = new BinElement(i, in->readNext());
            }
            inputStreams[i] = in;
            total = total + toread;
            node->children[i - 1]->pageCounter--; // Brug som counter til at se om vi senere kan læse mere fra denne
        }

        // Dan en heap
        Binary *bin = new Binary();
        bin->setheap(mergeBinBuffer, total);

        // Outstream til root
        OutputStream *out;
        if (streamType == 1) {
            out = new OutputStreamA();;
        } else if (streamType == 2) {
            out = new OutputStreamB();
        } else if (streamType == 3) {
            out = new OutputStreamC(blockSize / 4);
        } else if (streamType == 4) {
            out = new OutputStreamD(blockSize, fanout * pageSize);
        }
        out->create("outRoot");

        // Fyld root ud med ints fra heap
        for (int i = 0; i < fanout * pageSize; i++) {
            BinElement *ele = bin->outheap(mergeBinBuffer, total);
            out->write(&ele->value);
            total--;

            cout << "Placed " << ele->value << " in file outRoot" << '\n';

            if (ele->id != 0) {
                node->children[ele->id - 1]->records--; // Tæl ned nu, ryd op senere
            } else {
                node->records--;
            }

            // Find en ny value og genbrug ele
            if (inputCounter[ele->id] - 1 != 0) {
                inputCounter[ele->id] = inputCounter[ele->id] - 1;
                ele->value = inputStreams[ele->id]->readNext();
                bin->inheap(mergeBinBuffer, total, ele);
                total++;


            } else {
                // Skift page

                // Luk gammel in
                InputStream *in = inputStreams[ele->id];
                in->close();
                delete (in);

                // Hvis der er flere pages at indlæse
                if (ele->id == 0) {
                    if (node->pageCounter > 0) {
                        if (streamType == 1) {
                            in = new InputStreamA();;
                        } else if (streamType == 2) {
                            in = new InputStreamB();
                        } else if (streamType == 3) {
                            in = new InputStreamC(blockSize / 4);
                        } else if (streamType == 4) {
                            in = new InputStreamD(blockSize, toread);
                        }
                        in->open(node->pages[node->pageCounter - 1].c_str());
                        ele->value = in->readNext();
                        bin->inheap(mergeBinBuffer, total, ele);
                        total++;
                        inputStreams[0] = in;
                        inputCounter[0] = pageSize - 1;
                        node->pageCounter--;
                    }
                    // Ellers gør intet, uden et nyt element i bin vil vi aldrig kigge på root igen
                } else {
                    if (node->children[ele->id - 1]->pageCounter > 0) {
                        if (streamType == 1) {
                            in = new InputStreamA();;
                        } else if (streamType == 2) {
                            in = new InputStreamB();
                        } else if (streamType == 3) {
                            in = new InputStreamC(blockSize / 4);
                        } else if (streamType == 4) {
                            in = new InputStreamD(blockSize, toread);
                        }
                        in->open(node->children[ele->id - 1]->pages[node->children[ele->id - 1]->pageCounter -
                                                                    1].c_str());
                        ele->value = in->readNext();
                        bin->inheap(mergeBinBuffer, total, ele);
                        total++;
                        inputStreams[ele->id] = in;
                        inputCounter[ele->id] = pageSize - 1;
                        node->children[ele->id - 1]->pageCounter--;
                    }
                    // Ellers gør intet, uden et nyt element i bin vil vi aldrig kigge på denne node igen
                }
            }
        }
        // Skal gøres her for a flushe bufferent i outputstream
        out->close();
        delete(out);

        if (toread != 0) {
            InputStream *in = inputStreams[0];
            in->close();
            delete (in);
        }

        for (int i = 1; i <= node->childrenCounter; i++) {
            InputStream *in = inputStreams[i];
            if (inputCounter[i] - 1 != 0) {
                in->close();
                delete (in);
            }
        }

        if (streamType == 1) {
            in = new InputStreamA();
        } else if (streamType == 2) {
            in = new InputStreamB();
        } else if (streamType == 3) {
            in = new InputStreamC(blockSize / 4);
        } else if (streamType == 4) {
            in = new InputStreamD(blockSize, fanout * pageSize);
        }
        in->open("outRoot");

        // Skrev tidligere til node i sorteret rækkefølge
        // mind stil størst, men skal skrives størst til mindst!

        for (int i = 1; i <= fanout * pageSize; i++) {
            int ret = in->readNext();
            mergeIntBuffer[i] = ret;
            cout << "Placed " << ret << " in mergeIntBuffer" << '\n';
        }
        MinHeap *min = new MinHeap();
        min->sortDescending(mergeIntBuffer, fanout * pageSize);
        delete (min);


        if (streamType == 1) {
            out = new OutputStreamA();
        } else if (streamType == 2) {
            out = new OutputStreamB();
        } else if (streamType == 3) {
            out = new OutputStreamC(blockSize / 4);;
        } else if (streamType == 4) {
            out = new OutputStreamD(blockSize, pageSize);
        }
        ostringstream oss1;
        ostringstream oss2;
        oss1 << node->id;
        oss2 << 0;
        string s = "node" + oss1.str() + "page" + oss2.str();
        const char *test = s.c_str();
        out->create(test);

        int r = 0;
        int j = 0;
        for (int i = 1; i <= fanout * pageSize; i++) {
            int val = mergeIntBuffer[i];
            j++;
            if( j <= pageSize) {
                out->write(&val);
                cout << "DeleteMin writing val=" << val << " to node id=" << node->id << " page=" << r << '\n';
            }
            else {
                out->close();
                delete (out);

                r++;
                if (streamType == 1) {
                    out = new OutputStreamA();
                } else if (streamType == 2) {
                    out = new OutputStreamB();
                } else if (streamType == 3) {
                    out = new OutputStreamC(blockSize / 4);;
                } else if (streamType == 4) {
                    out = new OutputStreamD(blockSize, pageSize);
                }
                ostringstream oss1;
                ostringstream oss2;
                oss1 << node->id;
                oss2 << r;
                string s = "node" + oss1.str() + "page" + oss2.str();
                const char *test = s.c_str();
                out->create(test);

                cout << "DeleteMin writing val=" << val << " to node id=" << node->id << " page=" << r << '\n';
                out->write(&val);
                j = 1;
            }
        }

        out->close();
        delete (out);
        in->close();
        delete (in);


        /* Gammel kode, virkede ikke korrekt, mindre detalje med outstream rækkefølge
        int r = 0;
        int j = pageSize;
        for (int i = 1; i <= fanout * pageSize; i++) {
            int val = mergeIntBuffer[i];
            j++;
            if (j > pageSize) {
                out->close();
                delete (out);

                if (streamType == 1) {
                    out = new OutputStreamA();
                } else if (streamType == 2) {
                    out = new OutputStreamB();
                } else if (streamType == 3) {
                    out = new OutputStreamC(blockSize / 4);;
                } else if (streamType == 4) {
                    out = new OutputStreamD(blockSize, pageSize);
                }
                ostringstream oss1;
                ostringstream oss2;
                oss1 << node->id;
                oss2 << r;
                string s = "node" + oss1.str() + "page" + oss2.str();
                const char *test = s.c_str();
                out->create(test);

                cout << "DeleteMin writing val=" << val << " to node id=" << node->id << " page=" << r << '\n';
                out->write(&val);
                j = 1;
                r++;

            } else {
                out->write(&val);
                cout << "DeleteMin writing val=" << val << " to node id=" << node->id << " page=" << r << '\n';
            }
        }*/



        node->records = fanout * pageSize;
        int pages = node->records / pageSize;
        if(node->records % pageSize != 0) {
            pages++;
        }
        node->pageCounter = pages;

        // Ryd op i børnene, deres records er talt ned
        // og pages er talt ned. Men pages kan være forkert.

        for (int i = 0; i < node->childrenCounter; i++) {
            int pages = node->children[i]->records / pageSize;
            if (node->children[i]->records % pageSize != 0) {
                pages++;
            }
            node->children[i]->pageCounter = pages;
            // Kan update last page, men vi bruger den ikke?
        }

        for (int i = 0; i < node->childrenCounter; i++) {
            // Se om vi skal køre siftdown på barnet.
            if (node->children[i]->records < fanout * pageSize / 2) {

                siftdown(node->children[i]);
            }
        }
    }
}


void ExternalHeap::siftdownLeaf(Node *node) {

    cout << "SiftDownLeaf on node " << node->id << '\n';

    if(node->id != lastNode->id) {

        // Stjæl records fra sidste node
        int recordsToSteal = fanout*pageSize - node->records; // Fyld den op
        if(lastNode->records < recordsToSteal) {
            recordsToSteal = lastNode->records; // Hvis vi ikke kan fylde helt op, så tag dem alle
        }




        // Læs nodes records ind i bufferen
        for(int i = 0; i < node->pageCounter; i++) {

            int counter;
            if(i == node->pageCounter-1) {
                counter = node->records % pageSize;
                if(counter == 0) {
                    counter = pageSize;
                }
            }
            else {
                counter = pageSize;
            }

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
                in = new InputStreamD(blockSize,counter);
            }
            in->open(node->pages[i].c_str());

            for(int j = 1; j <= counter; j++) {
                mergeIntBuffer[i*pageSize+j] = in->readNext();
            }
            in->close();
            delete(in);
        }

        // Læs lastnodes records ind i bufferen

        int toread = lastNode->records % pageSize;
        if(toread == 0) {
            toread = pageSize;
        }
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
            in = new InputStreamD(blockSize,toread);
        }
        in->open(lastNode->pages[lastNode->pageCounter-1].c_str());

        int r = 0;
        int j = 0;
        for(int i = 1; i<=recordsToSteal; i++) {
            j++;
            if(j > toread) {
                in->close();
                delete(in);
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
                r++;
                in->open(lastNode->pages[lastNode->pageCounter-r].c_str());
                mergeIntBuffer[node->records + i] = in->readNext();
                j = 1;
                toread = pageSize;
            }
            else {
                mergeIntBuffer[node->records + i] = in->readNext();
            }
        }
        in->close();
        delete(in);

        MinHeap* min = new MinHeap();
        min->sortAscending(mergeIntBuffer, node->records + recordsToSteal);

        node->records = node->records + recordsToSteal;
        int pages = node->records / pageSize;
        if(node->records % pageSize != 0) {
            pages++;
        }
        node->pageCounter = pages;

        for(int i = 0; i < pages; i++) {
            OutputStream* out;
            if(streamType == 1) {
                out = new OutputStreamA();
            }
            else if(streamType == 2) {
                out = new OutputStreamB();
            }
            else if(streamType == 3) {
                out = new OutputStreamC(blockSize/4);;
            }
            else if(streamType == 4) {
                out = new OutputStreamD(blockSize,pageSize);
            }
            ostringstream oss1;
            ostringstream oss2;
            oss1 << node->id;
            oss2 << i;
            string s = "node" + oss1.str() + "page" + oss2.str();
            const char* test = s.c_str();
            out->create(test);
            if(i == pages-1) {
                int counter = node->records % pageSize;
                if(counter == 0) {
                    counter = pageSize;
                }
                for(int j = 1; j <= counter; j++) {
                    out->write(&mergeIntBuffer[i*pageSize + j]);
                }
            }
            else {
                for(int j = 1; j <= pageSize; j++) {
                    out->write(&mergeIntBuffer[i*pageSize + j]);
                }
            }

            out->close();
            delete(out);

        }

        // Ryd op i lastNode
        lastNode->records = lastNode->records - recordsToSteal;
        if(lastNode->records == 0) {
            Node* parent = lastNode->parent;
            parent->childrenCounter--;
            // Kan her opdateres lastChildsPage med mere, men vi bruger det ikke
            cout << "Deleted node id=" << lastNode->id << '\n';
            lastNode = lastNode->predecessor;
            nodeCounter--;
        }
        else {
            int pages = lastNode->records / pageSize;
            if(lastNode->records % pageSize > 0) {
                pages++;
            }
            lastNode->pageCounter = pages;
        }


        // Check om vi nu overholder loadCondition

        if(node->records < fanout*pageSize / 2) {
            siftdownLeaf(node);
        }

    }
}