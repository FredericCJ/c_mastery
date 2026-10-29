#include "dispatch_modules.h"

int client(struct ds_module *m)
{
    return m->live;
}
