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
    int parallel;
    node* bgProcs;
} context;

/**
 * A function type that represents execution of a command.
 *
 * Takes in a context and returns null.
 */
typedef void (*execute) (context*);


/********************
 * EXECUTE COMMANDS *
 ********************/

/**
 * Multiplexes the command into individual functions.
 */
execute commandMux(context *ctx);

/**
 * Wait for child process to finish executing.
 */
void waitForChild(context *ctx);

/**
 * Prints the current background tasks present in the context.
 */
void printchild(context *ctx);

/**
 * Launches the program requested by the user.
 */
void launch(context *ctx);


/***************************
 * SHELL CONTEXT FUNCTIONS *
 ***************************/

/**
 * Create a new child process given the current shell context.
 */
void spawn(context *ctx);

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
 * Saves command executed as previous command in shell context
 */
void rememberCommand(context *ctx, char *command);

/**
 * Parses the command input by the user, transforming the command if
 * necessary.
 */
void checkPrevious(context *ctx, char *command);

/**
 * Frees up memory used by the shell context.
 */
void freeContext(context *ctx);


/*************************
 * LINKEDLIST OPERATIONS *
 *************************/

node* addToHead(node* head, int newData);
node* removeFromList(node* head, int value);
void destroyList(node* head);
int exists(node* head, int value);


/***********
 * HELPERS *
 ***********/

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
 * Forces the shell to wait for a child process.
 */
void forceWait(pid_t childPid);


/*********
 * DEBUG *
 *********/

/**
 * Prints out on screen information about the current shell context.
 */
void debugContext(context *ctx);


/********
 * MAIN *
 ********/

int main()
{
    // Allocate memory for shell context
    context *ctx = malloc(sizeof(context));

    // Entire command as a string
    char command[120];

    // Action to take
    execute action;

    readInput(command);
    while (strcmp(command, "quit") != 0) {
        // Checks if user is asking to run the previous command
        checkPrevious(ctx, command);

        setTokens(ctx, command);
        setParallelFlag(ctx);

        // Get the appropriate action (as a function ptr) and run it
        action = commandMux(ctx);
        (*action)(ctx);

        // Reset context
        resetParallelFlag(ctx);
        resetTokens(ctx);

        rememberCommand(ctx, command);

        // Next input
        readInput(command);
    }

    printf("Goodbye!\n");

    // Clean up
    freeContext(ctx);

    return 0;
}


/********************
 * EXECUTE COMMANDS *
 ********************/

/**
 * Multiplexes the command to return different execution implementations.
 *
 * Returns a function pointer to an `execute` function.
 */
execute commandMux(context *ctx)
{
    char *path = ctx->tokens[0];
    if (strcmp(path, "wait") == 0) {
        return waitForChild;
    } else if (strcmp(path, "printchild") == 0) {
        return printchild;
    } else {
        return launch;
    }
}

/**
 * Wait for child process to finish executing.
 */
void waitForChild(context *ctx)
{
    // Check if command is to wait for a specific PID. If PID exists
    // in our PID history, wait for it to complete - BLOCK.
    pid_t childPid = (int) strtol(ctx->tokens[1], NULL, 10);
    if (isBackgroundTask(ctx, childPid)) {
        forceWait(childPid);
        removeBackgroundTask(ctx, childPid);
    } else {
        printf("%d not a valid child pid\n", childPid);
    }
}


/**
 * Prints the current background tasks present in the context.
 */
void printchild(context *ctx)
{
    // Print out all background processes
    printf("Unwaited Child Processes:\n");
    node* curr = ctx->bgProcs;
    while (curr) {
        printf("%d\n", curr->data);
        curr = curr->next;
    }
}


/**
 * Launches the program requested by the user.
 */
void launch(context *ctx)
{
    // Check if path of executable is valid and exists
    int fd = fileCheck(ctx->tokens[0]);
    switch (fd) {
        case NON_EXECUTABLE:
            printf("%s is not an executable.\n", ctx->tokens[0]);
            break;
        case NOT_FOUND:
            invalidCommand(ctx->tokens[0]);
            break;
        default:
            spawn(ctx);
    }
}


/***************************
 * SHELL CONTEXT FUNCTIONS *
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
 * Saves user's previous command only if command is not `last`. This is to prevent
 * an infinite loop.
 */
void rememberCommand(context *ctx, char *command)
{
    if (strcmp(command, "last") != 0) {
        strcpy(ctx->previous, command);
    }
}

/**
 * Checks for the `last` command, we do not update the command unless `last`
 * is invoked.
 *
 * If `last` is invoked, we will update our current command to that of the
 * previous command that we input into the interpreter.
 */
void checkPrevious(context *ctx, char *command)
{
    if (strcmp(command, "last") == 0 && strcmp(ctx->previous, "") != 0) {
        strcpy(command, ctx->previous);
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
        char *args[6];
        for (int i = 0; i < 5; i++) {
            if (strcmp(ctx->tokens[i], "") == 0 || strcmp(ctx->tokens[i], "&") == 0) {
                args[i] = NULL; // Nullify empty strings or parallel token
            } else {
                args[i] = ctx->tokens[i];
            }
        }

        // `execv` requires the array of pointers to be terminated with a NULL
        // element.
        args[5] = NULL;

        execv(ctx->tokens[0], args);
    }
}

/**
 * Frees up memory allocated to the shell context.
 */
void freeContext(context *ctx)
{
    destroyList(ctx->bgProcs);
    free(ctx);
}


/************************
 * LINKEDLIST FUNCTIONS *
 ************************/

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


/***********
 * HELPERS *
 ***********/

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
 * Waits for the background process with PID
 */
void forceWait(pid_t childPid)
{
    waitpid(childPid, NULL, 0);
}


/*********
 * DEBUG *
 *********/

void debugContext(context *ctx)
{
    // Prints out all tokens
    printf("Tokens:\n");
    for (int i = 0; i < 6; i++) {
        printf("Token #%d: %s\n", i, ctx->tokens[i]);
    }

    // Prints out last command
    printf("Previous command: %s\n", ctx->previous);

    // Prints out parallel flag
    printf("Parallel: %d\n", ctx->parallel);

    // Prints out all child processes
    printf("Child processes:\n");
    int i = 0;
    node *curr = ctx->bgProcs;
    while (curr) {
        printf("Child #%d: %d\n", i, curr->data);
        curr = curr->next;
        i++;
    }
}
