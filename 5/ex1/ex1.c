/*************************************
* Lab 5 Exercise 1
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
    int fdIn,  fileSize;
    int *ip;
    char fileName[40], buffer[4];

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

    //Open file for read only
    fdIn = open(  fileName, O_RDONLY );

    //TODO:Check for valid file
    if (fdIn == -1) {
        printf("Cannot Open\n");
        return 1;
    }

    //TODO:Calculate the file size
    fileSize = lseek(fdIn, 0, SEEK_END); // Read to the end of the file
    printf("Size = %i bytes\n", fileSize);
    lseek(fdIn, 0, SEEK_SET); // Reset the FD to start of the file again

    //TODO: Change the code below so that it reads until the end of file
    // Right now, this only read the first item.
    while (read(fdIn, buffer, 4)) {
        //This is how you can "convert" bytes into a single integer
        ip = (int*) &buffer;
        printf("%d\n", *ip);
    };

    close( fdIn );  //good practice

    return 0;
}
