#include "mymalloc.h"

// global variable to store the elapsed time:
double elapsedTimes[6] = {0};

// Each of below function should allocate and then deallocate complete memory

void test1() {
	
    //A: malloc() 1 byte and immediately free it - do this 150 times
    int i=0;
	for(i; i<150; i++) {
		void *x = malloc(1);
		free(x);
	}
    
}

void test2() {
    
	// B: malloc() 1 byte, store the pointer in an array - do this 150 times.
	void *ptr[100];
	int count = 0;
	while(count < 100) {
		ptr[count] = malloc(1);
		count++;
		
		if(count == 50) {
            int i=0;
			for(i; i<50; i++) {
				free(ptr[i]);
			}
		}
	}
	// remove the remaining 50 blocks.
    int i=50;
    for(i; i<100; i++) {
		free(ptr[i]);
	}
    
}

void test3() {
    
	// C: Randomly choose between a 1 byte malloc() or free()ing a 1 byte pointer
	void *ptr[100];
	int alloced = 0;
	while(alloced != 50) {
		int x = rand() % 2;
		if(x == 0) {
			// create new
			ptr[alloced++] = malloc(1);
		} else {
			// free.
			if(alloced != 0) {
				free(ptr[--alloced]);
			}
		}
	}
	while(alloced != 0) {
		free(ptr[--alloced]);
	}
    
}

void test4() {
    
	// D. Randomly choose between a randomly-sized malloc() or free()ing a pointer 
	void *ptr[100];
	int alloced = 0;
	int freed = 0;
	
	while(alloced != 50) {
		int x = rand() % 2;
		if(x == 0) {
			// create new
			ptr[alloced++] = malloc(1 + rand() % 64);
		} else {
			// free.
			if(freed < alloced) {
				free(ptr[freed++]);
			}
		}
	}
	
	while(freed != alloced) {
		free(ptr[freed++]);
	}
    
}

void test5() {
    
	// E. => random malloc try 5000 times.
	int count = 0;
	int totalByteMalloced = 0;
	
	void *ptr[100];
	int size[100];
	
	int alloced = 0;
	
	// we will only allocate blocks in multipl of 100 from 100 bytes till 1000 bytes.
	// SO maximum overhead can be 41*2 = 82 bytes.
	while(count < 5000) {
		
		int x = rand() % 2;
		if(x == 0) {
			// create new
			int s = 100 * (1 + rand() % 10);
			
			if((totalByteMalloced + s) < (MAX_SIZE - 82)) {
				ptr[alloced] = malloc(s);
				
				if(ptr[alloced] == NULL) {
					printf("Test failed for random memory allocation\n");
				} else {
					size[alloced++] = s;
					totalByteMalloced += s;
				}
			}
			
		} else {
			// free.
			if(alloced != 0) {
				free(ptr[--alloced]);
				totalByteMalloced -= size[alloced];
			}
		}
		
		count++;
	}
	
	while(alloced != 0) {
		free(ptr[--alloced]);
	}
    
}

void test6() {
    
	// F. Test efficient memory allocation after freeing memory block
	void *ptr[6];
	
	// assuming 2 bytes overhead on each block.
	// 4 blocks of size: (4096 - 2*4)/4 = 1022
    ptr[0] = malloc(1021);
    ptr[1] = malloc(1021);
    ptr[2] = malloc(1021);
    ptr[3] = malloc(1021);
	
	// The entire memory has been allocated now.
	// Lets remove the middle block
	free(ptr[1]);
	
	// now we should be able to allocate 2 blocks from block size of 1024
	// The 2 block size can be: (1024 - 2*2)/2 = 510
    ptr[4] = malloc(509);
    ptr[5] = malloc(509);
	
	if(ptr[4] == NULL || ptr[5] == NULL) {
		printf("Test Failed: The freed block memory should have been allocated again efficiently.\n");
	}
	free(ptr[0]);
	free(ptr[2]);
	free(ptr[3]);
	free(ptr[4]);
	free(ptr[5]);
    
}

/*
void test7() {
    
    for(int i=0; i<15000; i++) {
        void *x = malloc(1);
        free(x);
    }
}
 
 */

void runWorkload(int loadIndex) {
	
	struct timeval t1, t2;
	
	// start timer
	gettimeofday(&t1, NULL);
	
	if(loadIndex == 1) test1();
	if(loadIndex == 2) test2();
	if(loadIndex == 3) test3();
	if(loadIndex == 4) test4();
	if(loadIndex == 5) test5();
	if(loadIndex == 6) test6();
	//if(loadIndex == 7) test7();
	
    // end timer
	gettimeofday(&t2, NULL);
	
	if(numOfAllocatedBlocks() != 0) {
		printf("Workload %d Failed. All the memory should be freed once a workload is completed\n", loadIndex);
		exit(1);
	}
	
	
	// sum up total time for workload in microseconds
	elapsedTimes[loadIndex-1] += (t2.tv_sec*1e6 + t2.tv_usec - t1.tv_sec*1e6 - t1.tv_usec);
}


int main() {
	
	// run 7 tests one by one.
	int count = 1000;
	
	int simulations = 0;
	while(simulations < count) {
        int t=1;
		for(t; t<=6; t++) {
			runWorkload(t);
		}
		
		simulations++;
	}
	
	// Now average the times.
	printf("Elapsed time as per workload:\n");
    int t=1;
	for(t; t<=6; t++) {
		elapsedTimes[t-1] /= count;
		printf("The Elapsed time for Testcase %d: %lf Microseconds.\n", t, elapsedTimes[t-1]);
	}
}
