#include <x86intrin.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#define ROWS 200
#define COLS 200

typedef struct
{

    uint8_t r, g, b;

} RGB;

int main(int argc, char **argv)
{
    srand(time(NULL));

    RGB *bfr = malloc(ROWS * COLS * sizeof(RGB));

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            bfr[i * COLS + j].r = rand() % 256;
            bfr[i * COLS + j].g = rand() % 256;
            bfr[i * COLS + j].b = rand() % 256;
        }
    }

    uint64_t start = __rdtsc();

    mlock(bfr, ROWS * COLS * sizeof(RGB));

    for (short i = 0; i < ROWS; i++)
    {

        for (short j = 0; j < COLS; j++)
        {

            bfr[i * COLS + j].r = (77 * bfr[i * COLS + j].r + 150 * bfr[i * COLS + j].g + 29 * bfr[i * COLS + j].b) >> 8;
            bfr[i * COLS + j].g = bfr[i * COLS + j].r;
            bfr[i * COLS + j].b = bfr[i * COLS + j].r;
        }
    }

    munlock(bfr, ROWS * COLS * sizeof(RGB));

    printf("%llu\n", __rdtsc() - start);

    free(bfr);

    return 0;
}