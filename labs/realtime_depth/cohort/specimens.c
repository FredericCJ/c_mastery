#include "specimens.h"
void su_flawed_init(struct su_flawed *s,unsigned q,unsigned p){*s=(struct su_flawed){0};s->q=s->remaining=q;s->p=p;}
void su_flawed_update(struct su_flawed *s,unsigned t) {
 for(unsigned i=0;i<s->count;) {
  if(s->at[i]>t){++i;continue;}
  s->remaining+=s->amount[i];if(s->remaining>s->q)s->remaining=s->q;
  for(unsigned j=i+1;j<s->count;++j){s->at[j-1]=s->at[j];s->amount[j-1]=s->amount[j];}
  --s->count;
 }
}
static void finish(struct su_flawed *s) {
 if(s->usage && s->count<16){s->at[s->count]=s->begin+s->p;s->amount[s->count++]=s->usage;}
 s->usage=0;s->active=0;
}
void su_flawed_active(struct su_flawed *s,unsigned t,unsigned active) {
 su_flawed_update(s,t);
 if(active && !s->active && s->remaining){s->active=1;s->begin=t;}
 else if(!active && s->active)finish(s);
}
void su_flawed_run(struct su_flawed *s,unsigned t,unsigned n) {
 su_flawed_active(s,t,1);s->usage+=n;
 s->remaining=n>s->remaining?0:s->remaining-n;
 if(!s->remaining)finish(s);
}
