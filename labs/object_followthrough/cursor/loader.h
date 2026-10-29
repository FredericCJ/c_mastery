#ifndef OC_LOADER_H
#define OC_LOADER_H
#include <stddef.h>
#include <stdint.h>

struct oc_plugin
{
    uint32_t abi;
    size_t size;
    int (*apply)(int);
};

struct oc_module
{
    void *handle;
    const struct oc_plugin *table;
    unsigned borrows;
};

enum
{
    OC_LOAD_OK = 0,
    OC_LOAD_BAD = -1,
    OC_LOAD_BUSY = -2
};

int oc_open(struct oc_module *, const char *);
int oc_borrow(struct oc_module *, const struct oc_plugin **);
int oc_return(struct oc_module *, const struct oc_plugin **);
int oc_close(struct oc_module *);
/* Explicit Linux/POSIX loader boundary, outside the ISO-only core. */
int oc_function_lookup(struct oc_module *, int *);
#endif
