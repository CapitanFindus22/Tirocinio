#include "lib.h"
#include <stdio.h>

#define ROWS 5
#define COLS 5

int main(int argc, char **argv)
{
    srand(time(NULL));

    init();

    RGB *bfr = (RGB *)get_buff();

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            bfr[i * COLS + j].r = rand()%256;
            bfr[i * COLS + j].g = rand()%256;
            bfr[i * COLS + j].b = rand()%256;
        }
    }

    enqueue(to_grey, 2, ROWS, COLS);
    ex_queue();

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            //printf("%d-%d-%d ", bfr[i * COLS + j].r, bfr[i * COLS + j].g, bfr[i * COLS + j].b);
            printf("\033[38;5;%dm█\033[0m", 232 + (bfr[i * COLS + j].g * 23) / 255);
        }

        printf("\n");
    }

    finish((void *)bfr);

    return 0;
}
