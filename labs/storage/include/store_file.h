#ifndef CMASTERY_STORE_FILE_H
#define CMASTERY_STORE_FILE_H
#include "store.h"

typedef struct store_file
{
    int fd;
    uint32_t pages;
} store_file;

/* Open an existing, already allocated regular image. Ownership of fd stays
 * here. */
int store_file_open(store_file *f, const char *path, uint32_t pages);
void store_file_close(store_file *f);
store_io store_file_io(store_file *f);
#endif
