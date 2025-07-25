#include <x86intrin.h>
#include <time.h>
#include "lib.h"

#define ROWS 200
#define COLS 200

int main(int argc, char **argv)
{
    srand(time(NULL));

    init();

    uint64_t start = __rdtsc();

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
            //printf("%d ", bfr[i * COLS + j]);
        }

        //printf("\n");
    }

    printf("%llu\n", __rdtsc() - start);

    finish();

    return 0;
}
