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

    while (scanf("%i %i %i", &position, &input, &copies) != EOF) {
        myList = insertAt(myList, position, copies, input);
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


// HELPER:
// Create a linked list of length k with same elements
node* createList(int copies, int value) {
    // Create a head node first
    node* head = malloc(sizeof(node));
    head->data = value;

    // `head` takes up one copy of what we want
    copies--;

    // Append the copies of our item to the back of the list
    node* curr = head;
    for (int i = 0; i < copies; i++) {
        node* added = malloc(sizeof(node));
        added->data = value;

        curr->next = added;
        curr = added;
    }

    return head;
}

// HELPER:
// Get the node that is the tail of the linked list
node* getTail(node* head) {
    node* curr = head;
    while (curr->next) {
        curr = curr->next;
    }

    return curr;
}

//Actual Function Implementations
node* insertAt(node* head, int position, int copies, int newValue)
{
    node* toInsert = createList(copies, newValue);

    // Our newly created list will be the result if we want to insert to an
    // empty list
    if (head == NULL) {
        return toInsert;
    }

    // If we want to insert the copies before the current head of the list
    if (position == 0) {
        node* tail = getTail(toInsert);
        tail->next = head;
        return toInsert;
    }

    // -1 because we want to insert the copies BEFORE the position
    int currPos = position - 1;

    // We shift the current pointer to the position we want to insert our
    // copies into
    node* curr = head;
    while (curr->next && currPos > 0) {
        curr = curr->next;
        currPos--;
    }

    node* tmp = curr->next;

    // Insert our newly created list
    curr->next = toInsert;

    // Connect the disconnected part of linked list back
    node* tail = getTail(curr);
    tail->next = tmp;

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
