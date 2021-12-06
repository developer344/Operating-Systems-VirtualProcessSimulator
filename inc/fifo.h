#ifndef _FIFO_H_
#define _FIFO_H_

#include "virtualprocess.h"

//Defining struct fifo node and pointer fifo node
typedef struct FifoNode FifoNode;
typedef struct FifoNode *FifoNodePtr;

typedef struct FifoNode
{
    VPptr VProcess;
    FifoNodePtr nextNode;
} FifoNode;

//Defining struct linked fifo and pointer linked fifo
typedef struct fifo Fifo;
typedef struct fifo *FifoPtr;

typedef struct fifo
{
    FifoNodePtr Begining;
    FifoNodePtr End;
    int numOfElements;
} Fifo;

void fifo_init(FifoPtr fifo);
int fifo_push(FifoPtr FifoOfVPs, VPptr VProcess);
VPptr fifo_pop(FifoPtr FifoOfVPs);
int fifo_getNumOfElements(FifoPtr FifoOfVPs);
int fifo_isEmpty(FifoPtr FifoOfVPs);
void fifo_increase_waiting(FifoPtr FifoOfVPs);
void fifo_deleteFifo(FifoPtr FifoOfVPs);
void fifo_deleteFifoNode(FifoNodePtr FifoNode);

#endif