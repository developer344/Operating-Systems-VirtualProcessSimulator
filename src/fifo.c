#include <stdlib.h>
#include <string.h>

#include "fifo.h"

void fifo_init(FifoPtr fifo)
{
    fifo->Begining = NULL;
    fifo->End = NULL;
    fifo->numOfElements = 0;
}

int fifo_push(FifoPtr FifoOfVPs, VPptr VProcess)
{
    if (fifo_isEmpty(FifoOfVPs))
    {
        FifoOfVPs->Begining = malloc(sizeof(FifoNode));
        FifoOfVPs->End = FifoOfVPs->Begining;
        FifoOfVPs->End->VProcess = VProcess;
        FifoOfVPs->End->nextNode = NULL;
        FifoOfVPs->numOfElements++;
        return 0;
    }
    else
    {
        FifoOfVPs->End->nextNode = malloc(sizeof(FifoNode));
        FifoOfVPs->End = FifoOfVPs->End->nextNode;
        FifoOfVPs->End->VProcess = VProcess;
        FifoOfVPs->End->nextNode = NULL;
        FifoOfVPs->numOfElements++;
        return 0;
    }
    return -1;
}

VPptr fifo_pop(FifoPtr FifoOfVPs)
{
    if (!fifo_isEmpty(FifoOfVPs))
    {
        int flag = 0;
        FifoNodePtr prevNode = NULL, currNode = FifoOfVPs->Begining;
        while (currNode != NULL)
        {
            if (currNode->VProcess->processId == FifoOfVPs->End->VProcess->processId)
            {
                break;
            }
            prevNode = currNode;
            currNode = currNode->nextNode;
        }

        FifoOfVPs->End = prevNode;
        if (prevNode == NULL)
            FifoOfVPs->Begining = NULL;
        FifoOfVPs->numOfElements--;
        VPptr cc = currNode->VProcess;
        free(currNode);
        return cc;
    }
    return NULL;
}

int fifo_getNumOfElements(FifoPtr FifoOfVPs)
{
    return FifoOfVPs->numOfElements;
}

int fifo_isEmpty(FifoPtr FifoOfVPs)
{
    return fifo_getNumOfElements(FifoOfVPs) == 0;
}

void fifo_increase_waiting(FifoPtr FifoOfVPs)
{
    FifoNodePtr currNode = FifoOfVPs->Begining;
    int count = 0;
    while (currNode != NULL && count < FifoOfVPs->numOfElements)
    {
        currNode->VProcess->timeWaiting++;
        currNode = currNode->nextNode;
        count++;
    }
}

void fifo_deleteFifo(FifoPtr FifoOfVPs)
{
    FifoNodePtr nextNode, currNode = FifoOfVPs->Begining;
    while (currNode != NULL)
    {
        nextNode = currNode->nextNode;
        fifo_deleteFifoNode(currNode);
        currNode = nextNode;
    }
    free(FifoOfVPs);
}

void fifo_deleteFifoNode(FifoNodePtr FifoNode)
{
    free(FifoNode->VProcess);
    free(FifoNode);
}