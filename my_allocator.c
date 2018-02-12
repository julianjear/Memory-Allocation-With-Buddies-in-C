/* 
    File: my_allocator.c

    Author: Julian Alvarez
            Department of Computer Science
            Texas A&M University
    Date  : 2/9/18
    Modified: 

    This file contains the implementation of the module "MY_ALLOCATOR".

*/

//call on putty: : gcc -std=c99 -lm -g -o memtest *.c && ./memtest -b 64 -s 512


/*--------------------------------------------------------------------------*/
/* WHAT I'VE LEARNED */
/*--------------------------------------------------------------------------*/
//1. adding to an address will multiply the integer value of what you're adding * the size of the type you're adding to
//EX: int * p = (int *) malloc(4)
// p = p + 2
// If p is at address 100, p+2 will translate to 100+(2*4) where 4 is the size of the int
//EX: int * p = (char *)p + 1 	| will translate to 100+(2*1) where 1 is the size of char

//2. A double pointer is an array of pointers

//3. sizeof(freeListHeader) = 24 - the size of all the different data fields in the header
//   sizeof(freeListHeader*) = 8 because it is getting the size of the pointer 
   
//4. (flHeader *) - points to flheader (flHeader **) - points to flheader pointer

//5. A node can be added to the beginning of a linked list refrenced by an array of pointers with the following code:
//	node->next = freeList[index]; //the new node being inserted has it's next point to the first node of the linked list
//	freeList[index] = node; //the first element of the linked list is changed to the new node. The new node's next points to the previous first node
	
//6. When dynamically allocating a new piece of memory, the argument inside of malloc specifies the number of indicies of the array * the size of each entry
//	freeList = (flHeader **) malloc(numBlocks*sizeof(flHeader *));


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include<stdlib.h>
#include "my_allocator.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
	

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

	void * memStart = NULL;
	flHeader ** freeList;
	unsigned int bbs;
	unsigned int memSize;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

	
//helper function to get index based on blockSize
int getIndex(int bSize) {
	return log2(memSize) - log2(bSize);
}

void printFL() {
	int numBlocks = log2(memSize) - log2(bbs) + 1; //size of the number of blocks in the free list
	int sizeM = memSize;
	
	for(int i = 0; i < numBlocks; i++) {
		printf("\n %i blocks: ", sizeM);
		int blocks = 0;
		flHeader * header = freeList[i];
		
		while(header != NULL) {
			blocks++;
			header = header->next;
			
		}
		printf("%i", blocks);
		sizeM /= 2;
	}
}
	
void addBlock(flHeader * hAdd, int index) {
	
	hAdd->next = freeList[index];
	freeList[index] = hAdd;
}

//delete block at the end of linked list
void deleteBlock(flHeader * hDelete, int index) {
	
	flHeader *head = freeList[index];
	
	if(head == hDelete) {
		freeList[index] = hDelete->next;
	} else {
		
		while(head->next != hDelete) {
			head = head->next;
		}
		
		head->next = hDelete->next;
		hDelete->next = NULL;
	}
		
}

void splitBlock(flHeader * hSplit, int index) {
	printf("\n\n Split called: %i", hSplit->blockSize);
	
	int bSize = hSplit->blockSize;
	Addr splitAddress = hSplit; //get the address of the block that is being split
		
	deleteBlock(hSplit, index);
	index++; //go one level lower in the free list to get the next smallest block
	
	flHeader * hNew1 = (flHeader *) ((char *)hSplit);
	//(flHeader *) - points to flheader (flHeader **) - points to flheader pointer
	hNew1->blockSize = bSize/2;
	hNew1->inUse = false;
	
	// (char *) is used to get the correct size in the pointer arithmetic
	// (flHeader *) is used to turn the char * back to flHeader *
	flHeader * hSplit2 = (flHeader *) ((char *)hNew1 + hNew1->blockSize);
	hSplit2->blockSize = bSize/2;
	hSplit2->inUse = false;

	addBlock(hNew1, index);
	addBlock(hSplit2, index);
	printf("\n Split succeeded");
	
		
}

int nextPowTwo(int num) {
	double numPower = log2(num);
	return pow(2, ceil(numPower));
}

