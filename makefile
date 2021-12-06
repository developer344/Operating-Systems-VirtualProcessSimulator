CC=gcc
EXEC=generator memory_manager

all: $(EXEC)

generator: generator.c virtualprocess.c linkedList.c fifo.c sharedMemory.c
	gcc -o generator generator.c virtualprocess.c linkedList.c sharedMemory.c fifo.c -lm -lpthread -g

memory_manager: memory_manager.c virtualprocess.c linkedList.c sharedMemory.c memory.c
	gcc -o memory_manager memory_manager.c virtualprocess.c linkedList.c sharedMemory.c memory.c -lm -lpthread -g

clean:
	rm -rf *.o $(EXEC)