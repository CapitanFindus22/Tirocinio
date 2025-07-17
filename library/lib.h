#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "cmd.h"

int file_desc = -1;

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

// Prototypes
void add1(size_t, size_t);
void togrey(size_t, size_t);

/**
 * Initialize the library
 */
void init()
{
    file_desc = open("/dev/disp", O_RDWR);

    if (file_desc < 0)
    {
        perror("Errore nell'apertura");
        exit(EXIT_FAILURE);
    }
}

/**
 * Get the device buffer
 */
void *get_buff()
{
    void *bfr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, file_desc, 0);

    if (bfr == MAP_FAILED)
    {
        perror("Errore mmap");
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

/**
 * Insert a command in the queue
 * @param cmd Defined in cmd.h
 * @param num The number of arguments passed, see the corrisponding functio
 * @return -1 if an error occured
 */
int enqueue(int cmd, int num, ...)
{

    node *new_node = (node *)malloc(sizeof(node));

    if (!new_node)
    {
        perror("Errore allocazione nodo");
        return -1;
    }

    new_node->cmd = cmd;
    new_node->next = NULL;

    va_list valist;
    va_start(valist, num);

    switch (cmd)
    {
    case add_1:
        new_node->arg[0].s = va_arg(valist, size_t);
        new_node->arg[1].s = va_arg(valist, size_t);
        break;
    case to_grey:
        new_node->arg[0].s = va_arg(valist, size_t);
        new_node->arg[1].s = va_arg(valist, size_t);
        break;
    default:
        break;
    }

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
 * Execute all command in the queue
 */
void ex_queue()
{

    node *p = q.head;

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
        default:
            break;
        }
        p = p->next;
    }

    clear_queue();
}

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
 * Add 1 to every cell of the matrix
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
 * Free everything
 */
void finish(void *ptr)
{
    clear_buff();
    clear_queue();
    munmap(ptr, 4096);
    close(file_desc);
}
