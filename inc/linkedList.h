#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include "virtualprocess.h"

//Defining struct list node and pointer list node
typedef struct listnode listNode;
typedef struct listnode *listNodePtr;

typedef struct listnode
{
    VPptr VProcess;
    listNodePtr nextNode;
} listNode;

//Defining struct linked list and pointer linked list
typedef struct linkedlist linkedList;
typedef struct linkedlist *linkedListPtr;

typedef struct linkedlist
{
    listNodePtr Begining;
    listNodePtr End;
    int numOfElements;
} linkedList;

void list_init(linkedListPtr list);
int list_insertItem(linkedListPtr linkedListOfVPs, VPptr VProcess);
VPptr list_deleteItem(linkedListPtr linkedListOfVPs, int processId);
void runTimeFunction(linkedListPtr linkedListOfVPs);
void runTimeFunction_formemman(linkedListPtr linkedListOfVPs);
void list_VP_Finished(linkedListPtr linkedListOfVPs, int processId, int endSecond);
void createLogFile(linkedListPtr linkedListOfVPs, double sumOfEindicator, double avrgSpaceSize, double diviationOfSpaceSize);
VPptr terminatedFunction(linkedListPtr linkedListOfVPs);
int list_getNumOfElements(linkedListPtr linkedListOfVPs);
int list_isEmpty(linkedListPtr linkedListOfVPs);
void list_deleteList(linkedListPtr linkedListOfVPs);
void list_deleteListNode(listNodePtr linkedListNode);
void print_list(linkedListPtr ll);

#endif