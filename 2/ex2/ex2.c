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


/*************
 * CONSTANTS *
 *************/

#define NON_EXECUTABLE 1
#define NOT_FOUND 2


/*************************
 * FUNCTION DECLARATIONS *
 *************************/

/**
 * Returns the valid file descriptor for the input path.
 *
 * Returns:
 * - NON_EXECUTABLE: if the file is not an executable
 * - NOT_FOUND: if the file directory does not exist
 * - 0: if the file exists and can be run
 */
int fileCheck(char *path);

/**
 * Reads a line of input from the user.
 */
void readInput(char *path);

/**
 * Handles an invalid command
 */
void invalidCommand(char *path);

/**
 * Parses the command input by the user, transforming the command if
 * necessary.
 */
void parseCommand(char *path, char *prev);

/**
 * Saves the previous command for use if `last` is invoked.
 */
void rememberCommand(char *path, char *prev);

/**
 * Create a new child process that runs the program located at `path`.
 */
void spawn(char *path);


/********
 * MAIN *
 ********/

int main()
{
    char path[20];
    char last[20] = "";

    readInput(path);
    while (strcmp(path, "quit") != 0) {
        // Check whether file exist
        // In real interpreter, a lot more checks are needed
        // E.g. check for file type, execution right etc

        // TODO: Change this to make use of function pointers
        int fd = fileCheck(path);
        switch (fd) {
            case NON_EXECUTABLE:
                printf("%s is not an executable.\n", path);
                break;
            case NOT_FOUND:
                invalidCommand(path);
                break;
            default:
                spawn(path);
        }

        rememberCommand(path, last);
        readInput(path);
        parseCommand(path, last);
    }

    printf("Goodbye!\n");
    return 0;
}


/***************************
 * FUNCTION IMPLEMENTATION *
 ***************************/

/**
 * Returns the valid file descriptor for the input path.
 *
 * Returns:
 * - NON_EXECUTABLE: if the file is not an executable
 * - NOT_FOUND: if the file directory does not exist
 * - 0: if the file exists and can be run
 */
int fileCheck(char *path)
{
    struct stat fileStat;
    if (stat(path, &fileStat) == -1) {
        return NOT_FOUND;
    }

    if (!S_ISREG(fileStat.st_mode)) {
        return NON_EXECUTABLE;
    }

    return 0;
}


/**
 * Reads a line of input from the user.
 */
void readInput(char *path)
{
    printf("YWIMC > ");
    scanf("%s", path);
}


/**
 * Handles an invalid command
 */
void invalidCommand(char *path)
{
    if (strcmp(path, "last") == 0) {
        printf("No previous command available.\n");
    } else {
        printf("%s not found\n", path);
    }
}


/**
 * Checks for the `last` command, we do not update the command unless `last`
 * is invoked.
 *
 * If `last` is invoked, we will update our current command to that of the
 * previous command that we input into the interpreter.
 */
void parseCommand(char *path, char *prev)
{
    if (strcmp(path, "last") == 0 && strcmp(prev, "") != 0) {
        strcpy(path, prev);
    }
}


/**
 * Saves user's previous command only if path is not `last`. This is to prevent
 * an infinite loop.
 */
void rememberCommand(char *path, char *prev)
{
    if (strcmp(path, "last") != 0) {
        strcpy(prev, path);
    }
}


/**
 * Spawns a new child process that runs the program located at `path`.
 */
void spawn(char *path)
{
    pid_t childPid;

    childPid = fork();
    if (childPid != 0) { // Parent
        waitpid(childPid, NULL, 0);
    } else { // Child
        execl(path, path, NULL);
    }
}

