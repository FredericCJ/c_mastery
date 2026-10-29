#include "jump_scope.h"
#include <limits.h>
#ifndef EJD_MUTATION
#define EJD_MUTATION 0
#endif
struct er_state;
struct er_resource {struct er_state *state;int32_t *cell;unsigned id;};
struct er_state {
 struct v4_allocator allocator;struct ejd_plan plan;unsigned *published,start;
 er_frame outer,inner;struct ow_scope scope;struct er_resource resource[5];struct ejd_report report;
};
static void dispose(void *context){
 struct er_resource *r=context;struct er_state *s=r->state;int32_t *cell=r->cell;
 if(!cell)return;
 r->cell=NULL;s->report.held&=~(1u<<r->id);
 if(s->report.trace_count<5){unsigned n=s->report.trace_count++;s->report.trace[n]=r->id;s->report.remaining[n]=s->report.held;}
 s->allocator.release(s->allocator.context,cell);
}
static int acquire(struct er_state *s,unsigned id){
 struct er_resource *r=&s->resource[id-1];r->state=s;r->id=id;r->cell=s->allocator.take(s->allocator.context,id);
 if(!r->cell)return 0;
 *r->cell=(int32_t)id;s->report.held|=1u<<id;return 1;
}
static void finish_scope(void *context){
 struct er_state *s=context;++s->report.scope_finishes;(void)ow_finish(&s->scope,1);
}
static er_result inner_body(er_frame *frame,void *context){
 struct er_state *s=context;s->report.inner_entered=1;s->report.outer_live=s->resource[0].cell!=NULL;
 if(EJD_MUTATION!=1&&er_defer(frame,finish_scope,s)!=ER_OK)return er_error(ER_CONTRACT,0);
 for(unsigned i=0;i<3;++i){
  if(s->plan.cut==2*i){s->report.stage=2*i;(void)er_raise(frame,s->plan.code);return er_error(ER_INVALID,0);}
  unsigned id=i+2;
  if(!acquire(s,id)){s->report.stage=8+i;(void)er_raise(frame,ER_IO);return er_error(ER_INVALID,0);}
  ++*s->published;
  if(s->plan.cut==2*i+1){
   s->report.stage=2*i+1;
   if(EJD_MUTATION!=5)dispose(&s->resource[id-1]);
   (void)er_raise(frame,s->plan.code);return er_error(ER_INVALID,0);
  }
  if(ow_defer(&s->scope,dispose,&s->resource[id-1],OW_ALWAYS)!=OW_OK){dispose(&s->resource[id-1]);s->report.stage=11;(void)er_raise(frame,ER_RANGE);return er_error(ER_INVALID,0);}
 }
 if(s->plan.cut==6){s->report.stage=6;(void)er_raise(frame,s->plan.code);return er_error(ER_INVALID,0);}
 s->report.stage=7;return er_ok(37);
}
static er_result outer_body(er_frame *frame,void *context){
 struct er_state *s=context;
 if(!acquire(s,1)){s->report.stage=20;return er_error(ER_IO,20);}
 if(er_defer(frame,dispose,&s->resource[0])!=ER_OK){dispose(&s->resource[0]);return er_error(ER_CONTRACT,20);}
 if(EJD_MUTATION==4)dispose(&s->resource[0]);
 s->report.inner=er_run(&s->inner,inner_body,s);
 if(EJD_MUTATION==2&&s->report.inner.tag!=ER_OK)*s->published=s->start;
 if(!acquire(s,5))return er_error(ER_IO,21);
 if(er_defer(frame,dispose,&s->resource[4])!=ER_OK){dispose(&s->resource[4]);return er_error(ER_CONTRACT,21);}
 if(s->report.inner.tag!=ER_OK)return er_error(EJD_MUTATION==3?s->report.inner.tag:ER_CONTRACT,EJD_MUTATION==6?0:s->report.stage);
 return er_ok(99);
}
int ejd_run(struct v4_allocator allocator,struct ejd_plan plan,unsigned *published,struct ejd_report *out){
 if(!published||!out||!allocator.take||!allocator.release||*published>UINT_MAX-3||plan.cut>7||(plan.cut==7?plan.code!=ER_OK:plan.code<=ER_OK||plan.code>ER_NOT_IMPLEMENTED))return EJD_INVALID;
 struct er_state s={0};s.allocator=allocator;s.plan=plan;s.published=published;s.start=*published;s.report.inner=er_error(ER_ABSENT,0);
 s.report.result=er_run(&s.outer,outer_body,&s);s.report.effects_after=*published;s.report.scope_empty=s.scope.count==0;s.report.frames_inactive=!s.outer.active&&!s.outer.cleaning&&!s.inner.active&&!s.inner.cleaning;*out=s.report;return EJD_OK;
}
