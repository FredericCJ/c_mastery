#ifndef OC_LOADER_H
#define OC_LOADER_H
#include <stddef.h>
#include <stdint.h>
typedef struct { uint32_t abi; size_t size; int (*apply)(int); } OcPlugin;
typedef struct { void *handle; const OcPlugin *table; unsigned borrows; } OcModule;
enum { OC_LOAD_OK=0,OC_LOAD_BAD=-1,OC_LOAD_BUSY=-2 };
int oc_open(OcModule *,const char *);
int oc_borrow(OcModule *,const OcPlugin **);
int oc_return(OcModule *,const OcPlugin **);
int oc_close(OcModule *);
/* Explicit Linux/POSIX loader boundary, outside the ISO-only core. */
int oc_function_lookup(OcModule *,int *);
#endif
