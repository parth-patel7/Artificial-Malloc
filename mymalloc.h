#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)

#define MAX_SIZE 4096

#define MAGIC_NUMBER 57921

static char STORAGE_ARR[MAX_SIZE];

void * mymalloc(unsigned int size, char *file, unsigned int line);

void myfree(void *ptr, char *file, unsigned int line);

void printReport();

int numOfAllocatedBlocks();
