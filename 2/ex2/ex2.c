/*************************************
* Lab 2 Exercise 2
* Name: Xu Bili
* Student No: A0124368A
* Lab Group: 4
*************************************
Warning: Make sure your code works on
lab machine (Linux on x86)
*************************************/

#include <stdio.h>
#include <string.h>     //For string comparison, string copy
#include <fcntl.h>      //For stat()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>     //for fork()
#include <sys/wait.h>   //for wait()

int main()
{
    char path[20];
    char last[20] = "";

    // Read user input
    printf("YWIMC > ");
    scanf("%s", path);

    struct stat fileStat;
    pid_t childPid;
    while (strcmp(path, "quit") != 0) {
        // Check whether file exist
        // In real interpreter, a lot more checks are needed
        // E.g. check for file type, execution right etc

        if (stat(path, &fileStat) == -1) {
            if (strcmp(path, "last") == 0) {
                printf("No previous command available.\n");
            } else {
                printf("%s not found\n", path);
            }
        } else if (S_ISREG(fileStat.st_mode)) {
            // Make sure that found file is a executable process

            childPid = fork();
            if (childPid != 0) { // Parent
                waitpid(childPid, NULL, 0);
            } else { // Child
                execl(path, path, NULL);
            }
        } else {
            printf("%s is not an executable.\n", path);
        }

        // Save previous command only if path is not `last`.
        // This is done to prevent an infinite loop.
        if (strcmp(path, "last") != 0) {
            strcpy(last, path);
        }

        printf("YWIMC > ");
        scanf("%s", path);

        // Checks for the `last` command, and update the path accordingly
        if (strcmp(path, "last") == 0 && strcmp(last, "") != 0) {
            strcpy(path, last);
        }
    }

    printf("Goodbye!\n");
    return 0;
}
