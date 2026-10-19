#include "dispatch_plugin.h"
#include <stdint.h>
#ifndef DS_PLUGIN_CASE
#define DS_PLUGIN_CASE 0
#endif
void ds_constructor_note(void);
__attribute__((constructor)) static void note_load(void){ds_constructor_note();}
static int apply(int state,int amount,int *out){if(state<0||state>1000||amount<0||amount>100)return DS_RANGE;*out=state+amount;return DS_OK;}
static int migrate(unsigned previous,int state,int *out){if(previous>2||state<0||state>500)return DS_RANGE;*out=state;return DS_OK;}
#if DS_PLUGIN_CASE==7
const DSPlugin deliberately_wrong_symbol={1,sizeof(DSPlugin),1,apply,migrate};
#elif DS_PLUGIN_CASE==8
const DSPlugin dispatch_plugin={1,sizeof(DSPlugin)-1,1,apply,migrate};
#elif DS_PLUGIN_CASE==9
const DSPlugin dispatch_plugin={1,sizeof(DSPlugin)+1,1,apply,migrate};
#else
const DSPlugin dispatch_plugin={1,sizeof(DSPlugin),1,apply,migrate};
#endif
#if DS_PLUGIN_CASE==0
const DSPluginBase dispatch_plugin_extended={2,0,sizeof(DSPluginBase),0,0,apply};
#elif DS_PLUGIN_CASE==1 || DS_PLUGIN_CASE==2 || DS_PLUGIN_CASE==5 || DS_PLUGIN_CASE==6
static int describe(unsigned char *out,size_t cap,size_t *n){if(cap<2)return DS_FULL;out[0]='v';out[1]='2';*n=2;return DS_OK;}
#if DS_PLUGIN_CASE==5
struct Future { DSPluginOptional prefix;uint64_t tail; };
const struct Future dispatch_plugin_extended={{{2,9,sizeof(struct Future),0,3,apply},describe},123};
#else
const DSPluginOptional dispatch_plugin_extended={{2,1,DS_PLUGIN_CASE==6?sizeof(DSPluginBase):sizeof(DSPluginOptional),DS_PLUGIN_CASE==2?1:0,1,apply},describe};
#endif
#elif DS_PLUGIN_CASE==3
const DSPluginBase dispatch_plugin_extended={2,1,sizeof(DSPluginBase),2,2,apply};
#elif DS_PLUGIN_CASE==4
const DSPluginBase dispatch_plugin_extended={2,0,sizeof(DSPluginBase)-1,0,0,apply};
#endif
