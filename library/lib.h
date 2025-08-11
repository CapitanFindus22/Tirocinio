#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <x86intrin.h>
#ifdef NO_DEV
#include <math.h>
#endif
#include "cmd.h"

int file_desc = -1;
void *bfr;

struct timespec start, end;

/**
 * Args of the node
 */
typedef union
{

    int i;
    size_t s;
    double d;

} arg_type;

/**
 * Nodes of the queue
 */
typedef struct node
{

    int cmd;
    arg_type arg[5];
    struct node *next;

} node;

/**
 * A queue
 */
struct queue
{

    node *head;
    node *tail;

} q;

void add1(size_t, size_t);
void togrey(size_t, size_t);
void convol(size_t, size_t);
void *copy(void *, const void *, size_t);

/**
 * Initialize the library
 */
void init()
{
    file_desc = open("/dev/disp", O_RDWR);

    if (file_desc < 0)
    {
        perror("Error opening device");
        exit(EXIT_FAILURE);
    }
}

/**
 * Get the device shared buffer
 */
void *get_buff()
{
    bfr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, file_desc, 0);

    if (bfr == MAP_FAILED)
    {
        perror("mmap error");
        exit(EXIT_FAILURE);
    }

    return bfr;
}

/**
 * Clear the buffer
 */
void clear_buff()
{
    ioctl(file_desc, clr_buff, 0);
}

void *copy(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--)
    {
        *d++ = *s++;
    }
    return dest;
}

/**
 * Insert a command in the queue
 * @param cmd Defined in cmd.h
 * @param num The number of arguments passed, see the corrisponding function
 * @return -1 if an error occured
 */
int enqueue(int cmd, int num, ...)
{

    node *new_node = (node *)malloc(sizeof(node));

    if (!new_node)
    {
        perror("Error creating node");
        return -1;
    }

    new_node->cmd = cmd;
    new_node->next = NULL;

    va_list valist;
    va_start(valist, num);

    new_node->arg[0].s = va_arg(valist, size_t);
    new_node->arg[1].s = va_arg(valist, size_t);

    /*switch (cmd)
    {
    case add_1:
        new_node->arg[0].s = va_arg(valist, size_t);
        new_node->arg[1].s = va_arg(valist, size_t);
        break;
    case to_grey:
        new_node->arg[0].s = va_arg(valist, size_t);
        new_node->arg[1].s = va_arg(valist, size_t);
        break;
    case conv:
        new_node->arg[0].s = va_arg(valist, size_t);
        new_node->arg[1].s = va_arg(valist, size_t);
        break;
    default:
        break;
    }*/

    va_end(valist);

    if (q.head == NULL)
    {
        q.head = new_node;
        q.tail = new_node;
    }

    else
    {
        q.tail->next = new_node;
        q.tail = new_node;
    }
}
/**
 * Clear the queue
 */
void clear_queue()
{

    node *p = q.head;

    node *n;

    while (p != NULL)
    {
        n = p->next;
        free(p);
        p = n;
    }

    q.head = NULL;
    q.tail = NULL;
}

/**
 * Execute all commands in the queue
 */
void ex_queue()
{

    node *p = q.head;

    uint64_t tm = __rdtsc();

    while (p != NULL)
    {

        switch (p->cmd)
        {
        case add_1:
            add1(p->arg[0].s, p->arg[1].s);
            break;
        case to_grey:
            togrey(p->arg[0].s, p->arg[1].s);
            break;
        case conv:
            convol(p->arg[0].s, p->arg[1].s);
            break;
        default:
            break;
        }
        p = p->next;
    }

    //printf("%f\n", (__rdtsc() - tm) / 2.6e9);

    ioctl(file_desc, print_time, (int)(1000000 * (__rdtsc() - tm) / 2.6e9));

    clear_queue();
}

#ifdef NO_DEV

/**
 * Add 1 to every cell of the matrix
 * @param rows The rows of the matrix
 * @param cols The columns of the matrix
 */
void add1(size_t rows, size_t cols)
{

    int *mtr = (int *)bfr;

    for (size_t i = 0; i < rows; i++)
    {

        for (size_t j = 0; j < cols; j++)
        {
            mtr[i * cols + j]++;
        }
    }

    return;
}

/**
 * Transform the RGB matrix in grayscale
 * @param rows The rows of the matrix
 * @param cols The columns of the matrix
 */
