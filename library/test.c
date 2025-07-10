#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "lib.h"

int main(int argc, char **argv)
{
    init();

    wr_asd(0x1234567890);

    finish();

    return 0;
}
