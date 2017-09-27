/*************************************
* Lab 3 Exercise 1
* Name: Xu Bili
* Student No: A0124368A
* Lab Group: 4
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  // For Predefined constants
#include <sys/ipc.h>    // For POSIX IPC
#include <sys/shm.h>    // For POSIX Shared Memory

// Constants
#define READY_PARENT 0
#define READY_CHILD 1
#define READY_PARENT_VALUE 1111
#define READY_CHILD_VALUE 1111
#define PARTIAL_SUM_PARENT 2
#define PARTIAL_SUM_CHILD 3

int main()
{
    int result, arraySize, initValue;
    char* shdMemRegion;
    int shdMemId, shdMemSize;  //Remember to initialize this before shmget()

    //Get input
    printf("Enter Array Size: ");
    scanf("%i",&arraySize);

    printf("Enter Start Value: ");
    scanf("%i",&initValue);


    // TODO: Calculate the correct shdMemSize
    // We have two extra integers to indicate ready status of parent/child
    // We have another two extra integers to store the partial sums
    shdMemSize = (arraySize + 4) * 4;

    //Create a new shared memory region
    shdMemId = shmget( IPC_PRIVATE, shdMemSize, IPC_CREAT | 0666 );
    if (shdMemId < 0){
        printf("Cannot create shared memory region!\n");
        exit(1);
    }
    printf("Shared Memory Id is %i\n",shdMemId);

    //Attach a new shared memory region
    shdMemRegion = (char*) shmat(shdMemId, NULL, 0);
    if ( shdMemRegion == (char*)-1){
        printf("Cannot attach shared memory region!\n");
        exit(1);
    }

    // TODO: Initialize the shared memory region
    int *arr = (int*) shdMemRegion;

    arr[READY_PARENT] = -1;
    arr[READY_CHILD] = -1;
    arr[PARTIAL_SUM_PARENT] = -1;
    arr[PARTIAL_SUM_CHILD] = -1;

    for (int i = 4; i < arraySize + 4; i++) {
        arr[i] = initValue + i - 4;
    }

    //Shared memory regions remained attached after fork()
    // Parent and child can now communicate with each other!
    result = fork();
    if (result){        //Parent
        // Calculate our parent partial sum here
        int parentSum = 0;
        for (int i = (arraySize / 2) + 4; i < arraySize + 4; i++) {
            parentSum += arr[i];
        }
        arr[PARTIAL_SUM_PARENT] = parentSum;
        arr[READY_PARENT] = READY_PARENT_VALUE;

        // Wait for our child to be done calculating
        while (arr[READY_CHILD] != READY_CHILD_VALUE) {
            sleep(1);
        }

        int childSum = arr[PARTIAL_SUM_CHILD];

        //Calculation ends. Show results.
        printf("Parent Sum = %d\n", parentSum);
        printf("Child Sum = %d\n", childSum);
        printf("Total = %d\n", parentSum + childSum);

        /*Important: Remember to detach the shared memory region*/
        shmdt( shdMemRegion );
    } else {            //Child
        while (arr[READY_PARENT] != READY_PARENT_VALUE) {
            sleep(1);
        }

        int childSum = 0;
        for (int i = 4; i < (arraySize / 2) + 4; i++) {
            childSum += arr[i];
        }

        arr[PARTIAL_SUM_CHILD] = childSum;
        arr[READY_CHILD] = READY_CHILD_VALUE;

        /*Important: Remember to detach the shared memory region*/
        shmdt( shdMemRegion );

        return 0;   //Child Ends here

    }

    /*Important: Remember to remove the shared memory region after use!*/
    shmctl(shdMemId, IPC_RMID, NULL);
    return 0;
}
