#include "dispatch_modules.h"
int client(struct ds_user_map *m){return ds_user_put(m,(struct ds_user_key){0},(struct ds_metric_value){1});}
