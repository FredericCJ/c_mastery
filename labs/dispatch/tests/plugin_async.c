#include "dispatch.h"
#ifndef PLUGIN_SCHEMA
#define PLUGIN_SCHEMA 1
#endif
void ds_async_pause(void);
static int apply(int state,int amount,int *out){
 if(state<0 || state>1000 || amount<0 || amount>100)return DS_RANGE;
 if(amount==99)ds_async_pause();
 *out=state+amount;return DS_OK;
}
static int migrate(unsigned previous,int value,int *out){
 if(previous>2 || value < -1000 || value>1000)return DS_RANGE;
 if(PLUGIN_SCHEMA==2 && (value<0 || value>500))return DS_RANGE;
 *out=PLUGIN_SCHEMA==2?value*2:value;return DS_OK;
}
const struct ds_plugin dispatch_plugin={1,sizeof(struct ds_plugin),PLUGIN_SCHEMA,apply,migrate};