void togrey(size_t rows, size_t cols)
{

    RGB *mtr = (RGB *)bfr;

    for (size_t i = 0; i < rows; i++)
    {

        for (size_t j = 0; j < cols; j++)
        {

            mtr[i * cols + j].r = (77 * mtr[i * cols + j].r + 150 * mtr[i * cols + j].g + 29 * mtr[i * cols + j].b) >> 8;
            mtr[i * cols + j].g = mtr[i * cols + j].r;
            mtr[i * cols + j].b = mtr[i * cols + j].r;
        }
    }

    return;
}

/**
 * Convolution for edge detection, need a grayscale matrix
 * @param rows The rows of the matrix
 * @param cols The columns of the matrix
 */
void convol(size_t rows, size_t cols)
{

    int kernelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};

    int kernelY[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};

    RGB *ptr = (RGB *)bfr;

    RGB *mtr = (RGB *)malloc(rows * cols * sizeof(RGB));

    int vx, vy;

    for (size_t i = 1; i < rows - 1; i++)
    {

        vx = 0;
        vy = 0;

        for (size_t j = 1; j < cols - 1; j++)
        {

            vx = ptr[(i - 1) * cols + j - 1].g * kernelX[0][0] +
                 ptr[(i - 1) * cols + j].g * kernelX[0][1] +
                 ptr[(i - 1) * cols + j + 1].g * kernelX[0][2] +
                 ptr[i * cols + j - 1].g * kernelX[1][0] +
                 ptr[i * cols + j].g * kernelX[1][1] +
                 ptr[i * cols + j + 1].g * kernelX[1][2] +
                 ptr[(i + 1) * cols + j - 1].g * kernelX[2][0] +
                 ptr[(i + 1) * cols + j].g * kernelX[2][1] +
                 ptr[(i + 1) * cols + j + 1].g * kernelX[2][2];

            vy = ptr[(i - 1) * cols + j - 1].g * kernelY[0][0] +
                 ptr[(i - 1) * cols + j].g * kernelY[0][1] +
                 ptr[(i - 1) * cols + j + 1].g * kernelY[0][2] +
                 ptr[i * cols + j - 1].g * kernelY[1][0] +
                 ptr[i * cols + j].g * kernelY[1][1] +
                 ptr[i * cols + j + 1].g * kernelY[1][2] +
                 ptr[(i + 1) * cols + j - 1].g * kernelY[2][0] +
                 ptr[(i + 1) * cols + j].g * kernelY[2][1] +
                 ptr[(i + 1) * cols + j + 1].g * kernelY[2][2];

            mtr[i * cols + j].g = (uint8_t)sqrt(vx * vx + vy * vy);

            mtr[i * cols + j].b = mtr[i * cols + j].g;
            mtr[i * cols + j].r = mtr[i * cols + j].g;
        }
    }

    copy(ptr, mtr, rows * cols * sizeof(RGB));

    free(mtr);

    return;
}

#else

/**
 * Add 1 to every cell of the matrix
 * @param rows The rows of the matrix
 * @param cols The columns of the matrix
 */
void add1(size_t rows, size_t cols)
{

    ioctl(file_desc, wr_args, rows);
    ioctl(file_desc, wr_args, cols);
    ioctl(file_desc, rst_offset, 0);

    ioctl(file_desc, wr_func, add_1);
}

/**
 * Transform the RGB matrix in grayscale
 * @param rows The rows of the matrix
 * @param cols The columns of the matrix
 */
void togrey(size_t rows, size_t cols)
{

    ioctl(file_desc, wr_args, rows);
    ioctl(file_desc, wr_args, cols);
    ioctl(file_desc, rst_offset, 0);

    ioctl(file_desc, wr_func, to_grey);
}

/**
 * Convolution for edge detection
 * @param rows The rows of the matrix
 * @param cols The columns of the matrix
 */
void convol(size_t rows, size_t cols)
{

    ioctl(file_desc, wr_args, rows);
    ioctl(file_desc, wr_args, cols);
    ioctl(file_desc, rst_offset, 0);

    ioctl(file_desc, wr_func, conv);
}

#endif

/**
 * Free and close everything
 */
void finish()
{
    clear_buff();
    clear_queue();
    munmap(bfr, SIZE);
    close(file_desc);
}
