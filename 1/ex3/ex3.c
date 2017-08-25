/*************************************
* Lab 1 Exercise 3
* Name     : Xu Bili
* Matric No: A0124368A
* Lab Group: 4
*************************************/

#include <stdio.h>
#include <stdlib.h> //for malloc() and free()

typedef struct POSITION {
    int x;
    int y;
} position;
typedef void (*move) (position*, int);

move getMove(int op);
void up(position* p, int steps);
void down(position* p, int steps);
void left(position* p, int steps);
void right(position* p, int steps);

int main()
{
    int op, steps;
    position* p = malloc(sizeof(position));
    p->x = 0;
    p->y = 0;

    while (scanf("%i %i", &op, &steps) != EOF) {
        move m = getMove(op - 1);
        (*m)(p, steps);

        printf("%d %d\n", p->x, p->y);
    }

    return 0;
}

move getMove(int op) {
    move moves[4] = { up, down, left, right };
    return moves[op];
}

void up(position* p, int steps) {
    p->y += steps;
}

void down(position* p, int steps) {
    p->y -= steps;
}

void left(position* p, int steps) {
    p->x -= steps;
}

void right(position* p, int steps) {
    p->x += steps;
}

