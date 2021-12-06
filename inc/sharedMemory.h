#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>

// Function for the managment of the shared memory.
char *attach_memory_block(char *filename, int size);
bool detach_memory_block(char *block);
bool destroy_memory_block(char *filename);

// The size of the shared memory block.
#define BLOCK_SIZE 4096

//Shared memory block names.
#define BLOCK_GEN_MEM "generator.c"
#define BLOCK_MEM_GEN "memory_manager.c"

//-------------Semaphore Names-------------//

// P1 Enc1 relation.
#define SEM_PRODUCER_GEN "semGenProducerToMemMan"
#define SEM_CONSUMER_MEM_MAN "semMemManConsumerFromGen"
#define SEM_PRODUCER_MEM_MAN "semMemManProducerToGen"
#define SEM_CONSUMER_GEN "semGenConsumerFromMemMan"

#endif