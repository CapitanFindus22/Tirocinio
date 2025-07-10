int file_desc;

void init() {
    file_desc = open("/dev/disp",O_RDWR);
}

void wr_asd(unsigned long val) {
    ioctl(file_desc, 0, val);
}

void finish() {
    close(file_desc);
}