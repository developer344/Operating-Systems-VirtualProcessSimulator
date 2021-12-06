#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "memory.h"

void memory_init(MemoryPtr memory, int memorySize, char *typeOfMemoryPlacement)
{
    memory->memorySize = memorySize;
    if (!strcmp(typeOfMemoryPlacement, "best-fit"))
    {
        memory->typeOfMemoryPlacement = 1;
    }
    else if (!strcmp(typeOfMemoryPlacement, "worst-fit"))
    {
        memory->typeOfMemoryPlacement = 2;
    }
    else if (!strcmp(typeOfMemoryPlacement, "buddy"))
    {
        memory->typeOfMemoryPlacement = 3;
    }
    memory->array = malloc(sizeof(int) * memorySize);
    for (int i = 0; i < memory->memorySize; i++)
    {
        if (memory->typeOfMemoryPlacement == 3)
            memory->array[i] = 0;
        else
            memory->array[i] = -1;
    }
}

//Return 0 for success, return -1 for failure
int memory_insert_process(MemoryPtr memory, int processId, int processSize)
{
    if (memory->typeOfMemoryPlacement != 3)
    {
        typedef struct pointer
        {
            int start;
            int end;
            int size;
        } pointer;
        pointer emptySpaces[memory->memorySize];
        int biggestSpaceSize = -1;
        int biggestSpaceSizePointer = -1;
        int maxEmptySpaces = 0;
        int count = 0;
        int flag = 0;
        while (count < memory->memorySize)
        {
            if (memory->array[count] >= 0)
                flag = 0;
            else
                flag = 1;

            if (flag)
            {
                int i;
                emptySpaces[maxEmptySpaces].start = count;

                for (i = count; i < memory->memorySize; i++)
                    if (memory->array[i] >= 0)
                        break;

                emptySpaces[maxEmptySpaces].end = i - 1;
                emptySpaces[maxEmptySpaces].size = emptySpaces[maxEmptySpaces].end - emptySpaces[maxEmptySpaces].start + 1;
                if (emptySpaces[maxEmptySpaces].size > biggestSpaceSize)
                {
                    biggestSpaceSize = emptySpaces[maxEmptySpaces].size;
                    biggestSpaceSizePointer = maxEmptySpaces;
                }
                maxEmptySpaces++;
                count = i;
            }
            else
                count++;
        }
        if (biggestSpaceSize >= processSize)
        {
            if (memory->typeOfMemoryPlacement == 1)
            {
                int bestFitSpace = memory->memorySize + 1, bestFitSpacePointer = -1;
                for (count = 0; count < maxEmptySpaces; count++)
                {
                    if (emptySpaces[count].size >= processSize && emptySpaces[count].size < bestFitSpace)
                    {
                        bestFitSpacePointer = count;
                        bestFitSpace = emptySpaces[count].size;
                    }
                }
                for (int i = emptySpaces[bestFitSpacePointer].start; i < emptySpaces[bestFitSpacePointer].start + processSize; i++)
                {
                    memory->array[i] = processId;
                }
            }
            else if (memory->typeOfMemoryPlacement == 2)
            {
                for (int i = emptySpaces[biggestSpaceSizePointer].start; i < emptySpaces[biggestSpaceSizePointer].start + processSize; i++)
                {
                    memory->array[i] = processId;
                }
            }
            return 0;
        }
        else
            return -1;
    }
    else
    {
        return memory_insert_processBuddy(memory, 0, memory->memorySize, processId, processSize);
    }
}

int memory_insert_processBuddy(MemoryPtr memory, int startPointer, int segmmentSize, int processId, int processSize)
{
    if (processSize > segmmentSize)
        return -1;

    if (memory_insert_processBuddy(memory, startPointer, segmmentSize / 2, processId, processSize) == 0)
        return 0;
    if (memory_insert_processBuddy(memory, startPointer + (segmmentSize / 2), segmmentSize / 2, processId, processSize) == 0)
        return 0;

    for (int i = startPointer; i < startPointer + segmmentSize; i++)
    {
        if (memory->array[i] != 0)
            return -1;
    }
    for (int i = startPointer; i < startPointer + processSize; i++)
        memory->array[i] = processId;

    for (int i = startPointer + processId; i < startPointer + segmmentSize; i++)
        memory->array[i] = -processId;

    return 0;
}

int memory_countEmptySpaces(MemoryPtr memory)
{
    int returnValue = 0;
    for (int i = 0; i < memory->memorySize; i++)
    {
        returnValue += memory->array[i] < 0 ? 1 : 0;
    }
    return returnValue;
}

void memory_delete_process(MemoryPtr memory, int processId)
{
    if (memory->typeOfMemoryPlacement != 3)
    {
        for (int i = 0; i < memory->memorySize; i++)
            if (memory->array[i] == processId)
                memory->array[i] = -1;
    }
    else
    {
        for (int i = 0; i < memory->memorySize; i++)
            if (memory->array[i] == processId || memory->array[i] == -processId)
                memory->array[i] = 0;
    }
}

void destroy_memory(MemoryPtr memory)
{
    free(memory->array);
    free(memory);
}