#include "lib.h"
#include <time.h>

#define ROWS 20
#define COLS 20

int main(int argc, char **argv)
{
    srand(time(NULL));

    init();

    int *bfr = (int *)get_buff();

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            bfr[i * COLS + j] = rand() % 256;
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
            printf("%d ", bfr[i * COLS + j]);
        }

        printf("\n");
    }

    finish((void *)bfr);

    return 0;
}
