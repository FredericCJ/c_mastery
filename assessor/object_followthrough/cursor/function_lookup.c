/*
 * CM-SPECIMEN: conversion of the void * returned by dlsym() to a function
 * pointer, which ISO C does not define (POSIX does); a named Linux/POSIX
 * extension, rejected by gcc -pedantic-errors. Kept in its own file so that
 * no other loader code needs such a conversion (private assessor copy).
 */
#include "loader.h"
#include <dlfcn.h>

#define OC_FUNCTION_SYMBOL "oc_function"
#define OC_FUNCTION_ARGUMENT 9

int oc_function_lookup(struct oc_module *m, int *out)
{
    if (m == NULL || m->handle == NULL || m->table == NULL || out == NULL)
    {
        return OC_LOAD_BAD;
    }
    (void)dlerror();
    void *symbol = dlsym(m->handle, OC_FUNCTION_SYMBOL);
    if (dlerror() != NULL || symbol == NULL)
    {
        return OC_LOAD_BAD;
    }
    int (*function)(int) = (int (*)(int))symbol;
    *out = function(OC_FUNCTION_ARGUMENT);
    return OC_LOAD_OK;
}
