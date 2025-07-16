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

void clear_buff() {
    ioctl(file_desc, clr_buff, 0);
}

void add1(size_t rows, size_t cols) {

    ioctl(file_desc, wr_args, rows);
    ioctl(file_desc, wr_args, cols);
    ioctl(file_desc, rst_offset, 0);

    ioctl(file_desc, wr_func, add_1);

}

void finish() {
    close(file_desc);
}
