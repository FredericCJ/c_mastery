#include "abi.h"
#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    void *h;
    const struct spike_api *api;
    const char *error;
    int n;
    if (argc != 2)
        return 2;
    h = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (!h)
    {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }
    dlerror();
    api = dlsym(h, "dispatch_spike");
    error = dlerror();
    if (error || !api)
    {
        fprintf(stderr, "symbol failure\n");
        dlclose(h);
        return 1;
    }
    if (api->abi != 1 || api->size != sizeof(*api) || !api->apply)
    {
        dlclose(h);
        return 1;
    }
    n = api->apply(41);
    if (n != 42)
    {
        dlclose(h);
        return 1;
    }
    api = NULL;
    if (dlclose(h))
        return 1;
    puts("M2 dispatch spike: object-symbol ABI, callback result42, quiescent "
         "close");
    return 0;
}
