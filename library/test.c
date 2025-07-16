#include "lib.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    init();

    int *bfr = (int*) get_buff();

    short val = 0;

    for (short i = 0; i < 10; i++)
    {
        for (short j = 0; j < 10; j++)
        {
            bfr[i * 10 + j] = val++;
        }
        
    }
    
    add1(10,10);

    for (short i = 0; i < 10; i++)
    {
        for (short j = 0; j < 10; j++)
        {
            printf("%d ", bfr[i * 10 + j]);
        }
        
        printf("\n");

    }

    clear_buff();

    finish();

    return 0;
}
