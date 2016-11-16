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
    specialCounter = 0;

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

        /*cout << "---\n";
        for(int i = 1; i<= insertBufferCounter; i++) {
            cout << insertBuffer[i] << '\n';
        }*/

        // Først sorter dem i descending order med heapsort
        MinHeap* min = new MinHeap();
        min->sortDescending(insertBuffer, insertBufferCounter);
        delete(min);

        /*cout << "---\n";
        for(int i = 1; i<= insertBufferCounter; i++) {
            cout << insertBuffer[i] << '\n';
        }
        cout << "---\n";*/

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
                cout << "Wrote int = " << insertBuffer[i] << " to new node page = " << r << '\n';
            }
            else {
                out->write(&insertBuffer[i]);
                cout << "Wrote int = " << insertBuffer[i] << " to new node page = " << r << '\n';
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

    cout << "-SiftUp Initialize node id = " << node->id << '\n';

    Node* parent = node->parent;
    bool b = true;
    while(b && (parent != NULL) && parent->inSiftDown == false) {

        cout << "-SiftUp node id = " << node->id << '\n';

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
        cout << "Insert opened " << parent->pages[parent->pageCounter-1] << '\n';
        inChild->open(node->pages[node->pageCounter-1].c_str());
        cout << "Insert opened " << node->pages[node->pageCounter-1] << '\n';
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

        int readFromParent;
        if(parent->records % pageSize == 0) {
            readFromParent = pageSize;
        }
        else {
            readFromParent = parent->records % pageSize;
        }


        Binary* bin = new Binary();
        for(int i = 0; i < readFromParent; i++) {
            int value = inPar->readNext();
            BinElement* binElement = new BinElement(1,value);
            bin->inheap(mergeBinBuffer, i, binElement);
            cout << "Placed " << value << " in mergeBuffer\n";
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
            cout << "Placed " << value << " in mergeBuffer\n";
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

        int parentCounter = readFromParent;
        int childCounter = readFromChild;
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
                if(parentCounter > 0) {

                   // Slet element
                    delete(ele);

                    /*// Genbrug element
                    ele->value = inPar->readNext();
                    bin->inheap(mergeBinBuffer, binCounter, ele);
                    binCounter++;
                    cout << "Placed " << ele->value << " in mergeBuffer\n";*/
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
                        cout << "Insert parent opened " << parent->pages[parentPageCounter-1] << '\n';
                        parentCounter = pageSize;
                        ele->value = inPar->readNext();
                        bin->inheap(mergeBinBuffer, binCounter, ele);
                        binCounter++;
                        cout << "Placed " << ele->value << " in mergeBuffer\n";
                        for(int j = 0; j < pageSize-1; j++) {
                            int ret = inPar->readNext();
                            BinElement* newEle = new BinElement(1,ret);
                            bin->inheap(mergeBinBuffer,binCounter,newEle);
                            binCounter++;
                            cout << "Placed " << ret << " in mergeBuffer\n";
                        }
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

                    delete(ele);

                    /*// Genbrug element
                    ele->value = inChild->readNext();
                    bin->inheap(mergeBinBuffer, binCounter, ele);
                    binCounter++;
                    cout << "Placed " << ele->value << " in mergeBuffer\n";*/
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
                        cout << "Insert parent opened " << node->pages[childPageCounter-1] << '\n';
                        childCounter = pageSize;
                        ele->value = inChild->readNext();
                        bin->inheap(mergeBinBuffer, binCounter, ele);
                        binCounter++;
                        cout << "Placed " << ele->value << " in mergeBuffer\n";
                        for(int j = 0; j < pageSize-1; j++) {
                            int ret = inChild->readNext();
                            BinElement* newEle = new BinElement(2,ret);
                            bin->inheap(mergeBinBuffer,binCounter,newEle);
                            binCounter++;
                            cout << "Placed " << ret << " in mergeBuffer\n";
                        }
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
        int prev = -1;

        for(int i = 0; i < recordsToChild; i++) {
            BinElement* ele = bin->outheap(mergeBinBuffer,binCounter);
            cout << "To child: " << ele->value << '\n';
            cout << "ID = " << ele->id << '\n';
            outChild->write(&ele->value);
            binCounter--;

            if(ele->value == prev) {
                cout << "!!!!!!!!!!!\n";
            }
            prev = ele->value;

            if(ele->id == 1) {
                parentCounter--;
                if(parentCounter > 0) {

                    delete(ele);

                    /*// Genbrug element
                    ele->value = inPar->readNext();
                    bin->inheap(mergeBinBuffer, binCounter, ele);
                    binCounter++;
                    cout << "Placed " << ele->value << " in mergeBuffer\n";*/
                }
                else {
                    inPar->close();
                    delete(inPar);
                    parentPageCounter--;
                    if(parentPageCounter > 0) {
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
                        cout << "Insert in Child opened " << parent->pages[parentPageCounter-1] << '\n';
                        parentCounter = pageSize;
                        ele->value = inPar->readNext();
                        bin->inheap(mergeBinBuffer, binCounter, ele);

                        binCounter++;
                        cout << "Placed " << ele->value << " in mergeBuffer\n";
                        for(int j = 0; j < pageSize-1; j++) {
                            int ret = inPar->readNext();
                            BinElement* newEle = new BinElement(1,ret);
                            bin->inheap(mergeBinBuffer,binCounter,newEle);
                            binCounter++;
                            cout << "Placed " << ret << " in mergeBuffer\n";
                        }

                    }
                    else {
                        delete(ele);
                    }

                }

            }
            else {
                childCounter--;
                if(childCounter > 0) {

                    delete(ele);

                    /*// Genbrug element
                    ele->value = inChild->readNext();
                    bin->inheap(mergeBinBuffer, binCounter, ele);
                    cout << "Placed " << ele->value << " in mergeBuffer\n";*/
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
                        cout << "Insert in Child opened " << node->pages[childPageCounter - 1] << '\n';
                        childCounter = pageSize;
                        ele->value = inChild->readNext();
                        bin->inheap(mergeBinBuffer, binCounter, ele);
                        binCounter++;
                        cout << "Placed " << ele->value << " in mergeBuffer\n";
                        for(int j = 0; j < pageSize-1; j++) {
                            int ret = inChild->readNext();
                            BinElement* newEle = new BinElement(2,ret);
                            bin->inheap(mergeBinBuffer,binCounter,newEle);
                            binCounter++;
                            cout << "Placed " << ret << " in mergeBuffer\n";
                        }
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
                cout << "Insert wrote to " << test << '\n';
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
                cout << "Wrote int = " << mergeIntBuffer[i] << " to parent page = " << r << '\n';
                cout << "Insert wrote to " << test << '\n';
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
        oss11 << node->id;
        oss22 << 0;
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
                cout << "Wrote int = " << mergeIntBuffer[i] << " to child page = " << r << '\n';
                cout << "Insert wrote to " << test2 << '\n';
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
                const char* test2 = s.c_str();
                outChild->create(test2);
                outChild->write(&mergeIntBuffer[i]);
                j = 1;
                cout << "Wrote int = " << mergeIntBuffer[i] << " to child page = " << r << '\n';
                cout << "Insert wrote to " << test2 << '\n';
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
        if(parent != NULL && parent->inSiftDown) {
            parent->haltedSiftup = true;
        }

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
    cout << "RootPageBuffercouter = " << rootPageBufferCounter << '\n';

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
        min->sortDescending(rootPageBuffer, rootPageBufferCounter);
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

    if(rootNode->records == 0 && rootNode->childrenCounter == 0) {
        return;
    }

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
    if(node->childrenCounter > 0) {
        cout << "child 1 has size " << node->children[0]->records << '\n';
    }


    if(node->childrenCounter == 0) {
        // Special case
        siftdownLeaf(node, true);
    }
    else {

        int inputCounter[node->childrenCounter + 1]; // <---------------------OBS! Skal det news? Eller er det så småt at det kan være op stack?
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
                int val = in->readNext();
                mergeBinBuffer[i] = new BinElement(0, val);
                cout << "SD: Added " << val << " to mergebuffer from Parent\n";

            }
            cout << "page-name " << node->pages[node->pageCounter - 1] << '\n';
            inputStreams[0] = in;
            node->pageCounter--;
        }


        int total = toread;
        int childToRead = 0;

        inputCounter[0] = toread;

        // Gør det samme for børnene
        for (int i = 1; i <= node->childrenCounter; i++) {
            InputStream *in;
            childToRead = node->children[i-1]->records % pageSize;
            if (childToRead == 0) {
                childToRead = pageSize;
            }

            inputCounter[i] = childToRead;

            if (streamType == 1) {
                in = new InputStreamA();;
            } else if (streamType == 2) {
                in = new InputStreamB();
            } else if (streamType == 3) {
                in = new InputStreamC(blockSize / 4);
            } else if (streamType == 4) {
                in = new InputStreamD(blockSize, childToRead);
            }
            cout << "Nodes child id = " << node->children[i - 1]->id << '\n';
            //cout << "Opening: " << node->children[i - 1]->pages[node->children[i - 1]->pageCounter - 1].c_str() << '\n';
            in->open(node->children[i - 1]->pages[node->children[i - 1]->pageCounter - 1].c_str());
            for (int j = 1; j <= childToRead; j++) {
                int val = in->readNext();
                mergeBinBuffer[total + j] = new BinElement(i, val);
                cout << "SD: Added " << val << " from " << node->children[i - 1]->id << '\n';

            }
            cout << "page-name " << node->children[i - 1]->pages[node->children[i - 1]->pageCounter - 1] << '\n';
            inputStreams[i] = in;
            total = total + childToRead;
            node->children[i - 1]->pageCounter--; // Brug som counter til at se om vi senere kan læse mere fra denne
        }

        // Dan en heap
        Binary *bin = new Binary();
        if(total > 1) {

            bin->setheap(mergeBinBuffer, total);
        }


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


        // Kode til hvis vi kun har et enkelt barn, dvs. lastNode
        // så henter vi dem op vi kan nu, og kører en siftdownleaf senere.
        int toNode = node->records;
        for(int i = 0; i< node->childrenCounter; i++) {
            toNode = toNode + node->children[i]->records;
        }
        if(toNode > fanout*pageSize) {
            toNode = fanout*pageSize;
        }

        // Restore points
        int records[fanout+1];
        records[0] = node->records;
        for(int i = 1; i <= node->childrenCounter; i++) {
            records[i] = node->children[i-1]->records;
        }


        // Alternativ siftup løsning
        bool b = false;
        int y = 0;

        int prev = -1;

        // Fyld root ud med ints fra heap
        for (int i = 0; i < toNode; i++) {
            BinElement *ele = bin->outheap(mergeBinBuffer, total);
            out->write(&ele->value);
            total--;

            cout << "Placed " << ele->value << " in file outRoot from HEAPid" << ele->id << '\n';

            if(ele->value == prev) {
                cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
            }
            prev = ele->value;

            if (ele->id != 0) {
                node->children[ele->id - 1]->records--; // Tæl ned nu, ryd op senere
            } else {
                node->records--;
            }

            // Find en ny value og genbrug ele
            if (inputCounter[ele->id] - 1 > 0) {
                inputCounter[ele->id] = inputCounter[ele->id] - 1;
                /*ele->value = inputStreams[ele->id]->readNext();
                bin->inheap(mergeBinBuffer, total, ele);
                total++;*/
                delete(ele);


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
                            in = new InputStreamD(blockSize, pageSize);
                        }
                        in->open(node->pages[node->pageCounter - 1].c_str());
                        ele->value = in->readNext();
                        bin->inheap(mergeBinBuffer, total, ele);
                        total++;
                        cout << "Added " << ele->value << " to mergebuffer from internal-id=" << ele->id << '\n';
                        cout << "page-name " << node->pages[node->pageCounter - 1] << '\n';
                        inputStreams[0] = in;
                        inputCounter[0] = pageSize;
                        node->pageCounter--;
                        for(int j = 0; j < pageSize-1;j++) {
                            int ret = in->readNext();
                            BinElement* newEle = new BinElement(0,ret);
                            bin->inheap(mergeBinBuffer,total,newEle);
                            total++;
                            cout << "Added " << newEle->value << " to mergebuffer from internal-id=" << ele->id << '\n';
                        }
                    }
                    else {
                        delete(ele);
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
                            in = new InputStreamD(blockSize, pageSize);
                        }
                        in->open(node->children[ele->id - 1]->pages[node->children[ele->id - 1]->pageCounter -
                                                                    1].c_str());
                        ele->value = in->readNext();
                        bin->inheap(mergeBinBuffer, total, ele);
                        total++;
                        inputStreams[ele->id] = in;
                        inputCounter[ele->id] = pageSize;

                        cout << "Added " << ele->value << " to mergebuffer from internal-id=" << ele->id << '\n';
                        cout << "page-name " << node->children[ele->id - 1]->pages[node->children[ele->id - 1]->pageCounter - 1] << '\n';
                        node->children[ele->id - 1]->pageCounter--;
                        for(int j = 0; j < pageSize-1;j++) {
                            int ret = in->readNext();
                            BinElement* newEle = new BinElement(ele->id,ret);
                            bin->inheap(mergeBinBuffer,total,newEle);
                            total++;
                            cout << "Added " << newEle->value << " to mergebuffer from internal-id=" << ele->id << '\n';
                        }
                    }
                    else {



                        b = true;
                        y = ele->id;


                        // Kun hvis vi ikke er færdige
                        if(i != toNode - 1 && node->children[ele->id-1]->id != lastNode->id &&
                                node->children[ele->id - 1]->childrenCounter > 0) {
                            // Vigtig special case. Vi er løbet tør for records i et barn, og den mindste int
                            // kan nu potentielt ligge i barnets børn!
                            // Afbryd vores siftdown, siftdown på barnet, og siftdown på denne node igen.
                            // Vi bliver nødt til at afbryde da vi lige nu har records inde i mergebufferen
                            // samt special case hvor et barn er last node.

                            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!----- SPECIAL CASE ----- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                            cout << "Special case for node " << node->id << " because child " << node->children[ele->id-1]->id << " ran out\n";
                            cout << "i = " << i << " and toNode = " << toNode << " and node records = " << node->records << '\n';

                            specialCounter++;

                            // Først afbryd, dvs. skriv til noden de records vi har + de records der ligge i noden

                            /*int c = node->records;
                            if(c > 0) {
                                if (streamType == 1) {
                                    in = new InputStreamA();;
                                } else if (streamType == 2) {
                                    in = new InputStreamB();
                                } else if (streamType == 3) {
                                    in = new InputStreamC(blockSize / 4);
                                } else if (streamType == 4) {
                                    in = new InputStreamD(blockSize, childToRead);
                                }
                                in->open(node->pages[0].c_str());
                            }

                            int q = 0;
                            int p = 0;
                            for(int j = 1; j <= node->records; j++){
                                q++;
                                if(q <= pageSize) {
                                    int ret = in->readNext();
                                    out->write(&ret);
                                }
                                else {
                                    // Load ny page
                                    p++;

                                    in->close();
                                    delete(in);

                                    if (streamType == 1) {
                                        in = new InputStreamA();;
                                    } else if (streamType == 2) {
                                        in = new InputStreamB();
                                    } else if (streamType == 3) {
                                        in = new InputStreamC(blockSize / 4);
                                    } else if (streamType == 4) {
                                        in = new InputStreamD(blockSize, childToRead);
                                    }
                                    in->open(node->pages[p].c_str());
                                    int ret = in->readNext();
                                    out->write(&ret);
                                    q = 1;

                                }
                            }


                            if(c > 0) {
                                in->close();
                                delete(in);
                            }

                            out->close();
                            delete(out);

                            if (toread != 0) {
                                InputStream *in = inputStreams[0];
                                if(inputCounter[0] -1 > 0) {
                                    in->close();
                                    delete (in);
                                }
                            }

                            for (int j = 1; j <= node->childrenCounter; j++) {
                                InputStream *in = inputStreams[j];
                                if (inputCounter[j] - 1 != 0) {
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

                            // Bemærk at vi henter i+1+c records ind

                            for (int j = 1; j <= i+1+c; j++) {
                                int ret = in->readNext();
                                mergeIntBuffer[j] = ret;
                                cout << "Placed " << ret << " in mergeIntBuffer" << '\n';
                            }
                            MinHeap *min = new MinHeap();
                            min->sortDescending(mergeIntBuffer, i+1);
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
                            for (int x = 1; x <= i+1+c; x++) {
                                int val = mergeIntBuffer[x];
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

                            delete(bin);

                            // Bare juster børnenes pageCounter nu.
                            // Hvis de er for små vil det sekundære siftdown på node ordne det

                            node->records = i+1+c;

                            // Ny metode, bare reset records
                            //node->records = records[0];
                            //for(int m = 1; m <= node->childrenCounter; m++) {
                                //node->children[m-1]->records = records[m];
                            //}

                            int pages = node->records / pageSize;
                            if(node->records % pageSize != 0) {
                                pages++;
                            }
                            node->pageCounter = pages;

                            if(node->records > fanout*pageSize) {
                                cout << "FEJL I NODE " << node->id << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                                cout << "i="<<i<<" c="<<c<<'\n';
                            }




                            // Ryd op i børnene, deres records er talt ned
                            // og pages er talt ned. Men pages kan være forkert.

                            for (int j = 0; j < node->childrenCounter; j++) {
                                int pages = node->children[j]->records / pageSize;
                                if (node->children[j]->records % pageSize != 0) {
                                    pages++;
                                }
                                node->children[j]->pageCounter = pages;
                            }

                            //siftdown(node->children[ele->id-1]);


                            // Vi har nu afbrudt siftdown
                            // Kald siftdown på det barn der løb tør for records
                            // Men som en del af et siftdown kan der komme et siftup, så sæt
                            // node->insiftdown = true for at stoppe it siftup til denne node!
                            node->inSiftDown = true;
                            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!----- SPECIAL CASE CLEANED UP<<-----!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                            siftdown(node->children[ele->id-1]);

                            node->inSiftDown = false;
                            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!----- SPECIAL CASE SIFTED CHILD DOWN<<-----!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                            // Kald nu siftdown på node igen
                            siftdown(node);

                            if(node->id <= lastNode->id && node->haltedSiftup) {
                                siftup(node); // Fordi vi bremsede en eventuel siftup
                                node->haltedSiftup = false;
                            }
                            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!----- SPECIAL CASE SLUT -----!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

                            /*if(node->parent != NULL) {
                                node->parent->inSiftDown = true;
                            }
                            siftup(node->children[ele->id -1]);
                            if(node->parent != NULL) {
                                node->parent->inSiftDown = false;
                            }
                            siftdown(node);*/

                            /*if(node->parent != NULL) {
                                node->parent->inSiftDown = true;
                            }
                            siftdown(node->children[ele->id -1]);
                            if(node->parent != NULL) {
                                node->parent->inSiftDown = false;
                            }
                            siftdown(node);*/



                            // Sidste forsøg

                            out->close();
                            delete(out);

                            if (toread != 0) {
                                InputStream *in = inputStreams[0];
                                if(inputCounter[0] -1 > 0) {
                                    in->close();
                                    delete (in);
                                }
                            }

                            for (int j = 1; j <= node->childrenCounter; j++) {
                                InputStream *in = inputStreams[j];
                                if (inputCounter[j] - 1 != 0) {
                                    in->close();
                                    delete (in);
                                }
                            }

                            node->records = records[0];
                            for(int m = 1; m <= node->childrenCounter; m++) {
                                node->children[m-1]->records = records[m];
                            }

                            int pages = node->records / pageSize;
                            if(node->records % pageSize != 0) {
                                pages++;
                            }
                            node->pageCounter = pages;

                            if(node->records > fanout*pageSize) {
                                cout << "FEJL I NODE " << node->id << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                                /*cout << "i="<<i<<" c="<<c<<'\n';*/
                            }

                            // Ryd op i børnene, deres records er talt ned
                            // og pages er talt ned. Men pages kan være forkert.

                            for (int j = 0; j < node->childrenCounter; j++) {
                                int pages = node->children[j]->records / pageSize;
                                if (node->children[j]->records % pageSize != 0) {
                                    pages++;
                                }
                                node->children[j]->pageCounter = pages;
                            }

                            node->inSiftDown = true;
                            siftdown(node->children[ele->id -1]);
                            int argh = node->children[ele->id -1]->records;
                            node->inSiftDown = false;
                            siftdown(node);
                            if(node->id <= lastNode->id && node->haltedSiftup) {
                                siftup(node); // Fordi vi bremsede en eventuel siftup
                                node->haltedSiftup = false;
                            }






                            delete(ele);
                            return;
                        }
                        else {
                            delete(ele);
                        }
                    }
                }
            }
        }
        // Skal gøres her for a flushe bufferen i outputstream
        out->close();
        delete(out);
        delete(bin);

        if (toread != 0 && inputCounter[0]-1 != 0) {
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

        for (int i = 1; i <= toNode; i++) {
            int ret = in->readNext();
            mergeIntBuffer[i] = ret;
            cout << "Placed " << ret << " in mergeIntBuffer" << '\n';
        }
        MinHeap *min = new MinHeap();
        min->sortDescending(mergeIntBuffer, toNode);
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
        for (int i = 1; i <= toNode; i++) {
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



        node->records = toNode;
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

        // Alternativ siftup løsning
        /*if(b && node->childrenCounter >= y) {
            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!----- SPECIAL CASE SIFTDOWN -----!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
            siftdown(node->children[y-1]);
            siftup(node->children[y-1]);
            specialCounter++;
            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!----- SPECIAL CASE SIFTDOWN SLUT -----!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        }*/



        for (int i = 0; i < node->childrenCounter; i++) {
            // Se om vi skal køre siftdown på barnet.
            if (node->children[i]->records < fanout * pageSize / 2) {

                siftdown(node->children[i]);
            }
        }



        if(node->records < fanout*pageSize / 2) {
            siftdownLeaf(node,true);
        }
    }
}


void ExternalHeap::siftdownLeaf(Node *node, bool b) {

    cout << "SiftDownLeaf on node " << node->id << '\n';
    cout << "node has " << node->records << " and lastNode has " << lastNode->records << '\n';

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
                int ret = in->readNext();
                mergeIntBuffer[i*pageSize+j] = ret;
                cout << "SiftDownLeaf: Placed " << ret << " in mergebuffer from " << node->pages[i] << '\n';
            }
            in->close();
            delete(in);
        }

        // Læs igennem lastNode indtil vi når til den første record vi skal stjæle

        if(recordsToSteal > 0) {
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
            in->open(lastNode->pages[0].c_str());

            int buffer = lastNode->records - recordsToSteal;
            int x = 1;
            int c = 0;
            int j = 0;
            int r = 0;

            cout << "LastNode has " << lastNode->records << " stealing " << recordsToSteal << " and skipping " << buffer << '\n';

            for(int i = 0; i < lastNode->records; i++) {
                j++;
                c++;
                if(j <= pageSize) {
                    int ret = in->readNext();
                    if(c > buffer) {
                        mergeIntBuffer[node->records + x] = ret;
                        cout << "SiftDownLeaf: Placed " << ret << " in mergebuffer from " << lastNode->pages[r] << " i="<< i << '\n';
                        x++;
                    }
                }
                else {
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
                    in->open(lastNode->pages[r].c_str());
                    int ret = in->readNext();
                    if(c > buffer) {
                        mergeIntBuffer[node->records + x] = ret;
                        x++;
                        cout << "SiftDownLeaf: Placed " << ret << " in mergebuffer from " <<  lastNode->pages[r] << " i=" << i << '\n';
                    }
                    j = 1;


                }
            }
            in->close();
            delete(in);
        }











        // Læs lastnodes records ind i bufferen

        /*int toread = lastNode->records % pageSize;
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
                in->open(lastNode->pages[lastNode->pageCounter-r-1].c_str());
                int ret = in->readNext();
                mergeIntBuffer[node->records + i] = ret;
                cout << "SiftDownLeaf: Placed " << ret << " in mergebuffer" << '\n';
                j = 1;
                toread = pageSize;
            }
            else {
                int ret = in->readNext();
                mergeIntBuffer[node->records + i] = ret;
                cout << "SiftDownLeaf: Placed " << ret << " in mergebuffer" << '\n';
            }
        }
        in->close();
        delete(in);*/

        MinHeap* min = new MinHeap();
        min->sortDescending(mergeIntBuffer, node->records + recordsToSteal);

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
                    cout << "SiftDownLeaf: Wrote " << mergeIntBuffer[i*pageSize + j] << " to node " << node->id << " page " << i << '\n';
                }
            }
            else {
                for(int j = 1; j <= pageSize; j++) {
                    out->write(&mergeIntBuffer[i*pageSize + j]);
                    cout << "SiftDownLeaf: Wrote " << mergeIntBuffer[i*pageSize + j] << " to node " << node->id << " page " << i << '\n';
                }
            }

            out->close();
            delete(out);

        }

        cout << "Siftdown complete, node now has " << node->records << " records and stole " << recordsToSteal << '\n';

        Node* prevLastNodeParent = lastNode->parent;

        // Ryd op i lastNode
        lastNode->records = lastNode->records - recordsToSteal;
        if(lastNode->records <= 0) {
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

        if(node->records < fanout*pageSize / 2 && node->id != lastNode->id) {
            // False fordi at vi ikke skal sift up i rekursionen men i denne her siftdownleaf!
            siftdownLeaf(node, false);
        }

        if(node->records == 0 && node->id == lastNode->id) {
            checkLastNodeRecursive();
            /*cout << "Deleted node id=" << lastNode->id << '\n';
            lastNode->parent->childrenCounter--;
            lastNode = lastNode->predecessor;
            nodeCounter--;*/
        }

        // Vi har stjålet fra lastNode, og skal muligvis siftup for at overholde heap condition!
        if(b == true && prevLastNodeParent->id != node->parent->id && node->id != 1 && node->id <= lastNode->id) {
            cout << "!!! SiftUp som del af siftdown !!! -------------------------- START!\n";
            siftup(node);
            cout << "!!! SiftUp som del af siftdown !!! -------------------------- SLUT!\n";
        }

    }
    else {
        // Vi kan være last node, og vi kan være i den situation at alle vores records
        // er blevet stjålet. Nedlæg os selv.
        if(lastNode->records == 0) {
            cout << "Deleted node id=" << lastNode->id << '\n';
            lastNode->parent->childrenCounter--;
            lastNode = lastNode->predecessor;
            nodeCounter--;
        }
    }
}

void ExternalHeap::checkLastNodeRecursive() {

    if(lastNode->records == 0) {
        cout << "Deleted node id=" << lastNode->id << '\n';
        lastNode->parent->childrenCounter--;
        lastNode = lastNode->predecessor;
        nodeCounter--;
        checkLastNodeRecursive();
    }
}