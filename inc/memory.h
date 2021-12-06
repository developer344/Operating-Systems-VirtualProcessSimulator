#ifndef _MEMORY_H_
#define _MEMORY_H_

typedef struct memory Memory;
typedef struct memory *MemoryPtr;

typedef struct memory
{
    int memorySize;
    int typeOfMemoryPlacement; //1: best-fit, 2: worst-fit, 3: buddy
    int *array;
} Memory;

void memory_init(MemoryPtr memory, int memorySize, char *typeOfMemoryPlacement);
int memory_insert_process(MemoryPtr memory, int processId, int processSize);
int memory_insert_processBuddy(MemoryPtr memory, int startPointer, int segmmentSize, int processId, int processSize);
int memory_countEmptySpaces(MemoryPtr memory);
void memory_delete_process(MemoryPtr memory, int processId);
void destroy_memory(MemoryPtr memory);
#endif