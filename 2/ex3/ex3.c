/*************************************
* Lab 2 Exercise 3
* Name:
* Student No:
* Lab Group:
*************************************
Warning: Make sure your code works on
lab machine (Linux on x86)
*************************************/

#include <stdio.h>
#include <fcntl.h>      //For stat()
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>   //for waitpid()
#include <unistd.h>     //for fork(), wait()
#include <string.h>     //for string comparison etc
#include <stdlib.h>     //for malloc()


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
 * TODO: The return result for this function can be abstracted into an enum for
 * better clarity.
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
void readInput(char *command);

/**
 * Splits a command into an array of strings, delimited by space. Inspiration
 * from stringTokenizer.
 *
 * Parameters:
 * - command: command string to split up
 * - numTokens: allowed number of space delimited tokens in the command
 * - tokenSize: maximum size of each token in the input command
 *
 * Returns: an array of strings that represents the split up command
 */
char** tokenize(char *command, int numTokens, int tokenSize);

/**
 * Frees the array that was used to hold the tokens tokenized.
 */
void freeTokensArray(char **tokens, int size);

/**
 * Handles an invalid command
 */
void invalidCommand(char *path);

/**
 * Parses the command input by the user, transforming the command if
 * necessary.
 */
void checkLast(char *path, char *prev);

/**
 * Saves the previous command for use if `last` is invoked.
 */
void rememberCommand(char *path, char *prev);

/**
 * Create a new child process that runs the program located at `path`.
 */
void spawn(char *path, char **args, int parallel);


int main()
{
    // Entire command as a string
    char command[120];

    // Previous command
    char last[120] = "";

    // Tokenized command, delimited by whitespace
    char** tokens;

    // String that represents the path of the executable
    char *path;

    // Argument array, note that this array consists only of POINTERS to the
    // actual argument
    char *args[4];

    // 1 if program is to be run in parallel, otherwise block the shell
    int parallelFlag;

    readInput(command);
    while (strcmp(command, "quit") != 0) {

        // 1. Splits up the user input into the array of strings

        tokens = tokenize(command, 6, 120);
        path = tokens[0];
        parallelFlag = tokens[5] && strcmp(tokens[5], "&") == 0;

        // Set index 1 - (size-1) as the arguments
        for (int i = 1; i < 5; i++) {
            args[i-1] = tokens[i];
        }

        // 2. Check if path of executable is valid and exists
        //
        // 2a. Check if command is to wait for a specific PID. If PID exists
        // in our PID history, wait for it to complete - BLOCK. Return after
        // completion

        // TODO: Change this to make use of function pointers
        int fd = fileCheck(path);
        switch (fd) {
            case NON_EXECUTABLE:
                printf("%s is not an executable.\n", command);
                break;
            case NOT_FOUND:
                invalidCommand(command);
                break;
            default:
                // 3. Spawns a new process, keep track of the new process ID
                // in a data structure
                spawn(command, args, parallelFlag);
        }

        rememberCommand(command, last);
        readInput(command);
        checkLast(command, last);

        // Free up memory for new tokens
        freeTokensArray(tokens, 6);
        tokens = NULL;
    }

    printf("Goodbye!\n");
    return 0;
}

/***************************
 * FUNCTION IMPLEMENTATION *
 ***************************/

/**
 * Returns the valid file descriptor for the path of the executable.
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
 * Reads a line of command from the user.
 */
void readInput(char *command)
{
    printf("YWIMC > ");
    fgets(command, 120, stdin);

    // Get rid of the newline attached
    command[strcspn(command, "\n")] = '\0';
}

/**
 * Splits a command into an array of tokens delimited by space.
 */
char** tokenize(char *command, int numTokens, int tokenSize)
{
    const char delim[3] = " ";
    int i;
    char* tStart;
    char** tokens;
    tokens = (char**) malloc(sizeof(char*) * numTokens);

    // Nullify all entries
    for (i = 0; i < numTokens; i++) {
        tokens[i] = NULL;
    }

    tStart = strtok(command, delim);
    i = 0;
    while(i < numTokens && tStart) {
        // Allocate space for token string
        tokens[i] = (char*) malloc(sizeof(char) * tokenSize);

        // Ensure at most 19 + null characters are copied
        strncpy(tokens[i], tStart, tokenSize);

        // Add NULL terminator in the worst case
        tokens[i][tokenSize-1] = '\0';

        i++;
        tStart = strtok(NULL, delim);
    }

    return tokens;
}

/**
 * Iterate through the tokens array and frees the memory allocated by the
 * tokenize function.
 */
void freeTokensArray(char **tokens, int size) {
    for (int i = 0; i < size; i++) {
        if (tokens[i]) {
            free(tokens[i]);
        }
        tokens[i] = NULL;
    }

    free(tokens);

    // Caller needs to set tokens to NULL
}


/**
 * Handles an invalid command
 */
void invalidCommand(char *command)
{
    if (strcmp(command, "last") == 0) {
        printf("No previous command available.\n");
    } else {
        printf("%s not found\n", command);
    }
}


/**
 * Checks for the `last` command, we do not update the command unless `last`
 * is invoked.
 *
 * If `last` is invoked, we will update our current command to that of the
 * previous command that we input into the interpreter.
 */
void checkLast(char *command, char *prev)
{
    if (strcmp(command, "last") == 0 && strcmp(prev, "") != 0) {
        strcpy(command, prev);
    }
}


/**
 * Saves user's previous command only if command is not `last`. This is to prevent
 * an infinite loop.
 */
void rememberCommand(char *command, char *prev)
{
    if (strcmp(command, "last") != 0) {
        strcpy(prev, command);
    }
}


/**
 * Spawns a new child process that runs the program located at `path`.
 */
void spawn(char *path, char **args, int parallel)
{
    pid_t childPid;

    childPid = fork();
    if (childPid != 0) { // Parent
        // 4. Check if last argument is a background job request
        //
        // If it is: save the PID of child and proceed with next iteration
        //
        // If NOT: `wait()` for the PID to finish execution.
        waitpid(childPid, NULL, 0);
    } else { // Child
        //
        // 4. Execute with `execv()` and the arguments excluding the last
        // argument if it is background task
        execl(path, path, NULL);
    }
}
