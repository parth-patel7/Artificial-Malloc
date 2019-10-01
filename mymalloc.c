#include "mymalloc.h"
#include <string.h>

/*
We will check in each step if the array has already been initialized or not. The static array can initially be initialized by all bytes as 0s or any other garbage value in the metadata column by the system. When we get first call, we store the size of remaining memory by storing the short value. So, if total size is 4096, in first 2 bytes we store our magic number to check for the initialization of the static array during first call and in next 2 bytes we store the actual metadata needed to keep track of the free or used block along with its length. So, the remaining space left with us to do the allocation is 4092 Bytes (4096 - 4). Also, throughout the program, any positive size at start of block in the metadata column will be treated as if the block of memory is getting used. When the user frees some block, we will convert the block size to negative value, to show that the block is now not being used but has just specified size. Also, between calls, we will try to maintain successive free blocks as a single chunk by merging them. This way by using only 2 bytes for metadata column and 2 bytes to store a magic number we can make the most efficient program for allocating and freeing the memory in the same way as malloc() and free().
 */
 
short myAbs(short x) {
	if(x < 0) {
		return -x;
	} else {
		return x;
	}
}

/*
 * This function returns the allocated bytes for user in the given 
 * block of memory. The size is stored on starting 2 bytes.
 */
short getBlockSize(void *blockStart) {
	short *blockSize = (short *)blockStart;
	return *blockSize;
}


/*
 * This function puts the size of the block on the starting 2
 * bytes of the block. We need to take 2 bytes, because size can vary 
 * till 4096, which fits in range of 2 bytes.
 */
void putBlockSize(void *blockStart, short size) {
	short *blockSize = (short *)blockStart;
	*blockSize = size;
}


/*
 * This function returns the address of first memory block
 * after skipping the magic number.
 * Precondition: Memory has been initialized.
 */
void *getFirstBlockAddress() {
	// We can not perform pointer arithmetic on void pointers
	char *start = STORAGE_ARR;
	char *end = (char *) STORAGE_ARR + sizeof(unsigned short);
	
	return (void *)end;
}


/*
 * This function checks if the block of memory has been initialized before the
 * first call. After initialization, the first 2 bytes of memory will contain
 * some positive value, denoting the memory size.
 */ 
void checkInitialization() {	
	
	unsigned short *memStart = (unsigned short *) STORAGE_ARR;
	
	if(*memStart != MAGIC_NUMBER) {
        
        
		// Memory has not been initialized.
		
		// clean memory
		memset((void *)STORAGE_ARR, 0, MAX_SIZE);
		
		// Put magic number first.
		*memStart = MAGIC_NUMBER;
    
        // forward for skipping MAGIC_NUMBER
        char *start = (char *) STORAGE_ARR + sizeof(unsigned short);
		char *end = (char *) STORAGE_ARR + MAX_SIZE;
		
		// Negative size below shows the block is available to be allocated
		short size = -1 * (end - start - sizeof(short));
        //printf("size = %d \n", size);
		putBlockSize((void *)start, size);
	
    }
}


/*
 * this function checks the memory allocated to current block, if the size
 * is positive, it means the block is currently in use. Else, The function
 * treets it as available.
 */
int isBlockAvailable(void *blockStart) {
	// Any block, which is being used by users, will be
	// having positive blocksize in start 2 bytes.
	short blockSize = getBlockSize(blockStart);
	return blockSize < 0;
}


/*
 * Give current memory block address, this function gives the 
 * address of next memory block after skipping allocated memory
 * units. If the next block is outside the static array range, then 
 * it returns NULL.
 */
void *nextBlockAddr(void *blockStart) {
	char *END = (char *) STORAGE_ARR + MAX_SIZE;
	
	short blockSize = getBlockSize(blockStart);
	char *start = (char *)blockStart;
	
	// next block is Size + 2 bytes away from start.
	start += myAbs(blockSize) + sizeof(short);
	
	// check if block does not go out of static array bounds.
	if(start < END) {
		return (void *)start;
	} else {
		return NULL;
	}
}


/*
 * this functiuon tries to find the first free block, which is having 
 * a minimum free memory, so that it can be returned to the user.
 * If function is not able to find such a block, it returns NULL.
 */
void *getFirstFreeBlock(unsigned int minSize) {
	void *current = getFirstBlockAddress();
	
	while(current != NULL) {
		// Check if block is available for allocation, and has required memory size
		if(isBlockAvailable(current) && myAbs(getBlockSize(current)) >= minSize) {
			return current;
		}
		
		current = nextBlockAddr(current);
	}
	
	return NULL;
}


/*
 * Function which is analogous to malloc function, it gives a reference to the 
 * static char array to user, which user can use for its own purpose.
 */
