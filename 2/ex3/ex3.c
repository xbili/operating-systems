/*************************************
* Lab 2 Exercise 3
* Name: Xu Bili
* Student No: A0124368A
* Lab Group: 4
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

/************
 * TYPEDEFS *
 ************/

// A single linked list node
typedef struct NODE{
    int data;
    struct NODE* next;
} node;

/**
 * Encapsulates the current state of the shell.
 *
 * Contains information about:
 * - previous: previous command string input by user
 * - tokens: parsed tokens of the command, array of size 6
 * - bgProcs: linked list of background processes
 */
typedef struct CONTEXT {
    char previous[120];
    char tokens[6][120];
    node* bgProcs;
    int parallel;
} context;


/*************************
 * FUNCTION DECLARATIONS *
 *************************/


/********************
 * Shell operations *
 ********************/

/**
 * Tokenizes command and store tokens in shell context.
 */
void setTokens(context *ctx, char *command);

/**
 * Resets the tokens in the shell context.
 */
void resetTokens(context *ctx);

/**
 * Returns true if the PID is a background task in the shell context.
 */
int isBackgroundTask(context *ctx, pid_t pid);

/**
 * Adds a background task PID into shell context.
 */
void addBackgroundTask(context *ctx, pid_t pid);

/**
 * Removes a background task PID from the shell context.
 */
void removeBackgroundTask(context *ctx, pid_t pid);

/**
 * Sets the parallel flag inside shell context.
 */
void setParallelFlag(context *ctx);

/**
 * Resets the parallel flag of a shell context.
 */
void resetParallelFlag(context *ctx);

/**
 * Frees up memory used by the shell context.
 */
void freeContext(context *ctx);

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
 * Create a new child process given the current shell context.
 */
void spawn(context *ctx);

/**
 * Forces the shell to wait for a child process.
 */
void forceWait(pid_t childPid);


/*************************
 * LinkedList operations *
 *************************/

node* addToHead(node* head, int newData);
node* removeFromList(node* head, int value);
void destroyList(node* head);
int exists(node* head, int value);


/*********
 * Debug *
 *********/

/**
 * Prints out on screen information about the current user input
 */
void debugInput(char *path, char **args, char **tokens, int parallelFlag);


/********
 * MAIN *
 ********/

int main()
{
    // Allocate memory for shell context
    context *ctx = malloc(sizeof(context));

    // Store a LinkedList of background processes
    node* bgProcs = NULL;

    // Entire command as a string
    char command[120];

    // Previous command
    char last[120] = "";

    readInput(command);
    while (strcmp(command, "quit") != 0) {
        setTokens(ctx, command);
        setParallelFlag(ctx);

        if (strcmp(ctx->tokens[0], "wait") == 0) {
            // Check if command is to wait for a specific PID. If PID exists
            // in our PID history, wait for it to complete - BLOCK.
            pid_t childPid = (int) strtol(ctx->tokens[1], NULL, 10);
            if (isBackgroundTask(ctx, childPid)) {
                forceWait(childPid);
                removeBackgroundTask(ctx, childPid);
            } else {
                printf("%d not a valid child pid\n", childPid);
            }
        } else if (strcmp(ctx->tokens[0], "printchild") == 0) {
            // Print out all background processes
            printf("Unwaited Child Processes:\n");
            node* curr = bgProcs;
            while (curr) {
                printf("%d\n", curr->data);
                curr = curr->next;
            }
        } else {
            // Check if path of executable is valid and exists
            // TODO: Change this to make use of function pointers
            int fd = fileCheck(ctx->tokens[0]);
            switch (fd) {
                case NON_EXECUTABLE:
                    printf("%s is not an executable.\n", command);
                    break;
                case NOT_FOUND:
                    invalidCommand(command);
                    break;
                default:
                    // Spawns a new process, keep track of the new process ID
                    // in a data structure
                    spawn(ctx);
            }
        }

        resetParallelFlag(ctx);
        resetTokens(ctx);

        rememberCommand(command, last);
        readInput(command);
        checkLast(command, last);
    }

    destroyList(bgProcs);
    freeContext(ctx);
    printf("Goodbye!\n");

    return 0;
}

/***************************
 * FUNCTION IMPLEMENTATION *
 ***************************/

/**
 * Tokenizes and saves tokens into shell context.
 */
void setTokens(context *ctx, char *command)
{
    char **tokens = tokenize(command, 6, 120);
    for (int i = 0; i < 6; i++) {
        if (tokens[i] == NULL) continue;
        strcpy(ctx->tokens[i], tokens[i]);
    }
    freeTokensArray(tokens, 6);
}

