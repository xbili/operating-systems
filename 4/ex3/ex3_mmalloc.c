/*************************************
 * Lab 4 Exercise 3
 * Name: Xu Bili
 * Student No: A0124368A
 * Lab Group: 4
 *************************************/

#include <stdio.h>
#include <string.h>
#include "mmalloc.h"

//This is a "global" structure storing heap information
// Only visible to functions in this file
static heapMetaInfo hmi;

void initializeMetaPartAt(partMetaInfo* bmiPtr, int size)
{
    bmiPtr->size = size;
    bmiPtr->nextPart = NULL;
    bmiPtr->status = FREE;
}

void printMetaPartList(partMetaInfo* bmiPtr)
{
    partMetaInfo* current = bmiPtr;

    for ( current = bmiPtr; current != NULL;
            current = current->nextPart){

        printf("[+%5d | %5d bytes | %d]\n",
                (int)((void*)current - (void*)hmi.base),
                current->size, current->status);
    }
}

void printHeapStatistic()
{
    //You dont need to provide the solution for this function for ex2
    printf("Heap Usage Statistics\n");

    printf("Total Space: %d bytes\n", hmi.totalSize);

    printf("Total Occupied Partitions: %d\n", 0);
    printf("\tTotal Occupied Size: %d bytes\n", 0);

    printf("Total Number of Holes: %d\n", 0);
    printf("\tTotal Hole Size: %d bytes\n", 0);

    printf("Total Meta Information Size: %d bytes\n", 0);

}

int setupHeap(int initialSize)
{
    void* base;

    base = sbrk(0);
    if(	sbrk(initialSize) == (void*)-1){
        printf("Cannot set break! Behavior undefined!\n");
        return 0;
    }

    hmi.totalSize = initialSize;
    hmi.base = (partMetaInfo*) base;
    hmi.partMetaSize = sizeof(partMetaInfo);

    initializeMetaPartAt(hmi.base, initialSize - hmi.partMetaSize);

    return 1;
}

void printHeapMetaInfo()
{
    printf("Heap Meta Info:\n");
    printf("===============\n");
    printf("Total Size = %d bytes\n", hmi.totalSize);
    printf("Start Address = %p\n", hmi.base);
    printf("Partition Meta Size = %d bytes\n", hmi.partMetaSize);
    printf("Partition list:\n");

    printMetaPartList((partMetaInfo*) hmi.base);
    printf("\n");

}

void splitPart(partMetaInfo *bigPart, int size)
{
    partMetaInfo *holeAt;
    int holeSize;

    //Remember: When calculating address offset, use a pointer of
    //(void*) type (or char*), so that the calculation is in bytes.
    holeAt = (void*)bigPart + hmi.partMetaSize + size;
    holeSize = bigPart->size - hmi.partMetaSize - size;

    //Make a new partition for the hole
    initializeMetaPartAt(holeAt, holeSize);
    holeAt->nextPart = bigPart->nextPart;
    bigPart->nextPart = holeAt;

    bigPart->size = size;

}

void* malloc(int size)
{
    partMetaInfo *current = hmi.base;

    //We need to make sure the size is word
    // aligned, i.e. if the word size is 4 bytes, the size need to be
    // rounded to nearest multiples of 4. Otherwise, user can get "bus
    // error" when accessing non-aligned memory locations

    // divide by 4 then multiply by 4 gives rounded multiples of 4.
    // addition  of 4 round up to the next multiple
    // subtraction take care of the case where size is already multiples
    //  of 4
    size = (size - 4) / 4 * 4 + 4;

    while ( current!=NULL &&
            (current->status == OCCUPIED || current->size < size) ){

        current = current->nextPart;
    }

    if (current == NULL){	//heap full
        return NULL;
    }

    //Can we split the part?
    //The new "hole" should >= 4 bytes after placing in a new part
    // meta info structure
    if (current->size >= size + hmi.partMetaSize + 4 ){
        splitPart(current, size);
    }

    current->status = OCCUPIED;

    return (void*)current + hmi.partMetaSize;
}

void free(void* address)
{
    partMetaInfo *toBeFreed;

    toBeFreed = address - hmi.partMetaSize;
    toBeFreed->status = FREE;

    // Checks all partitions to merge free partitions
    partMetaInfo *curr = hmi.base;
    while (curr && curr->nextPart) {
        if (curr->status == FREE && curr->nextPart->status == FREE) {
            curr->size += curr->nextPart->size + hmi.partMetaSize;
            curr->nextPart = curr->nextPart->nextPart;
        } else {
            curr = curr->nextPart;
        }
    }
}


void compact()
{
    partMetaInfo *curr = hmi.base;
    partMetaInfo *prev = hmi.base;

    // Keep track of the size of all the free memory in the middle
    int totalFreeSize = 0;
    while (curr) {
        if (curr->status == OCCUPIED) {
            // Copy this partition into the next available partition

            // Trick to increment pointer by bytes
            char *dest = (char*) prev;
            if (prev->status != FREE) {
                dest += prev->size + hmi.partMetaSize;
            }

            memmove(
                dest,
                curr,
                curr->size + hmi.partMetaSize
            );

            prev->nextPart = (partMetaInfo*) dest;
            prev = prev->nextPart;
        } else {
            totalFreeSize += curr->size + hmi.partMetaSize;
        }

        curr = curr->nextPart;
    }

    // Move to the end of the last available block
    char *dest = (char*) prev;
    dest += prev->size + hmi.partMetaSize;
    partMetaInfo *freeSpace = (partMetaInfo*) dest;
    prev->nextPart = freeSpace;

    initializeMetaPartAt(prev->nextPart, totalFreeSize - hmi.partMetaSize);
    freeSpace->nextPart = NULL; // For good measure
}

//Do NOT Change
void verifyCompact()
{
    partMetaInfo *current;
    int *array, first, last;

    printf("Compact Verifier:\n");
    printf("=================\n");
    printf("Content Check:\n");
    for (current = hmi.base ;current != NULL && current->status != FREE;
            current = current->nextPart) {

        //Pointer arithmetic is based on the type of pointer
        // so, need to use (void*) to calculate the offsetproperly
        array = (int*)((void*)current + hmi.partMetaSize);

        //print first and last element
        first = array[0];
        last = array[ current->size / sizeof(int) - 1];
        printf("\t[%d == %d]", first, last);
        if (first != last){
            printf(" = FAILED!\n");
        } else {
            printf("\n");
        }
    }

    //Should not have any more partition after the last free partition
    printf("Partition Check: ");
    if (current != NULL && current->nextPart != NULL){
        printf("FAILED!\n");
    } else {
        printf("PASSED\n");
    }
    printf("\n");
}
