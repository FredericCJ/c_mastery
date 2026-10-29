#ifndef DISPATCH_SPIKE_ABI_H
#define DISPATCH_SPIKE_ABI_H
#include <stddef.h>

struct spike_api
{
    unsigned abi;
    size_t size;
    int (*apply)(int);
};
#endif
