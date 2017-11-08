/*************************************
* Lab 5 Exercise 2
* Name: Xu Bili
* Student No: A0124368A
* Lab Group: 4
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define COMMAND_READ 1
#define COMMAND_FORWARD 2
#define COMMAND_BACKWARD 3

void printOffset(off_t offset);
int readAndPrintInt(int fdIn, int nItems, int nBytes);
int readAndPrintChar(int fdIn, int nItems, int nBytes);

int main()
{
    int fdIn, option, nItems, nBytes, fileSize;
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
    if (fdIn == -1) {
        printf("Cannot Open\n");
        return 1;
    }

    fileSize = lseek(fdIn, 0, SEEK_END); // Read to the end of the file
    printf("Size = %i bytes\n", fileSize);
    lseek(fdIn, 0, SEEK_SET); // Reset the FD to start of the file again

    //Read and perform the specified options
    off_t offset = 0;
    while( scanf("%i %i %i", &option, &nItems, &nBytes) == 3) {
        switch (option) {
            case COMMAND_READ:
                if (nBytes == 4) {
                    offset += readAndPrintInt(fdIn, nItems, nBytes);
                } else if (nBytes == 1) {
                    offset += readAndPrintChar(fdIn, nItems, nBytes);
                } else {
                    printf("failed");
                    break;
                }

                break;
            case COMMAND_FORWARD:
                if (offset + nItems * nBytes > fileSize) {
                    printf("not allowed\n");
                    break;
                }

                offset = lseek(fdIn, nItems * nBytes, SEEK_CUR);
                printOffset(offset);
                break;
            case COMMAND_BACKWARD:
                offset = lseek(fdIn, - nItems * nBytes, SEEK_CUR);
                printOffset(offset);
                break;
            default:
                printf("Invalid command.\n");
                return 1;
        }
    }

    close( fdIn );  //good practice

    return 0;
}

// HELPER
// Prints out offset if the current offset value is valid.
void printOffset(off_t offset)
{
    if (offset == -1) {
        printf("not allowed\n");
    } else {
        printf("%d\n", (int) offset);
    }
}

// HELPER
// Read and print out integers from file.
//
// Returns the number of bytes read from file.
int readAndPrintInt(int fdIn, int nItems, int nBytes)
{
    int buffer[nItems];
    int bytesRead = read(fdIn, buffer, nItems * nBytes);
    for (int i = 0; i < bytesRead / nBytes; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");

    return bytesRead;
}

// HELPER
// Read and print out characters from file.
//
// Returns the number of bytes read from file.
int readAndPrintChar(int fdIn, int nItems, int nBytes)
{
    char buffer[nItems];
    int bytesRead = read(fdIn, buffer, nItems * nBytes);
    for (int i = 0; i < bytesRead / nBytes; i++) {
        printf("%c ", buffer[i]);
    }
    printf("\n");

    return bytesRead;
}
