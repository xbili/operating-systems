/*************************************
* Lab 3 Exercise 1
* Name:
* Student No:
* Lab Group:
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  // For Predefined constants
#include <sys/ipc.h>    // For POSIX IPC
#include <sys/shm.h>    // For POSIX Shared Memory

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


    //TODO: Calculate the correct shdMemSize
    shdMemSize = 0; 

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

    //TODO: Initialized the shared memory region 


    //Shared memory regions remained attached after fork()
    // Parent and child can now communicate with each other!
    result = fork();
    if (result){        //Parent

        //TODO: Fill In Your Code Here




        //Calculation ends. Show results.
        //TODO: Indicate the correct variables for output
        printf("Parent Sum = %d\n", -1);
        printf("Child Sum = %d\n", -1);
        printf("Total = %d\n", -1);

        /*Important: Remember to detach the shared memory region*/
        shmdt( shdMemRegion );
    } else {            //Child

        //TODO:Fill in Your Code Here


        /*Important: Remember to detach the shared memory region*/
        shmdt( shdMemRegion );

        return 0;   //Child Ends here

    }

    /*Important: Remember to remove the shared memory region after use!*/
    shmctl(shdMemId, IPC_RMID, NULL); 
    return 0;
}
