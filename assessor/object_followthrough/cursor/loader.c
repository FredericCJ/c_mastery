/*
 * Reference loader boundary for the cursor laboratory (private assessor
 * copy; Linux/glibc dlopen family, hosted only). Contracts are in
 * labs/object_followthrough/cursor/loader.h.
 *
 * Test hooks: OC_MUTANT_RETURN and OC_MUTANT_CLOSE each break exactly one
 * contract so the campaign can prove that the tests reject the breakage.
 */
#include "loader.h"
#include <dlfcn.h>
#include <limits.h>

#define OC_PLUGIN_ABI_VERSION 1u
#define OC_PLUGIN_SYMBOL "oc_plugin"

/*
 * Ownership: the dlopen handle is held by the module from the moment it is
 * obtained. If the descriptor is rejected, the handle is parked in the
 * module and oc_close() is used to give it back, so a failed unload leaves
 * the module in the documented cleanup-only state (handle set, table NULL).
 */
int oc_open(struct oc_module *m, const char *path)
{
    if (m == NULL || path == NULL || m->handle != NULL || m->table != NULL ||
        m->borrows != 0)
    {
        return OC_LOAD_BAD;
    }
    void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL)
    {
        return OC_LOAD_BAD;
    }
    /* dlerror() is the authoritative failure check, because a NULL symbol
     * value can be legitimate for dlsym(); stale error state is cleared
     * first. A NULL table is rejected afterwards, together with a wrong ABI
     * version, size or missing apply. */
    (void)dlerror();
    const struct oc_plugin *table = dlsym(handle, OC_PLUGIN_SYMBOL);
    const char *error = dlerror();
    if (error != NULL || table == NULL || table->abi != OC_PLUGIN_ABI_VERSION ||
        table->size != sizeof(struct oc_plugin) || table->apply == NULL)
    {
        m->handle = handle;
        (void)oc_close(m);
        return OC_LOAD_BAD;
    }
    m->handle = handle;
    m->table = table;
    return OC_LOAD_OK;
}

int oc_borrow(struct oc_module *m, const struct oc_plugin **out)
{
    if (m == NULL || out == NULL || m->handle == NULL || m->table == NULL ||
        m->borrows == UINT_MAX)
    {
        return OC_LOAD_BAD;
    }
    ++m->borrows;
    *out = m->table;
    return OC_LOAD_OK;
}

int oc_return(struct oc_module *m, const struct oc_plugin **borrowed)
{
    if (m == NULL || borrowed == NULL || m->handle == NULL || m->borrows == 0 ||
        *borrowed != m->table)
    {
        return OC_LOAD_BAD;
    }
#ifndef OC_MUTANT_RETURN
    --m->borrows;
    *borrowed = NULL;
#endif
    return OC_LOAD_OK;
}

int oc_close(struct oc_module *m)
{
    if (m == NULL || m->handle == NULL)
    {
        return OC_LOAD_BAD;
    }
#ifdef OC_MUTANT_CLOSE
    if (m->borrows != 0)
    {
        /* Harmless false success: never unloads a module with a live
         * borrow. */
        return OC_LOAD_OK;
    }
#else
    if (m->borrows != 0)
    {
        return OC_LOAD_BUSY;
    }
#endif
    if (dlclose(m->handle) != 0)
    {
        return OC_LOAD_BAD;
    }
    m->handle = NULL;
    m->table = NULL;
    return OC_LOAD_OK;
}
