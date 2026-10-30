#ifndef OC_LOADER_H
#define OC_LOADER_H
#include <stddef.h>
#include <stdint.h>

/*
 * Exported descriptor of a loadable plugin: abi must be the supported
 * version, size must equal sizeof(struct oc_plugin), apply must not be NULL.
 */
struct oc_plugin
{
    uint32_t abi;
    size_t size; /* bytes */
    int (*apply)(int);
};

/*
 * One loaded module. Start with {0}. After a successful oc_open the handle
 * and table are set and borrows counts outstanding oc_borrow() results.
 * A non-NULL handle with a NULL table means only cleanup is owed: retry
 * oc_close(), never borrow.
 */
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

/*
 * Contract for the four functions below. Platform: Linux/glibc dynamic
 * loading, outside the ISO-only core. Not thread-safe; a module has exactly
 * one owner. Return OC_LOAD_OK, OC_LOAD_BAD for any rejection, or
 * OC_LOAD_BUSY from oc_close() while borrows are outstanding.
 */

/* Loads path and admits its oc_plugin descriptor. m must be empty. A module
 * that is rejected is unloaded again where possible. */
int oc_open(struct oc_module *m, const char *path);

/* Lends the descriptor: borrows increases and *out is set. */
int oc_borrow(struct oc_module *m, const struct oc_plugin **out);

/* Takes back the lent pointer (*borrowed must be the module's table); it is
 * set to NULL and borrows decreases. */
int oc_return(struct oc_module *m, const struct oc_plugin **borrowed);

/* Unloads the module once no borrow is outstanding and clears the module.
 * If the unload itself fails the module keeps its handle. */
int oc_close(struct oc_module *m);

/* Explicit Linux/POSIX loader boundary, outside the ISO-only core. For an
 * open module, looks up the exported symbol "oc_function", calls it once with
 * the argument 9 and stores the result in *out. */
int oc_function_lookup(struct oc_module *m, int *out);
#endif
