#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "virtualprocess.h"
#include "linkedList.h"

//This function initializes the linked list that containe the VPs
void list_init(linkedListPtr list)
{
    list->Begining = NULL;
    list->End = NULL;
    list->numOfElements = 0;
}

//This function is used to insert a VProcess in the linked list
int list_insertItem(linkedListPtr linkedListOfVPs, VPptr VProcess)
{

    if (linkedListOfVPs->numOfElements == 0)
    {
        linkedListOfVPs->Begining = malloc(sizeof(listNode));
        linkedListOfVPs->End = linkedListOfVPs->Begining;
        linkedListOfVPs->Begining->VProcess = VProcess;
        linkedListOfVPs->Begining->nextNode = NULL;
        linkedListOfVPs->numOfElements++;
        return 0;
    }
    else if (linkedListOfVPs->numOfElements > 0)
    {
        linkedListOfVPs->End->nextNode = malloc(sizeof(listNode));
        linkedListOfVPs->End = linkedListOfVPs->End->nextNode;
        linkedListOfVPs->End->VProcess = VProcess;
        linkedListOfVPs->End->nextNode = NULL;
        linkedListOfVPs->numOfElements++;
        return 0;
    }
    return -1;
}

VPptr list_deleteItem(linkedListPtr linkedListOfVPs, int processId)
{
    if (!list_isEmpty(linkedListOfVPs))
    {
        int flag = 0;
        listNodePtr prevNode = NULL, currNode = linkedListOfVPs->Begining;
        while (currNode != NULL)
        {
            if (currNode->VProcess->processId == processId)
            {
                flag = 1;
                break;
            }
            if (currNode == linkedListOfVPs->End)
                break;
            prevNode = currNode;
            currNode = currNode->nextNode;
        }
        if (flag)
        {
            if (currNode == linkedListOfVPs->Begining && currNode == linkedListOfVPs->End)
            {
                linkedListOfVPs->Begining = NULL;
                linkedListOfVPs->End = NULL;
                linkedListOfVPs->numOfElements = 0;
            }
            else if (currNode == linkedListOfVPs->Begining)
            {
                linkedListOfVPs->Begining = currNode->nextNode;
                linkedListOfVPs->numOfElements--;
            }
            else if (currNode == linkedListOfVPs->End)
            {
                linkedListOfVPs->End = prevNode;
                linkedListOfVPs->numOfElements--;
            }
            else
            {
                prevNode->nextNode = currNode->nextNode;
                linkedListOfVPs->numOfElements--;
            }
            VPptr cc = malloc(sizeof(VP));
            cc->processId = currNode->VProcess->processId;
            cc->size = currNode->VProcess->size;
            cc->duration = currNode->VProcess->duration;
            cc->startSec = currNode->VProcess->startSec;
            cc->endSec = currNode->VProcess->endSec;
            cc->finished = currNode->VProcess->finished;
            cc->timeWaiting = currNode->VProcess->timeWaiting;
            list_deleteListNode(currNode);
            return cc;
        }
    }
    return NULL;
}

//Reduces duration by 1 sec
void runTimeFunction(linkedListPtr linkedListOfVPs)
{
    listNodePtr currNode = linkedListOfVPs->Begining;
    int count = 0;
    while (currNode != NULL && count < linkedListOfVPs->numOfElements)
    {
        currNode->VProcess->duration--;
        currNode = currNode->nextNode;
        count++;
    }
}

//Increases time in the occupied in memory by 1 sec
void runTimeFunction_formemman(linkedListPtr linkedListOfVPs)
{
    listNodePtr currNode = linkedListOfVPs->Begining;
    int count = 0;
    while (currNode != NULL && count < linkedListOfVPs->numOfElements)
    {
        if (currNode->VProcess->finished == 0)
            currNode->VProcess->duration++;
        currNode = currNode->nextNode;
        count++;
    }
}

