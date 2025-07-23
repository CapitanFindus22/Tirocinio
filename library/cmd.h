#ifndef cmd_h
#define cmd_h

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#define SIZE 2097152

// IOCTL commands
#define DISP_IOCTL_MAGIC 'D'

#define wr_func _IOW(DISP_IOCTL_MAGIC, 0, uint32_t)
#define wr_args _IOW(DISP_IOCTL_MAGIC, 1, uint32_t)

#define rst_offset _IO(DISP_IOCTL_MAGIC, 2)
#define clr_buff _IO(DISP_IOCTL_MAGIC, 3)

// User functions, each correspond to a function in lib.h
#define add_1 0   // add1 function
#define to_grey 1 // togrey function
#define conv 2 // convol function

typedef struct
{

    uint8_t r, g, b;

} RGB;

#endif