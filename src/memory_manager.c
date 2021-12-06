#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include "virtualprocess.h"
#include "linkedList.h"
#include "sharedMemory.h"
#include "memory.h"

int main(int argc, char **argv)
{
    printf("Memory Size is: %s KB\nType of memory placement: %s\nSimulation time: %s seconds\n", argv[1], argv[2], argv[3]);
    int simulationTime = atoi(argv[3]);
    int spaceInMemoryPerSecond[simulationTime];
    //Creating memry
    MemoryPtr memory = malloc(sizeof(Memory));
    memory_init(memory, atoi(argv[1]), argv[2]);
    linkedListPtr logFileList = malloc(sizeof(linkedList));
    list_init(logFileList);
    // Semaphores for generator to memory manager communication
    sem_t *sem_gen_prod = sem_open(SEM_PRODUCER_GEN, O_CREAT, 0660, 0);
    if (sem_gen_prod == SEM_FAILED)
    {
        perror("sem_open/generator producer failed");
        exit(EXIT_FAILURE);
    }
    sem_t *sem_mem_man_cons = sem_open(SEM_CONSUMER_MEM_MAN, O_CREAT, 0660, 1);
    if (sem_mem_man_cons == SEM_FAILED)
    {
        perror("sem_open/memory manager consumer failed");
        exit(EXIT_FAILURE);
    }

    // Semaphores for memory manager to generator communication
    sem_t *sem_mem_man_prod = sem_open(SEM_PRODUCER_MEM_MAN, O_CREAT, 0660, 0);
    if (sem_mem_man_prod == SEM_FAILED)
    {
        perror("sem_open/memory manager producer failed");
        exit(EXIT_FAILURE);
    }
    sem_t *sem_gen_cons = sem_open(SEM_CONSUMER_GEN, O_CREAT, 0660, 1);
    if (sem_gen_cons == SEM_FAILED)
    {
        perror("sem_open/generator consumer failed");
        exit(EXIT_FAILURE);
    }

    //Init of variables
    char message[BLOCK_SIZE];
    memset(message, 0, 15);
    char *block_gen_mem_man;
    block_gen_mem_man = attach_memory_block(BLOCK_GEN_MEM, BLOCK_SIZE);
    if (block_gen_mem_man == NULL)
    {
        printf("ERROR: Can't get block\n");
        exit(EXIT_FAILURE);
    }
    int procID;
    int procSize;
    int procTimeWaited;
    int currSec = 0;
    VPptr currvp;
    while (strcmp(message, "Term"))
    {

        sem_wait(sem_gen_prod);
        strncpy(message, block_gen_mem_man, BLOCK_SIZE);
        sem_post(sem_mem_man_cons);
        if (!strcmp(message, "VPStart"))
        {
            //Getting process ID
            sem_wait(sem_gen_prod);
            strncpy(message, block_gen_mem_man, BLOCK_SIZE);
            sem_post(sem_mem_man_cons);
            procID = atoi(message);
            //Getting process size
            sem_wait(sem_gen_prod);
            strncpy(message, block_gen_mem_man, BLOCK_SIZE);
            sem_post(sem_mem_man_cons);
            procSize = atoi(message);
            //Insterting process to memory if possible
            if (memory_insert_process(memory, procID, procSize) == 0)
            { //Process succesfully inserted
                //Sending appropriate message back
                strcpy(message, "inserted");
            }
            else
            { //Process unsuccesfully inserted
                //Sending appropriate message back
                strcpy(message, "notinserted");
            }
            //Sending back status message for process
            sem_wait(sem_gen_cons);
            strncpy(block_gen_mem_man, message, BLOCK_SIZE);
            sem_post(sem_mem_man_prod);
            if (!strcmp(message, "inserted"))
            {
                //Getting the time thatthe process has waited before entering the memory
                sem_wait(sem_gen_prod);
                strncpy(message, block_gen_mem_man, BLOCK_SIZE);
                sem_post(sem_mem_man_cons);
                procTimeWaited = atoi(message);
                currvp = malloc(sizeof(VP));
                VPInit(currvp, 0, procSize, procID, procTimeWaited);
                list_insertItem(logFileList, currvp);
                currvp->startSec = currSec;
                // print_list(logFileList);
            }
        }
        else if (!strcmp(message, "VPStop"))
        {
            sem_wait(sem_gen_prod);
            strncpy(message, block_gen_mem_man, BLOCK_SIZE);
            sem_post(sem_mem_man_cons);
            procID = atoi(message);
            memory_delete_process(memory, procID);
            list_VP_Finished(logFileList, procID, currSec);
        }
        else if (!strcmp(message, "nextSec"))
        {
            runTimeFunction_formemman(logFileList);
            spaceInMemoryPerSecond[currSec] = memory_countEmptySpaces(memory);
            currSec++;
        }
    }
    int remainingProcInWaitingList;
    sem_wait(sem_gen_prod);
    strncpy(message, block_gen_mem_man, BLOCK_SIZE);
    sem_post(sem_mem_man_cons);
    remainingProcInWaitingList = atoi(message);
    for (int i = 0; i < remainingProcInWaitingList; i++)
    {
        //Getting process ID
        sem_wait(sem_gen_prod);
        strncpy(message, block_gen_mem_man, BLOCK_SIZE);
        sem_post(sem_mem_man_cons);
        procID = atoi(message);
        //Getting process size
        sem_wait(sem_gen_prod);
        strncpy(message, block_gen_mem_man, BLOCK_SIZE);
        sem_post(sem_mem_man_cons);
        procSize = atoi(message);
        //Getting the time that the process has waited before entering the memory
        sem_wait(sem_gen_prod);
        strncpy(message, block_gen_mem_man, BLOCK_SIZE);
        sem_post(sem_mem_man_cons);
        procTimeWaited = atoi(message);
        currvp = malloc(sizeof(VP));
        VPInit(currvp, 0, procSize, procID, procTimeWaited);
        //Adding vp to logfileList
        list_insertItem(logFileList, currvp);
    }
    listNodePtr ccNode = logFileList->Begining;

    //Calculating the product between time and memory which is the indicator E
    double sumOfEindicator = 0.0;
    for (int i = 0; i < logFileList->numOfElements; i++)
    {
        if (ccNode->VProcess->finished == 1)
            sumOfEindicator += ccNode->VProcess->size * ccNode->VProcess->duration;
        else if (ccNode->VProcess->startSec > -1)
            sumOfEindicator += ccNode->VProcess->size * (simulationTime - ccNode->VProcess->startSec);
        ccNode = ccNode->nextNode;
    }
    sumOfEindicator = sumOfEindicator / (double)(atoi(argv[1]) * simulationTime);

    //Calculating the average of the size of the empty space in memory
    double avrgSpaceSize = 0.0;
    for (int i = 0; i < simulationTime; i++)
    {
        avrgSpaceSize += spaceInMemoryPerSecond[i];
    }
    avrgSpaceSize /= ((double)simulationTime);

    //Calculating the diviation of the size of the emty space in memory
    double diviationOfSpaceSize = 0.0;
    for (int i = 0; i < simulationTime; i++)
    {
        diviationOfSpaceSize += (spaceInMemoryPerSecond[i] - avrgSpaceSize) * (spaceInMemoryPerSecond[i] - avrgSpaceSize);
    }
    diviationOfSpaceSize /= ((double)simulationTime - 1.0);
    detach_memory_block(block_gen_mem_man);
    //Creating log file
    createLogFile(logFileList, sumOfEindicator, avrgSpaceSize, diviationOfSpaceSize);
    //print_list(logFileList);
    list_deleteList(logFileList);
    free(logFileList);
    destroy_memory(memory);
    sem_close(sem_gen_cons);
    sem_close(sem_gen_prod);
    sem_close(sem_mem_man_cons);
    sem_close(sem_mem_man_prod);
    return 0;
}