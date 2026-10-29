#include "dispatch_modules.h"

int client(struct ds_module *m, struct ds_user_map *u, struct ds_metric_map *v)
{
    int out;
    struct ds_user_value a = {1};
    struct ds_metric_value b = {2};
    return ds_module_twice(m, &out) +
           ds_user_put(u, (struct ds_user_key){0}, a) +
           ds_metric_put(v, (struct ds_metric_key){0}, b);
}
