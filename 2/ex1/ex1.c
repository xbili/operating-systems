/*************************************
* Lab 2 Exercise 1
* Name: Xu Bili
* Student No: A0124368A
* Lab Group: 4
*************************************
Warning: Make sure your code works on
lab machine (Linux on x86)
*************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>     //for fork()
#include <sys/wait.h>   //for wait()

int main()
{
    int nChild;

    //Read the number of child
    scanf("%d", &nChild);

    // Make sure that N is between 0 and 9
    if (nChild < 1 || nChild > 9) return 1;

    pid_t pids[nChild];

    int childPid, pid;
    for (int i = 0; i < nChild; i++) {
        childPid = fork();
        pid = getpid();
        if (childPid != 0) { // Parent
            pids[i] = childPid;
        } else { // Child
            printf("Child %d[%d]: Hello!\n", i, pid);
            return 0; // Exit
        }
    }

    // Wait on child processes in order
    for (int i = 0; i < nChild; i++) {
        childPid = waitpid(pids[i], NULL, 0);
        printf("Parent: Child %d[%d] done.\n", i, childPid);
    }

    printf("Parent: Exiting.\n");

    return 0;
}
