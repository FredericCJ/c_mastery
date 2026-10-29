#ifndef LIBRARY_CONTROL_H
#define LIBRARY_CONTROL_H
#include <stdatomic.h>

enum lg_status
{
    LG_OK,
    LG_PENDING,
    LG_ARGUMENT,
    LG_FAILURE,
    LG_NOT_IMPLEMENTED
};

struct lg_owner
{
    void *allocation;
    unsigned acquired, released;
};
/* Single caller, no reentry; owner lives outside this destination frame. */
enum lg_status lg_jump(struct lg_owner *, unsigned mode, int *volatile_result);
enum lg_status lg_raise(void);

struct lg_publication
{
    atomic_int ready;
    int payload;
};

void lg_init(struct lg_publication *);
void lg_publish(struct lg_publication *, int);
enum lg_status lg_receive(struct lg_publication *, int *);
#endif
