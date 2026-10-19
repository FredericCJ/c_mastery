#include <stdio.h>
#ifndef DX_HIDDEN_BAD
#define DX_HIDDEN_BAD 0
#endif
/* Assessor-only changed component: a callback borrow is not an owned snapshot.
 * Integer generations make the diagnosis defined before any native stale read. */
typedef struct {unsigned generation,live,value;} Owner;
typedef struct {unsigned generation;} View;
static int read_view(const Owner *o,View v,unsigned *out){if(!o->live||(!DX_HIDDEN_BAD&&o->generation!=v.generation))return -1;*out=o->value;return 0;}
int main(void){Owner o={3,1,71};View v={3};unsigned out=99;if(read_view(&o,v,&out)||out!=71)return 1;/* callback ends, owner replaced */o=(Owner){4,1,82};out=99;int result=read_view(&o,v,&out);if(DX_HIDDEN_BAD){if(result!=0||out!=82)return 2;puts("DX_HIDDEN:stale-view-accepted-new-owner82:first-invalid-generation-check");}else{if(result!=-1||out!=99)return 2;puts("DX_HIDDEN:expired-borrow-rejected");}View repaired={4};if(read_view(&o,repaired,&out)||out!=82)return 3;puts("DX_HIDDEN:new-owner-valid");return 0;}
