#include "cohort.h"
#ifndef SU_MUTANT
#define SU_MUTANT 0
#endif
unsigned su_available(const struct su_state *s) {
  unsigned n=0;
  if(s)for(unsigned i=0;i<s->q && i<SU_UNITS;++i)n+=s->unit[i].due<=s->last;
  return n;
}
int su_invariant(const struct su_state *s) {
  if(!s || !s->q || s->q>SU_UNITS || !s->period || s->q>s->period ||
      s->overrun>s->q || s->active>1 || s->borrowed>s->served)return 0;
  for(unsigned i=0;i<s->q;++i) {
    const struct su_unit *u=&s->unit[i];
    if(u->active>1 || (u->active && (!s->active || u->due>s->last ||
        u->anchor<u->due || u->anchor>s->last)))return 0;
    if(s->active && u->due<=s->last && !u->active)return 0;
  }
  return 1;
}
enum cmx_result su_init(struct su_state *s,unsigned q,uint32_t period,uint32_t start,unsigned overrun) {
  if(!s || !q || q>SU_UNITS || !period || q>period || overrun>q)return CMX_INVALID;
  struct su_state x={0};x.q=q;x.period=period;x.last=start;x.overrun=overrun;
  for(unsigned i=0;i<q;++i)x.unit[i].due=start;
  *s=x;return CMX_OK;
}
static enum cmx_result advance(struct su_state *s,uint32_t now) {
  if(now<s->last)return CMX_INVALID;
  for(unsigned i=0;i<s->q;++i) {
    struct su_unit *u=&s->unit[i];
    if(s->active && !u->active && u->due<=now) {
      u->active=1;
      u->anchor=u->due<s->last?s->last:u->due;
      if(SU_MUTANT==1) {
        /* Improperly merge a newly ready cohort with an older activation. */
        for(unsigned j=0;j<s->q;++j)
          if(s->unit[j].active && s->unit[j].anchor<u->anchor)u->anchor=s->unit[j].anchor;
      }
    }
  }
  s->last=now;return CMX_OK;
}
enum cmx_result su_active(struct su_state *s,uint32_t now,unsigned active) {
  if(!su_invariant(s))return CMX_BROKEN;
  if(active>1)return CMX_INVALID;
  struct su_state x=*s;enum cmx_result r=advance(&x,now);if(r!=CMX_OK)return r;
  if(active!=x.active) {
    x.active=active;
    for(unsigned i=0;i<x.q;++i)if(x.unit[i].due<=now) {
      x.unit[i].active=active;
      if(SU_MUTANT!=3 || !active){x.unit[i].due=now;x.unit[i].anchor=active?now:0;}
    }
  }
  if(SU_MUTANT!=1 && !su_invariant(&x))return CMX_BROKEN;
  *s=x;return CMX_OK;
}
enum cmx_result su_run(struct su_state *s,uint32_t now,unsigned n) {
  if(!su_invariant(s))return CMX_BROKEN;
  struct su_state x=*s;enum cmx_result r=advance(&x,now);if(r!=CMX_OK)return r;
  if(SU_MUTANT==4)*s=x;
  if(!n || !x.active)return CMX_INVALID;
  unsigned available=su_available(&x);
  if(!available || n>available+x.overrun)return CMX_FULL;
  if(n>UINT32_MAX-now || n>UINT64_MAX-x.served)return CMX_FULL;
  for(unsigned k=0;k<n;++k) {
    uint32_t tick=now+k;
    r=advance(&x,tick);if(r!=CMX_OK)return r;
    unsigned chosen=SU_UNITS;
    for(unsigned i=0;i<x.q;++i)if(x.unit[i].active &&
       (chosen==SU_UNITS || x.unit[i].anchor<x.unit[chosen].anchor))chosen=i;
    if(chosen==SU_UNITS) {
      chosen=0;
      for(unsigned i=1;i<x.q;++i)if(x.unit[i].due<x.unit[chosen].due)chosen=i;
      struct su_unit *u=&x.unit[chosen];
      if(x.period>UINT32_MAX-u->due)return CMX_FULL;
      if(SU_MUTANT!=2)u->due+=x.period;
      ++x.borrowed;
    } else {
      struct su_unit *u=&x.unit[chosen];
      if(x.period>UINT32_MAX-u->anchor)return CMX_FULL;
      uint32_t due=u->anchor+x.period;
      /* Late busy intervals may pass the intended time. Delaying to the
       * observation boundary is explicit, never moving eligibility earlier. */
      u->due=due<tick+1?tick+1:due;
      if(SU_MUTANT==5)u->due=tick+1;
      u->active=0;u->anchor=0;
    }
    ++x.served;
  }
  r=advance(&x,now+n);if(r!=CMX_OK)return r;
  if(SU_MUTANT!=1 && !su_invariant(&x))return CMX_BROKEN;
  *s=x;return CMX_OK;
}
