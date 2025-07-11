#ifndef cmd_h
#define cmd_h

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

// IOCTL commands
#define wr_func 0
#define wr_addr 1
#define wr_args 2

// Device functions
#define add_1 0


#endif