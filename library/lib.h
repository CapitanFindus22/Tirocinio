#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "cmd.h"

int file_desc;

void init() {
    file_desc = open("/dev/disp", O_RDWR);
}

void *get_buff() {
    return mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, file_desc, 0);
}

void wr_asd(int val) {
    ioctl(file_desc, wr_args, 1);
    ioctl(file_desc, wr_args, 2);
    ioctl(file_desc, rd_args, 0);

    ioctl(file_desc, rst_offset, 0);

    ioctl(file_desc, wr_func, add_1);
}

void finish() {
    close(file_desc);
}
