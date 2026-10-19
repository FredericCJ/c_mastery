#include "operation.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SC_CHECK: operation_follow:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct { unsigned drops, starts, callbacks, frees; } Counts;
typedef struct { Counts *counts; int number; } Payload;
typedef struct Owned Owned;
struct Owned { MfoOperation op; MfoValue value; Owned *next; Counts *counts; };
static void payload_drop(void *p) {
  Payload *value=p; value->counts->drops++;free(value);
}
static void inline_backend(MfoOperation *op,void *arg) {
  Owned *owner=arg;
  owner->counts->starts++;
  /* The argument descriptor is a separate automatic object, not inside op. */
  MfoValue value=owner->value;owner->value=(MfoValue){0};
  CHECK(mfo_finish(op,MSG_OK,&value)==MSG_OK);
  CHECK(!value.data && !value.drop);
  /* Do not touch owner here: it has been freed by the receiver. */
}
static void chained(void *arg,MsgStatus status,MfoValue value) {
  Owned *owner=arg;CHECK(status==MSG_OK && value.data && value.drop);
  Counts *counts=owner->counts;Owned *next=owner->next;
  CHECK(((Payload *)value.data)->number==42);
  counts->callbacks++;
  if(next) next->value=value;
  else mfo_drop(&value);
  counts->frees++;free(owner);
  /* Successor observes ownership already transferred, predecessor no longer live. */
  if(next) CHECK(mfo_start(&next->op)==MSG_OK);
}
typedef struct { unsigned callbacks, starts; MsgStatus status; MfoValue value; } Later;
static void deferred_backend(MfoOperation *op,void *arg) {(void)op;((Later *)arg)->starts++;}
static void deferred_receiver(void *arg,MsgStatus status,MfoValue value) {
  Later *later=arg;later->callbacks++;later->status=status;later->value=value;
}
int main(void) {
  Counts counts={0};Owned *first=calloc(1,sizeof(*first)),*second=calloc(1,sizeof(*second));
  Payload *payload=malloc(sizeof(*payload));CHECK(first&&second&&payload);
  *payload=(Payload){&counts,42};first->value=(MfoValue){payload,payload_drop};
  first->next=second;first->counts=second->counts=&counts;
  CHECK(mfo_init(&first->op,inline_backend,first,chained,first)==MSG_OK);
  CHECK(mfo_init(&second->op,inline_backend,second,chained,second)==MSG_OK);
  CHECK(mfo_start(&first->op)==MSG_OK);
  CHECK(counts.starts==2&&counts.callbacks==2&&counts.frees==2&&counts.drops==1);
  MfoOperation op;Later later={0};MfoValue empty={0};
  CHECK(mfo_init(&op,deferred_backend,&later,deferred_receiver,&later)==MSG_OK);
  CHECK(mfo_finish(&op,MSG_OK,&empty)==MSG_BUSY);
  CHECK(mfo_start(&op)==MSG_OK && later.starts==1);
  CHECK(mfo_start(&op)==MSG_INVALID && later.starts==1);
  CHECK(mfo_stop(&op)==MSG_OK && later.callbacks==0);
  payload=malloc(sizeof(*payload));CHECK(payload);*payload=(Payload){&counts,42};
  MfoValue owned={payload,payload_drop};
  CHECK(mfo_finish(&op,MSG_EMPTY,&owned)==MSG_INVALID && owned.data==payload && !later.callbacks);
  CHECK(mfo_finish(&op,MSG_CANCELLED,&owned)==MSG_OK && !owned.data);
  CHECK(later.callbacks==1&&later.status==MSG_CANCELLED&&later.value.data==payload);
  mfo_drop(&later.value);CHECK(counts.drops==2);
  CHECK(mfo_finish(&op,MSG_OK,&empty)==MSG_BUSY&&later.callbacks==1);
  CHECK(mfo_stop(&op)==MSG_INVALID);
  puts("inline free-on-completion and owning successor passed");return 0;
}
