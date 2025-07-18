#include "lib.h"
#include <time.h>

#define ROWS 5
#define COLS 7

int main(int argc, char **argv)
{
    srand(time(NULL));

    init();

    RGB *bfr = (RGB *)get_buff();

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            bfr[i * COLS + j].r = rand() % 256;
            bfr[i * COLS + j].g = rand() % 256;
            bfr[i * COLS + j].b = rand() % 256;
        }
    }

    if (enqueue(to_grey, 2, ROWS, COLS) < 0)
        return -1;

    ex_queue();

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            printf("\033[38;5;%dm█\033[0m", 232 + (bfr[i * COLS + j].g * 23) / 255);
        }

        printf("\n");
    }

    clear_buff();

    int *bfr2 = (int*)bfr;

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            bfr2[i * COLS + j] = rand() % 256;
        }
    }

    if (enqueue(add_1, 2, ROWS, COLS) < 0)
        return -1;
    if (enqueue(add_1, 2, ROWS, COLS) < 0)
        return -1;
    if (enqueue(add_1, 2, ROWS, COLS) < 0)
        return -1;
    if (enqueue(add_1, 2, ROWS, COLS) < 0)
        return -1;

    ex_queue();

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            printf("%d ", bfr2[i * COLS + j]);
        }

        printf("\n");
    }

    finish((void *)bfr);

    return 0;
}
