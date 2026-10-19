#include "dispatch_modules.h"
int client(DSUserMap *m){return ds_user_put(m,(DSUserKey){0},(DSMetricValue){1});}
