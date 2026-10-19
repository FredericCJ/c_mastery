#include "dispatch_modules.h"
int client(DSModule *m,DSUserMap *u,DSMetricMap *v){int out;DSUserValue a={1};DSMetricValue b={2};return ds_module_twice(m,&out)+ds_user_put(u,(DSUserKey){0},a)+ds_metric_put(v,(DSMetricKey){0},b);}
