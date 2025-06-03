#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int fd = open("/dev/disp", O_WRONLY);

    if (fd < 0)
    {
        return -1;
    }

    write(fd, "afwgtjnf", 8);

    close(fd);

    return 0;
}
