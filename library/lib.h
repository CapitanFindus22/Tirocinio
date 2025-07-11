#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "cmd.h"

int file_desc;

void init() {
    file_desc = open("/dev/disp",O_RDWR);
}

void wr_asd(int val) {

    ioctl(file_desc, wr_func, add_1);
    ioctl(file_desc, wr_addr, val);
    //ioctl(file_desc, wr_args, len);
}

void finish() {
    close(file_desc);
}