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
    char last[20];

    // Read user input
    printf("YWIMC > ");
    scanf("%s", path);

    struct stat fileStat;
    pid_t childPid;
    while (strcmp(path, "quit") != 0) {
        // Check whether file exist
        // In real interpreter, a lot more checks are needed
        // E.g. check for file type, execution right etc

        // TODO: Make sure that if there is no last command available, skip
        // the while-loop
        if (last[0] != '\0' && strcmp(path, "last") == 0) {
            strcpy(path, last);
        } else if (strcmp(path, "last") == 0) {
            continue;
        }

        if (stat(path, &fileStat) == -1) {
            printf("%s not found\n", path);
        } else if (S_ISREG(fileStat.st_mode)) {
            // Make sure that found file is a executable process

            childPid = fork();
            if (childPid != 0) { // Parent
                waitpid(childPid, NULL, 0);
            } else { // Child
                execl(path, path);
            }

            strcpy(last, path);
        } else {
            printf("%s is not an executable.\n", path);
        }

        printf("YWIMC > ");
        scanf("%s", path);
    }

    printf("Goodbye!\n");
    return 0;
}
