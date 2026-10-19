#include "views.h"
#include <stdio.h>
#include <stdint.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FV_FAIL %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
static FvRecord implicit;
/* File-scope compound literal is a modifiable static-duration object. */
static int *file_values=(int[]){[1]=7,[3]=9};
static int values(const int *p){return p[0]==0&&p[1]==7&&p[2]==0&&p[3]==9;}
int main(void) {
 CHECK(implicit.values[0]==0&&implicit.values[3]==0&&implicit.link==NULL);
 FvRecord initialized={.values={[1]=7,[3]=9}};
 CHECK(values(initialized.values)&&initialized.link==NULL);
 FvRecord assigned;assigned=initialized; /* No read precedes the assignment. */
 CHECK(values(assigned.values)&&assigned.link==NULL);
 CHECK(values((int[]){[1]=7,[3]=9}));
 CHECK(values(file_values));int *alias=file_values;alias[2]=11;
 CHECK(file_values[2]==11);file_values[2]=0;
 int target=42;initialized.link=&target;
 CHECK(fv_reset(&initialized)==FV_OK);
 CHECK(initialized.values[0]==0&&initialized.values[3]==0&&initialized.link==NULL);
 CHECK(fv_reset(NULL)==FV_INVALID);
 puts("FV_INIT_PASS zero_values null_pointer designated assignment compound_durations");
 FvRecord caller;FvView returned=fv_initialize(&caller);
 CHECK(returned.data==caller.values&&returned.length==4&&values(returned.data)&&caller.link==NULL);
 CHECK(fv_initialize(NULL).length==0);
 FvView live=fv_view(&assigned);CHECK(live.data==assigned.values&&live.length==4);
 int snapshot[5]={31,32,33,34,35};
 CHECK(fv_copy((FvDestination){snapshot,3},live)==FV_FULL);
 CHECK(snapshot[0]==31&&snapshot[1]==32&&snapshot[2]==33&&snapshot[3]==34&&snapshot[4]==35);
 CHECK(fv_copy((FvDestination){snapshot,5},live)==FV_OK&&values(snapshot)&&snapshot[4]==35);
 CHECK(fv_update(&assigned,1,8)==FV_OK&&live.data[1]==8&&snapshot[1]==7);
 CHECK(fv_update(&assigned,4,9)==FV_RANGE&&live.data[1]==8);
 CHECK(fv_update(&assigned,SIZE_MAX,9)==FV_RANGE);
 CHECK(fv_update(NULL,0,1)==FV_INVALID);
 CHECK(fv_view(NULL).length==0&&fv_view(NULL).data==NULL);
 CHECK(fv_copy((FvDestination){NULL,0},(FvView){NULL,0})==FV_OK);
 CHECK(fv_copy((FvDestination){NULL,1},(FvView){NULL,0})==FV_INVALID);
 CHECK(fv_copy((FvDestination){snapshot,4},(FvView){NULL,1})==FV_INVALID);
 CHECK(fv_reset(&assigned)==FV_OK&&live.data[0]==0&&live.data[1]==0&&live.data[3]==0&&snapshot[1]==7);
 FvOwner *owner=fv_owner();CHECK(owner!=NULL);
 FvView opaque=fv_owner_view(owner);int *mutable=fv_owner_mutable(owner);
 CHECK(opaque.length==4&&opaque.data==mutable&&mutable!=NULL);
 mutable[0]=71;CHECK(opaque.data[0]==71);
 CHECK(fv_owner_view(NULL).length==0&&fv_owner_mutable(NULL)==NULL);
 puts("FV_VIEW_PASS caller_owned live_alias detached_snapshot empty capacity_atomic");
 int overlap[]={1,2,3,4,5,6};
 CHECK(fv_move(overlap,6,1,0,5)==FV_OK);
 CHECK(overlap[0]==1&&overlap[1]==1&&overlap[2]==2&&overlap[3]==3&&overlap[4]==4&&overlap[5]==5);
 CHECK(fv_move(overlap,6,0,1,5)==FV_OK);
 CHECK(overlap[0]==1&&overlap[1]==2&&overlap[2]==3&&overlap[3]==4&&overlap[4]==5);
 CHECK(fv_move(overlap,6,6,6,0)==FV_OK);
 CHECK(fv_move(NULL,0,0,0,0)==FV_OK);
 CHECK(fv_move(NULL,1,0,0,0)==FV_INVALID);
 CHECK(fv_move(overlap,6,SIZE_MAX,0,1)==FV_RANGE);
 CHECK(fv_move(overlap,6,0,SIZE_MAX,1)==FV_RANGE);
 CHECK(fv_move(overlap,6,0,0,SIZE_MAX)==FV_RANGE);
 volatile unsigned device=3;CHECK(fv_device_twice(&device)==0&&device==3);
 puts("FV_QUALIFIER_PASS restrict_disjoint overlap_direction volatile_RAM_only");
 puts("FV_PASS qualified_views");return 0;
}
