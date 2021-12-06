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
#include "fifo.h"

int sizegenerator(int low, int high)
{
    return ceil((rand() % (high - low + 1)) + low);
}

int exponantialDistributionGenerator(double lambda)
{
    lambda = 1 / lambda;
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return ceil(-log2(1 - u) / lambda);
}

int main(int argc, char **argv)
{ //Checking the number of arguments
    if (argc != 8)
    {
        perror("ERROR: Wrong number of arguments!\n");
        exit(1);
    }
    if (strcmp(argv[7], "best-fit") && strcmp(argv[7], "worst-fit") && strcmp(argv[7], "buddy"))
    {
        printf("Memory placement must be best-fit, worst-fit or buddy\n");
    }
    if (!strcmp(argv[7], "buddy"))
    {
        if ((ceil(log2(atoi(argv[5]))) != floor(log2(atoi(argv[5])))))
        {
            printf("When using buddy memory placement memory size must be a power of 2!!\n");
            printf("Please try again with a size like 1-2-4-16-32-64-128-256-512-1024 etc!!\n");
            exit(1);
        }
    }
    // Unlinking semaphore names before they are opened.
    sem_unlink(SEM_PRODUCER_GEN);
    sem_unlink(SEM_CONSUMER_MEM_MAN);
    sem_unlink(SEM_PRODUCER_MEM_MAN);
    sem_unlink(SEM_CONSUMER_GEN);
    // Semaphores for generator to memory manager communication
    sem_t *sem_mem_man_cons = sem_open(SEM_CONSUMER_MEM_MAN, O_CREAT | O_EXCL, 0660, 1);
    if (sem_mem_man_cons == SEM_FAILED)
    {
        perror("sem_open/memory manager consumer failed\n");
        exit(EXIT_FAILURE);
    }
    sem_t *sem_gen_prod = sem_open(SEM_PRODUCER_GEN, O_CREAT | O_EXCL, 0660, 0);
    if (sem_gen_prod == SEM_FAILED)
    {
        perror("sem_open/generator producer failed\n");
        exit(EXIT_FAILURE);
    }

    // Semaphores for memory manager to generator communication
    sem_t *sem_gen_cons = sem_open(SEM_CONSUMER_GEN, O_CREAT | O_EXCL, 0660, 1);
    if (sem_gen_cons == SEM_FAILED)
    {
        perror("sem_open/generator consumer failed\n");
        exit(EXIT_FAILURE);
    }
    sem_t *sem_mem_man_prod = sem_open(SEM_PRODUCER_MEM_MAN, O_CREAT | O_EXCL, 0660, 0);
    if (sem_mem_man_prod == SEM_FAILED)
    {
        perror("sem_open/memory manager producer failed\n");
        exit(EXIT_FAILURE);
    }

    int lo, hi;
    float avrg_time_to_live_T;
    float avrg_time_to_arrive_t;
    int memorySize;
    int simulationTime;
    char memoryplacement[15];
    lo = atoi(argv[1]);
    hi = atoi(argv[2]);
    avrg_time_to_live_T = atof(argv[3]);
    avrg_time_to_arrive_t = atof(argv[4]);
    memorySize = atoi(argv[5]);
    simulationTime = atoi(argv[6]);
    strcpy(memoryplacement, argv[7]);

    printf("Creating memory manager\n");
    //Creating memory manager
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        printf("Error in fork\n");
    }
    else if (pid == 0)
    {
        char *arguments[5];
        for (int count = 0; count < 4; count++)
        {
            arguments[count] = malloc(sizeof(char) * 30);
        }
        strncpy(arguments[0], "./memory_manager", strlen("./memory_manager") + 1);
        snprintf(arguments[1], sizeof(memorySize), "%d", memorySize);
        strncpy(arguments[2], memoryplacement, strlen(memoryplacement) + 1);
        snprintf(arguments[3], sizeof(simulationTime), "%d", simulationTime);
        arguments[4] = NULL;

        //Excecuting memory manager
        execv(arguments[0], arguments);
        for (int count = 0; count < 4; count++)
        {
            free(arguments[count]);
        }
    }
    int newProcessId = 1;
    int currSec = 0;
    int counterForNextProcess = 0;
    int duration;
    VPptr currVP;
    linkedListPtr VPRunningList = malloc(sizeof(linkedList));
    list_init(VPRunningList);
    FifoPtr VPwaitingfifo = malloc(sizeof(Fifo));
    fifo_init(VPwaitingfifo);
    char *block_gen_mem_man;
    char message[BLOCK_SIZE];
    block_gen_mem_man = attach_memory_block(BLOCK_GEN_MEM, BLOCK_SIZE);
    if (block_gen_mem_man == NULL)
    {
        printf("ERROR: Can't get block\n");
        exit(EXIT_FAILURE);
    }
    while (currSec <= simulationTime)
    {
        int prID = 0;
        if (VPRunningList->numOfElements > 0)
            while (prID >= 0)
            {
                prID = -1;
                currVP = terminatedFunction(VPRunningList);
                //print_list(VPRunningList);
                if (currVP != NULL)
                {
                    prID = currVP->processId;
                    //Sending VPStop
                    strcpy(message, "VPStop");
                    sem_wait(sem_mem_man_cons);
                    strncpy(block_gen_mem_man, message, BLOCK_SIZE);
                    sem_post(sem_gen_prod);
                    //Sending process Id
                    sprintf(message, "%d", currVP->processId);
                    sem_wait(sem_mem_man_cons);
                    strncpy(block_gen_mem_man, message, BLOCK_SIZE);
                    sem_post(sem_gen_prod);
                    free(currVP);
                }
            }
        if (currSec != simulationTime)
        {
            if (counterForNextProcess <= 0)
            {
                duration = exponantialDistributionGenerator(avrg_time_to_live_T);
                currVP = malloc(sizeof(VP));
                VPInit(currVP, duration, sizegenerator(lo, hi), newProcessId, 0);
                fifo_push(VPwaitingfifo, currVP);
                newProcessId++;
                counterForNextProcess = exponantialDistributionGenerator(avrg_time_to_arrive_t);
            }
            while (!fifo_isEmpty(VPwaitingfifo))
            {
                currVP = VPwaitingfifo->End->VProcess;
                //Sending VPStart
                block_gen_mem_man = attach_memory_block(BLOCK_GEN_MEM, BLOCK_SIZE);
                if (block_gen_mem_man == NULL)
                {
                    printf("ERROR: Can't get block\n");
                    exit(EXIT_FAILURE);
                }
                //Sending VPStart
                strcpy(message, "VPStart");
                sem_wait(sem_mem_man_cons);
                strncpy(block_gen_mem_man, message, BLOCK_SIZE);
                sem_post(sem_gen_prod);
                //Sending process Id
                sprintf(message, "%d", currVP->processId);
                sem_wait(sem_mem_man_cons);
                strncpy(block_gen_mem_man, message, BLOCK_SIZE);
                sem_post(sem_gen_prod);
                //Sending Process size
                sprintf(message, "%d", currVP->size);
                sem_wait(sem_mem_man_cons);
                strncpy(block_gen_mem_man, message, BLOCK_SIZE);
                sem_post(sem_gen_prod);

                //Receive confirmation
                sem_wait(sem_mem_man_prod);
                strncpy(message, block_gen_mem_man, BLOCK_SIZE);
                sem_post(sem_gen_cons);

                if (!strcmp(message, "inserted"))
                {
                    //Sending the time is seconds that the process has waited before entering the memory
                    sprintf(message, "%d", currVP->timeWaiting);
                    sem_wait(sem_mem_man_cons);
                    strncpy(block_gen_mem_man, message, BLOCK_SIZE);
                    sem_post(sem_gen_prod);
                    currVP = fifo_pop(VPwaitingfifo);
                    list_insertItem(VPRunningList, currVP);
                }
                else if (!strcmp(message, "notinserted"))
                    break;
            }
            // sleep(1);
            runTimeFunction(VPRunningList);
            fifo_increase_waiting(VPwaitingfifo);
            counterForNextProcess--;
            currSec++;
            block_gen_mem_man = attach_memory_block(BLOCK_GEN_MEM, BLOCK_SIZE);
            if (block_gen_mem_man == NULL)
            {
                printf("ERROR: Can't get block\n");
                exit(EXIT_FAILURE);
            }
            strcpy(message, "nextSec");
            sem_wait(sem_mem_man_cons);
            strncpy(block_gen_mem_man, message, BLOCK_SIZE);
            sem_post(sem_gen_prod);
        }
        else
            currSec++;
    }
    //Send termination to memory manager
    strcpy(message, "Term");
    sem_wait(sem_mem_man_cons);
    strncpy(block_gen_mem_man, message, BLOCK_SIZE);
    sem_post(sem_gen_prod);

    sprintf(message, "%d", fifo_getNumOfElements(VPwaitingfifo));
    sem_wait(sem_mem_man_cons);
    strncpy(block_gen_mem_man, message, BLOCK_SIZE);
    sem_post(sem_gen_prod);
    int remainingVPs = fifo_getNumOfElements(VPwaitingfifo);

    for (int count = 0; count < remainingVPs; count++)
    {
        currVP = fifo_pop(VPwaitingfifo);
        //Sending process Id
        sprintf(message, "%d", currVP->processId);
        sem_wait(sem_mem_man_cons);
        strncpy(block_gen_mem_man, message, BLOCK_SIZE);
        sem_post(sem_gen_prod);
        //Sending Process size
        sprintf(message, "%d", currVP->size);
        sem_wait(sem_mem_man_cons);
        strncpy(block_gen_mem_man, message, BLOCK_SIZE);
        sem_post(sem_gen_prod);
        //Sending the time is seconds that the process has waited before entering the memory
        sprintf(message, "%d", currVP->timeWaiting);
        sem_wait(sem_mem_man_cons);
        strncpy(block_gen_mem_man, message, BLOCK_SIZE);
        sem_post(sem_gen_prod);
        VPdestruct(currVP);
    }
    detach_memory_block(block_gen_mem_man);

    fifo_deleteFifo(VPwaitingfifo);
    list_deleteList(VPRunningList);
    free(VPRunningList);
    sem_close(sem_mem_man_cons);
    sem_close(sem_gen_prod);
    sem_close(sem_gen_cons);
    sem_close(sem_mem_man_prod);
    sem_unlink(SEM_PRODUCER_GEN);
    sem_unlink(SEM_CONSUMER_MEM_MAN);
    sem_unlink(SEM_PRODUCER_MEM_MAN);
    sem_unlink(SEM_CONSUMER_GEN);
    destroy_memory_block(BLOCK_GEN_MEM);
    return 0;
}
