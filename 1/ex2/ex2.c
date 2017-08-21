/*************************************
* Lab 1 Exercise 2
* Name: Xu Bili
* Matric No: A0124368A
* Lab Group: 4
*************************************/

#include <stdio.h>
#include <stdlib.h> //for malloc() and free()

//Declaration of a Linked List Node

typedef struct NODE{
    int data;
    struct NODE* next;
} node;

//Function Prototypes
node* insertAt(node*, int, int, int);

void printList(node*);
void destroyList(node*);



int main()
{
    node* myList = NULL;    //Empty List
    int position, input, copies;

    // TODO: Figure out why scanf isn't working as it is supposed to be
    while (scanf("%i%i%i", &position, &input, &copies) == 1) {
        printf("Position: %d\n", position);
        printf("Input: %d\n", input);
        printf("Copies: %d\n", copies);
    }

    //Output code coded for you
    printf("My List:\n");
    printList(myList);

    destroyList(myList);
    myList = NULL;


    printf("My List After Destroy:\n");
    printList(myList);

    return 0;
}

//Actual Function Implementations
node* insertAt(node* head, int position, int copies, int newValue)
{
    // -1 because we want to insert the copies BEFORE the position
    int currPos = position - 1;

    // We shift the current pointer to the position we want to insert our
    // copies into
    node* curr = head;
    while (currPos > 0) {
        curr = curr->next;
        currPos--;
    }

    // We insert `copies` number of nodes
    for (int i = 0; i < copies; ++i) {
        node* tmp = curr->next;
        node* added = malloc(sizeof(node));
        added->data = newValue;

        curr->next = added;
        added->next = tmp;

        curr = added;
    }

    return head;
}

void printList(node* head)
//Purpose: Print out the linked list content
//Assumption: the list is properly null terminated
{
    //This function has been implemented for you
    node* ptr = head;

    while (ptr != NULL)  {    //or you can write while(ptr)
        printf("%i ", ptr->data);
        ptr = ptr->next;
    }
    printf("\n");
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
