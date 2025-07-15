#include "lib.h"

int main(int argc, char **argv)
{
    init();

    int *bfr = (int*) get_buff();

    bfr[0] = 95;
    bfr[1] = 156;

    wr_asd(890);

    finish();

    return 0;
}
