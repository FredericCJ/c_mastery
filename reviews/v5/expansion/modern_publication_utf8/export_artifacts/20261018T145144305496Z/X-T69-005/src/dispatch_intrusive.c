#include "dispatch_intrusive.h"
void ds_dual_init(DSDualList *p,unsigned k){(void)p;(void)k;}
void ds_dual_item_init(DSDualItem *p,int v){(void)p;(void)v;}
int ds_dual_push(DSDualList *p,DSDualItem *i){(void)p;(void)i;return DS_FAILED;}
int ds_dual_remove(DSDualList *p,DSDualItem *i){(void)p;(void)i;return DS_FAILED;}
int ds_dual_fold(const DSDualList *p,DSCombine f,int v,int *o){(void)p;(void)f;(void)v;(void)o;return DS_FAILED;}
void ds_index_init(DSIndexOwner *p,DSIndexSlot s[4]){(void)p;(void)s;}
int ds_index_new(DSIndexOwner *p,int v,DSIndexHandle *o){(void)p;(void)v;(void)o;return DS_FAILED;}
int ds_index_get(const DSIndexOwner *p,DSIndexHandle h,int *o){(void)p;(void)h;(void)o;return DS_FAILED;}
int ds_index_push(DSIndexOwner *p,unsigned k,DSIndexHandle h){(void)p;(void)k;(void)h;return DS_FAILED;}
int ds_index_remove(DSIndexOwner *p,unsigned k,DSIndexHandle h){(void)p;(void)k;(void)h;return DS_FAILED;}
int ds_index_release(DSIndexOwner *p,DSIndexHandle h){(void)p;(void)h;return DS_FAILED;}
int ds_index_relocate(DSIndexOwner *p,DSIndexSlot s[4]){(void)p;(void)s;return DS_FAILED;}
int ds_index_fold(const DSIndexOwner *p,unsigned k,DSCombine f,int v,int *o){(void)p;(void)k;(void)f;(void)v;(void)o;return DS_FAILED;}
