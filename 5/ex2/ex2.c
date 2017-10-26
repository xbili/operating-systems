/*************************************
* Lab 5 Exercise 2
* Name:
* Student No:
* Lab Group:
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fdIn, option, nItems, nBytes;
    char fileName[40];

    //Checking 32-bit vs 64-bit. Do not remove!
    printf("Integer size is %d bytes. Should be 4 bytes (32-bit).\n",
            sizeof(int));

    if (sizeof(int) != 4){
        printf("64-bit environment detected!\n");
        printf("Please recompile with \"-m32\" flag\n");
        exit(1);
    }


    //Program proper
    printf("File Name: ");
    scanf("%s", fileName);

    fdIn = open(  fileName, O_RDONLY );

    //TODO: Copy from ex1, Check for valid file

    //TODO: Copy from ex1. Calculate the file size

    //Read and perform the specified options
    while( scanf("%i %i %i", &option, &nItems, &nBytes) == 3){
        //TODO: Handle the options here

    }
    
    close( fdIn );  //good practice

    return 0;
}
