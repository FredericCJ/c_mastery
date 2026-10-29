#include "loader.h"
#include <dlfcn.h>
#include <limits.h>
int oc_open(struct oc_module *m,const char *path)
{
    void *handle;const struct oc_plugin *table;const char *error;
    if(!m || !path || m->handle || m->table || m->borrows)return OC_LOAD_BAD;
    handle=dlopen(path,RTLD_NOW|RTLD_LOCAL);if(!handle)return OC_LOAD_BAD;
    dlerror();table=dlsym(handle,"oc_plugin");error=dlerror();
    if(error || !table || table->abi!=1 || table->size!=sizeof(struct oc_plugin) || !table->apply){m->handle=handle;(void)oc_close(m);return OC_LOAD_BAD;}
    m->handle=handle;m->table=table;return OC_LOAD_OK;
}
int oc_borrow(struct oc_module *m,const struct oc_plugin **out)
{
    if(!m || !out || !m->handle || !m->table || m->borrows==UINT_MAX)return OC_LOAD_BAD;
    ++m->borrows;*out=m->table;return OC_LOAD_OK;
}
int oc_return(struct oc_module *m,const struct oc_plugin **p)
{
    if(!m || !p || !m->handle || !m->borrows || *p!=m->table)return OC_LOAD_BAD;
#ifndef OC_MUTANT_RETURN
    --m->borrows;*p=NULL;
#endif
    return OC_LOAD_OK;
}
int oc_close(struct oc_module *m)
{
    if(!m || !m->handle)return OC_LOAD_BAD;
#ifdef OC_MUTANT_CLOSE
    if(m->borrows)return OC_LOAD_OK; /* Harmless false success; never actually unload live borrow. */
#else
    if(m->borrows)return OC_LOAD_BUSY;
#endif
    if(dlclose(m->handle))return OC_LOAD_BAD;
    m->handle=NULL;m->table=NULL;return OC_LOAD_OK;
}