/**
 * Resets the tokens in the shell context.
 */
void resetTokens(context *ctx)
{
    for (int i = 0; i < 6; i++) {
        ctx->tokens[i][0] = '\0';
    }
}

/**
 * Returns true if the PID is a background task in the shell context.
 */
int isBackgroundTask(context *ctx, pid_t pid)
{
    return exists(ctx->bgProcs, pid);
}

/**
 * Adds a background task PID into shell context.
 */
void addBackgroundTask(context *ctx, pid_t pid)
{
    // Add the PID as a node in the linked list of PIDs
    ctx->bgProcs = addToHead(ctx->bgProcs, pid);
}

/**
 * Removes a background task PID from the shell context.
 */
void removeBackgroundTask(context *ctx, pid_t pid)
{
    // Defend against invalid PIDs, this check should be done before this
    // method is called, but for good measure, let's check it here again.
    if (!exists(ctx->bgProcs, pid)) {
        return;
    }

    // Removes the background task PID from the linked list of PIDs
    ctx->bgProcs = removeFromList(ctx->bgProcs, pid);
}

/**
 * Set parallel flag in a context.
 */
void setParallelFlag(context *ctx)
{
    for (int i = 0; i < 5; i++) {
        if (ctx->tokens[i] && strcmp(ctx->tokens[i], "&") == 0) {
            ctx->parallel = 1;
            return;
        }
    }
}

/**
 * Resets the parallel flag of a shell context.
 */
void resetParallelFlag(context *ctx)
{
    ctx->parallel = 0;
}

/**
 * Frees up memory allocated to the shell context.
 */
void freeContext(context *ctx)
{
    destroyList(ctx->bgProcs);
    free(ctx);
}

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
    char** tokens = (char**) malloc(sizeof(char*) * numTokens);

    // Copy so that we don't mutate original command string
    char* copy = strdup(command);

    const char delim[3] = " ";
    char* tStart;
    int i;

    // Nullify all entries
    for (i = 0; i < numTokens; i++) {
        tokens[i] = NULL;
    }

    tStart = strtok(copy, delim);
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

    free(copy);

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
 *
 * Returns the linked list of background processes running
 */
void spawn(context *ctx)
{
    pid_t childPid;
    childPid = fork();
    if (childPid != 0) { // Parent
        if (!ctx->parallel) {
            waitpid(childPid, NULL, 0);
        } else {
            printf("Child %d in background\n", childPid);
            addBackgroundTask(ctx, childPid);
        }
    } else { // Child
        char *args[5];
        for (int i = 0; i < 5; i++) {
            if (strcmp(ctx->tokens[i], "") == 0 || strcmp(ctx->tokens[i], "&")) {
                args[i] = NULL; // Nullify empty strings or parallel token
            } else {
                args[i] = ctx->tokens[i];
            }
        }

        execv(ctx->tokens[0], args);
    }
}


/**
 * Waits for the background process with PID
 */
void forceWait(pid_t childPid)
{
    waitpid(childPid, NULL, 0);
}


/*************************
 * Linked List functions *
 *************************/

node* addToHead(node* head, int newData)
{
    node* added = malloc(sizeof(node));
    added->data = newData;
    added->next = head;

    return added;
}


// Returns 1 if value exists in linked list
int exists(node* head, int value)
{
    node* curr = head;
    while (curr) {
        if (curr->data == value) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}


node* removeFromList(node* head, int value)
{
    if (head->data == value) {
        // Keep temp variable of the new head
        node* nxt = head->next;

        // Free up memory
        free(head);
        head = NULL;

        return nxt;
    }

    node* curr = head;
    node* prev = NULL;

    while (curr) {
        if (curr->data == value) {
            // Remove the current node
            prev->next = curr->next;

            // Free up memory
            free(curr);
            curr = NULL;

            return head;
        }
        prev = curr;
        curr = curr->next;
    }

    return head;
}


void destroyList(node* head)
{
    node* curr = head;
    while (curr) {
        node* tmp = curr->next;
        free(curr);
        curr = tmp;
    }
}

/*****************
 * Debug helpers *
 *****************/

void debugInput(char *path, char **args, char **tokens, int parallelFlag)
{
    printf("Path: %s\n", path);
    printf("Args: ");
    for (int i = 0; i < 5; i++) {
        printf("%s ", args[i]);
    }

    printf("\nTokens: ");
    for (int i = 0; i < 6; i++) {
        printf("%s ", tokens[i]);
    }

    printf("\nParallel: %d\n", parallelFlag);
}
