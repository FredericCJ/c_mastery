#include "loader.h"
#include <dlfcn.h>
int oc_function_lookup(OcModule *m,int *out)
{
    void *symbol;int (*function)(int);int value;
    if(!m || !m->handle || !m->table || !out)return OC_LOAD_BAD;
    dlerror();symbol=dlsym(m->handle,"oc_function");
    if(dlerror() || !symbol)return OC_LOAD_BAD;
    function=(int (*)(int))symbol; /* Named Linux/POSIX extension, never labeled ISO C. */
    value=function(9);
    *out=value;return OC_LOAD_OK;
}