void *mymalloc(unsigned int size, char *file, unsigned int line) {
	
	checkInitialization();
	
    //printf("%hu \n", *(short*) &STORAGE_ARR);
    
	void *freeBlock = getFirstFreeBlock(size);
	
	// If no sufficient memory block is available
	if(freeBlock == NULL) {
		printf("Error: Not enough memory available to allocate, File %s and line %u\n", file, line);
		return NULL;
	}
	
	// Allocate the required memory from the freeBlock.
	void *result = freeBlock;
	short freeBlockSize = myAbs(getBlockSize(freeBlock));
	
	// get the blockSize which is remaining
	short remainingBlockSize = freeBlockSize - (short)size - sizeof(short);
	
	// if the next blocks memory remains as 0, then we will send the entire block to user.
	// It will contain some extra memory for user, but user should not use it.
	if(remainingBlockSize <= 0) {
		
		// put positive block Size, so that we mark it as used.
		putBlockSize(freeBlock, freeBlockSize);
		return freeBlock;
	}
	
	putBlockSize(freeBlock, myAbs((short)size));
	
	// put the negative remaining size in next block, to show that
	// The next block is available
	void *nextBlockStart = nextBlockAddr(freeBlock);
	putBlockSize(nextBlockStart, -remainingBlockSize);
	
	return freeBlock;
}


// check if pointer is in range of our static array
int isPointerInRange(void *ptr) {
	// We can not perform pointer arithmetic on void pointers
	char *start = (char *) getFirstBlockAddress();
	char *end = (char *) STORAGE_ARR + MAX_SIZE;
	char *x = (char *) ptr;
	
	return (start <= x) && (x < end);
}


// check if pointer has been allocated by our code.
int isPointerAllocatedByCode(void *ptr) {
	void *current = getFirstBlockAddress();
	
	while(current != NULL) {
		if(current == ptr) {
			return 1;
		}
		
		current = nextBlockAddr(current);;
	}	
	
	return 0;
}


/*
 * This function assumes that the given pointer is of correct allocated
 * block, and it need to free that. It checks to see if there are any 
 * free blocks which can be combined, to make bigger free memory chunk.
 * If yes, it merges those chunks.
 */
void freeMyBlock(void *blockStart) {
	short size = myAbs(getBlockSize(blockStart));
	putBlockSize(blockStart, -size); // put negative size to show that the block is available now.
	
	// Check if the newly created block can be merged with other free blocks.
	// To create a bigger chunk of memory
	void *current = getFirstBlockAddress();
	
	while(current != NULL) {
		void *nextBlock = nextBlockAddr(current);
		
		if(nextBlock == NULL) {
			break;
		} else if(isBlockAvailable(current) && isBlockAvailable(nextBlock)) {
			// we can merge the current and nextBlock
			short currentBlkSize = myAbs(getBlockSize(current));
			short nextBlkSize = myAbs(getBlockSize(nextBlock));
			
			// update the size of current block with new size, and mark it unused,
			short totalSize = currentBlkSize + nextBlkSize + sizeof(short);
			putBlockSize(current, -totalSize);
		} else {
			current = nextBlock;
		}
	}	
}


/*
 * Function which frees the memory allocated by mymalloc function
 */ 
void myfree(void *ptr, char *file, unsigned int line) {
	
	// check if pointer is in range of our static array
	if(!isPointerInRange(ptr)) {
		printf("Error: Invalid Pointer, File %s and line %u\n", file, line);
		return;
	}
	
	// check if pointer has been allocated by our code.
	if(!isPointerAllocatedByCode(ptr)) {
		printf("Error: Pointer has not been allocated by mymalloc, File %s and line %u\n", file, line);
		return;
	}
	
	// Check if the allocated block has already been freed
	if(isBlockAvailable(ptr)) {
		printf("Error: Pointer has already been freed, File %s and line %u\n", file, line);
		return;
	}
	
	// we found the block to be freed.
	freeMyBlock(ptr);
}

/*
 * Utility function to print the current memory map
 */
void printReport() {
	checkInitialization();
	
	unsigned short *memStart = (unsigned short *) STORAGE_ARR;
	printf("Magic Num: %d\n", *memStart);
	
	void *current = getFirstBlockAddress();
	
	int totalAllocatedBlocks = 0;
	while(current != NULL) {
		void *nextBlock = nextBlockAddr(current);
		
		printf("Block Start: %x, End: %x, BlockSize: %d, inUse: %d\n",
			current, nextBlock, myAbs(getBlockSize(current)), !isBlockAvailable(current));
		
		if(!isBlockAvailable(current)) {
			totalAllocatedBlocks++;
		}
		
		current = nextBlock;
	}
	printf("Total allocated Memory blocks: %d\n", totalAllocatedBlocks);
	printf("\n\n");
}


/*
 * Utility function to find how many blocks are currently allocated.
 */
int numOfAllocatedBlocks() {
	checkInitialization();
	
	void *current = getFirstBlockAddress();
	
	int totalAllocatedBlocks = 0;
	while(current != NULL) {
		
		if(!isBlockAvailable(current)) {
			totalAllocatedBlocks++;
		}
		
		current = nextBlockAddr(current);
	}
	return totalAllocatedBlocks;
}