void list_VP_Finished(linkedListPtr linkedListOfVPs, int processId, int endSecond)
{
    listNodePtr currNode = linkedListOfVPs->Begining;
    int count = 0;
    while (currNode != NULL && count < linkedListOfVPs->numOfElements)
    {
        if (currNode->VProcess->processId == processId)
        {
            currNode->VProcess->finished = 1;
            currNode->VProcess->endSec = endSecond;
        }
        currNode = currNode->nextNode;
        count++;
    }
}

//Checks if a process has terminated
VPptr terminatedFunction(linkedListPtr linkedListOfVPs)
{
    if (!list_isEmpty(linkedListOfVPs))
    {
        listNodePtr currNode = linkedListOfVPs->Begining;
        int count = 0;
        while (currNode != NULL && count < linkedListOfVPs->numOfElements)
        {
            if (currNode->VProcess->duration == 0)
            {
                VPptr cc = list_deleteItem(linkedListOfVPs, currNode->VProcess->processId);
                return cc;
            }
            currNode = currNode->nextNode;
            count++;
        }
    }
    return NULL;
}

void createLogFile(linkedListPtr linkedListOfVPs, double sumOfEindicator, double avrgSpaceSize, double diviationOfSpaceSize)
{
    char logfilename[25];
    sprintf(logfilename, "log_file.txt");
    printf("logfile: %s\n", logfilename);
    FILE *log_file = fopen(logfilename, "w");
    fprintf(log_file, "The product of time-memory is: %f\n", sumOfEindicator);
    fprintf(log_file, "The average value of the size of the empty spaces in memory per second is: %f\n", avrgSpaceSize);
    fprintf(log_file, "The variance of the size of the empty spaces in memory per second is: %f\n", diviationOfSpaceSize);
    listNodePtr currNode = linkedListOfVPs->Begining;
    while (currNode != NULL)
    {
        if (currNode->VProcess->finished == 1)
            fprintf(log_file, "Process with ID: %d and size: %dKB Started in second: %d and ended in second: %d it ran for %d seconds \n", currNode->VProcess->processId, currNode->VProcess->size, currNode->VProcess->startSec, currNode->VProcess->endSec, currNode->VProcess->duration);
        else
        {
            if (currNode->VProcess->startSec != -1)
                fprintf(log_file, "Process with ID: %d and size: %dKB Started in second: %d and did not finish, it ran for %d seconds \n", currNode->VProcess->processId, currNode->VProcess->size, currNode->VProcess->startSec, currNode->VProcess->duration);
            else
                fprintf(log_file, "Process with ID: %d and size: %dKB did not ran at all\n", currNode->VProcess->processId, currNode->VProcess->size);
        }
        currNode = currNode->nextNode;
    }
    fclose(log_file);
}

//This function is used to get the number of elements in the list
int list_getNumOfElements(linkedListPtr linkedListOfVPs)
{
    return linkedListOfVPs->numOfElements;
}

//This function is used to check whether the linked list is empty or not
int list_isEmpty(linkedListPtr linkedListOfVPs)
{
    return list_getNumOfElements(linkedListOfVPs) == 0;
}

//This function deletes the entire linked list
void list_deleteList(linkedListPtr linkedListOfVPs)
{
    listNodePtr nextNode,
        currNode = linkedListOfVPs->Begining;
    int count = 0;
    while (currNode != NULL && count < linkedListOfVPs->numOfElements)
    {
        nextNode = currNode->nextNode;
        list_deleteListNode(currNode);
        currNode = nextNode;
        count++;
    }
}

//This function deletes the given linked list node
void list_deleteListNode(listNodePtr linkedListNode)
{
    if (linkedListNode->VProcess != NULL)
        VPdestruct(linkedListNode->VProcess);
    free(linkedListNode);
}

void print_list(linkedListPtr ll)
{
    listNodePtr currNode = ll->Begining;
    printf("------------------------------------------------------------\n");
    printf("Totalrunning processes are:%d\n", ll->numOfElements);
    int count = 0;
    while (currNode != NULL && count < ll->numOfElements)
    {
        printf("This is process has id:%d size:%d and duration:%d\n", currNode->VProcess->processId, currNode->VProcess->size, currNode->VProcess->duration);
        currNode = currNode->nextNode;
        count++;
    }
    printf("------------------------------------------------------------\n");
}