/*************************************
* Lab 3 Exercise 1
* Name: Xu Bili
* Student No: A0124368A
* Lab Group: 4
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main()
{
    int result;
    int shdMemId;    
    char* shdMemRegion;
    int shdMemSize = 4096;  //Size in # of bytes

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

    //Shared memory regions remained attached after fork()
    // Parent and child can now communicate with each other!
    result = fork();
    if (result){        //Parent
        printf("%i is the parent\n", getpid());

        int* Array = (int*) shdMemRegion;

        Array[1] = 1234;
        Array[2] = 5678;
        Array[0] = 9999;    //Indicate writing is done for the child

        printf("Parent: I have finished writing!\n");

        //Wait for child to finish writing
        while (Array[3] != 1111){
            sleep(1);
        } 

        printf("Parent: read A[1] = %i, A[2] = %i\n", 
                Array[1], Array[2]);

        /*Important: Remember to detach the shared memory region*/
        shmdt( shdMemRegion );
    } else {            //Child

        printf("%i is the child\n", getpid());

        int* Array = (int*) shdMemRegion;

        //Wait for parent to finish writing
        while(Array[0] != 9999){
            sleep(1);           //Wait for a while
        }

        printf("Child: read A[1] = %i, A[2] = %i\n", 
                Array[1], Array[2]);

        Array[1] = 4321;
        Array[2] = 8765;
        Array[3] = 1111;    //Indicate writing is done for the parent

        printf("Child: Mischief achieved!\n");

        /*Important: Remember to detach the shared memory region*/
        shmdt( shdMemRegion );

        return 0;   //Child Ends here

    }

    /*Important: Remember to remove the shared memory region after use!*/
    shmctl(shdMemId, IPC_RMID, NULL); 
    return 0;
}