unsigned int init_allocator(unsigned int basic_block_size, unsigned int length)  {
	/* This function initializes the memory allocator and makes a portion of 
   ’_length’ bytes available. The allocator uses a ’_basic_block_size’ as 
   its minimal unit of allocation. The function returns the amount of 
   memory made available to the allocator. If an error occurred, 
   it returns 0. 
*/
	printf("\n bbs: %i memSize: %i", basic_block_size, length);
	bbs = basic_block_size;
	memSize = length;
	if(bbs > memSize) {
		printf("\n Error: basic block size can not be greater than the size of memory");
		abort();
	}

	memStart = malloc(memSize);
	
	int numBlocks = log2(memSize) - log2(bbs) + 1; //size of the number of blocks in the free list
	
	freeList = (flHeader **) malloc(numBlocks*sizeof(flHeader *));
	
	freeList[0] = (flHeader *) memStart;
	freeList[0]->blockSize = memSize;
	freeList[0]->next = NULL;
	freeList[0]->inUse = false;
	
	for(int i = 1; i < numBlocks; i++) {
		freeList[i] = NULL;
	}
	
}


int release_allocator() {
	/* This function returns any allocated memory to the operating system. 
	   After this function is called, any allocation fails.
	*/ 
	printf("\n Release Allocator Called\n");
	if(freeList != NULL) {
		free(freeList);
		freeList = NULL;
		free(memStart);
		memStart = 0;
		return 0;
	}
	return 1;

}
//my_malloc will only be called by user
extern Addr my_malloc(unsigned int request) {
	/* This preliminary implementation simply hands the call over the 
	 the C standard library! 
	 Of course this needs to be replaced by your implementation.
	*/
	printf("\n\n myMalloc Called");
	printf("\n request: %i", request);
	printFL();

	//find free block that satisfies request size

	int bSizeNeeded = nextPowTwo(request + sizeof(flHeader));

	if(bSizeNeeded < bbs)
	  bSizeNeeded = bbs;
	if(bSizeNeeded > memSize) {
		printf("Error: block size needed is greater than the size of memory");
		release_allocator();
		abort();
	}
	printf("\n blockSizeNeeded: %i", bSizeNeeded);


	int index = getIndex(bSizeNeeded);
	for(index; index >= 0; index--) {
		
		if(freeList[index] != NULL) {

		while(freeList[index]->blockSize > bSizeNeeded) {
				splitBlock(freeList[index], index);
				index++;
			}
			break;
			
		}
	}
	if(index == -1) {
		printf("\n Error: not enough memory available");
		release_allocator();
		abort();
	}
	
	flHeader * hDelete = (flHeader *) ((char *)freeList[index]);
	Addr * returnAddr = (Addr) ((char *)hDelete + sizeof(flHeader));
	freeList[index]->inUse = true; // set the block inuse to true but remove and remove from freelist
	deleteBlock(hDelete, index);
	
	printf("\n myMalloc Finished");

	return returnAddr; 
}

//my_free will only be called by user
extern int my_free(Addr a) {
	printf("\n\n myFree Called: %p", a);
	//add block back to free list
	flHeader *h = (flHeader *) ((char *)a - sizeof(flHeader));
	h->inUse = false;
	addBlock(h, getIndex(h->blockSize));
	printf("\n freeB size: %i", h->blockSize);
	
	//merge if possible
	while(h->blockSize < memSize) { //keep merging as long as there are available buddies
		flHeader *buddy = getBuddy((Addr) h, h->blockSize);
		
		printf("\n buddy 1 Address: %p", h);
		printf("\n buddy 2 Address: %p", buddy);

		if(!buddy->inUse && buddy->blockSize == h->blockSize && h != buddy) {
			h = mergeBlocks(h, buddy);
		} else {
			break;
		}
		
	}
	
	printf("\n myFree finished");
	printFL();

  return 0;
}

Addr getBuddy(Addr start, int bSize) {
	Addr buddyAddr = (Addr) (((start - memStart) ^ bSize ) + memStart);
	
	return buddyAddr;
}

flHeader * mergeBlocks(flHeader * h1, flHeader * h2) {
	printf("\n Merge Started");
	
	//delete two blocks from free list
	deleteBlock(h1, getIndex(h1->blockSize));
	deleteBlock(h2, getIndex(h2->blockSize));

	//merge blocks into a larger block
	flHeader *newBlock;
	if(h1 < h2) newBlock = h1;
	else newBlock = h2;
	newBlock->blockSize *= 2;
	newBlock->inUse = false;
	addBlock(newBlock, getIndex(newBlock->blockSize));
	
	printf("\n Merge Finished");
	return newBlock;
}
