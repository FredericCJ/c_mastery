#include "ownership.h"
static AcStatus event(AcLedger *l,unsigned id,unsigned request,unsigned acquire) {
  if(l->count>=AC_EVENTS) return AC_FULL;
  l->events[l->count++]=(AcEvent){id,request,acquire};return AC_OK;
}
AcStatus ac_resource_acquire(AcLedger *l,unsigned id,unsigned request,int fail,int lost) {
  if(!l||id>=AC_RESOURCES||!request) return AC_INVALID;
  if(l->held[id]) return l->request[id]==request?AC_OK:AC_FULL;
  if(fail) return AC_BACKEND;
  AcStatus r=event(l,id,request,1);if(r!=AC_OK) return r;
  l->held[id]=1;l->request[id]=request;++l->acquisitions[id];
  return lost?AC_BACKEND:AC_OK; /* failure does not imply acquisition did not occur */
}
AcStatus ac_resource_query(const AcLedger *l,unsigned id,unsigned request,int *out) {
  if(!l||id>=AC_RESOURCES||!out||!request) return AC_INVALID;
  *out=l->held[id]&&l->request[id]==request;return AC_OK;
}
AcStatus ac_resource_close(AcLedger *l,unsigned id,int fail) {
  if(!l||id>=AC_RESOURCES||!l->held[id]) return AC_INVALID;
  if(fail&&!l->consume_on_error[id]) {
#ifndef AC_MUTANT_CLOSE
    return AC_BACKEND;
#endif
  }
  AcStatus r=event(l,id,l->request[id],0);if(r!=AC_OK) return r;
#ifndef AC_MUTANT_RELEASE
  l->held[id]=0;++l->releases[id];
#endif
  return fail?AC_BACKEND:AC_OK;
}
AcCloseReport ac_close_all(AcLedger *l,unsigned failed) {
  AcCloseReport result={AC_OK,0};
  if(!l||failed>15) {result.first=AC_INVALID;return result;}
  if(l->count>AC_EVENTS-4) {result.first=AC_FULL;return result;}
  for(unsigned i=4;i>0;--i) if(l->held[i-1]) {
    AcStatus r=ac_resource_close(l,i-1,!!(failed&(1u<<(i-1))));
    if(result.first==AC_OK&&r!=AC_OK) result.first=r;
  }
  for(unsigned i=0;i<4;++i) if(l->held[i]) result.retained|=1u<<i;
  return result;
}
AcStatus ac_session_admit(const AcSession *s) {
  if(!s||(unsigned)s->state>AC_DESTROYED||s->mask>3||s->epoch>31) return AC_INVALID;
  switch(s->state) {
  case AC_ACTIVE:return s->mask==(s->optional?1u:3u)?AC_OK:AC_INVALID;
  case AC_PARTIAL:return s->mask==1?AC_OK:AC_INVALID;
  default:return !s->mask?AC_OK:AC_INVALID;
  }
}
static AcStatus release_prefix(AcSession *s,AcLedger *l) {
  for(unsigned i=2;i>0;--i) if(s->mask&(1u<<(i-1))) {
    AcStatus r=ac_resource_close(l,i-1,0);if(r!=AC_OK) return r;
    s->mask&=~(1u<<(i-1));
  }
  return AC_OK;
}
AcStatus ac_session_init(AcSession *s,AcLedger *l,unsigned fail,int optional) {
  if(!l||ac_session_admit(s)!=AC_OK||s->state!=AC_NEW||fail>4||l->held[0]||l->held[1]) return AC_INVALID;
  /* Reserve every possible acquire/release event before effects. */
  if(l->count>AC_EVENTS-4||s->epoch==31) return AC_FULL;
  s->optional=!!optional;
  AcStatus r=ac_resource_acquire(l,0,s->epoch+1,fail==1,0);
  if(r!=AC_OK) {s->state=AC_FAILED;return r;}
  s->mask=1;s->state=AC_PARTIAL;
  if(fail==2||fail==3) goto failed;
  if(!s->optional) {
    r=ac_resource_acquire(l,1,s->epoch+1,0,0);
    if(r!=AC_OK) goto failed;
    s->mask=3;
  }
  if(fail==4) goto failed;
  s->state=AC_ACTIVE;return AC_OK;
failed:
  r=release_prefix(s,l);s->state=AC_FAILED;
  return r==AC_OK?AC_BACKEND:r;
}
AcStatus ac_session_stop(AcSession *s,AcLedger *l) {
  if(!l||ac_session_admit(s)!=AC_OK||s->state==AC_NEW||s->state==AC_DESTROYED) return AC_INVALID;
  if(s->state==AC_STOPPED) return AC_OK;
  if(s->epoch==31||l->count>AC_EVENTS-2) return AC_FULL;
  AcStatus r=release_prefix(s,l);if(r!=AC_OK) return r;
  ++s->epoch;s->state=AC_STOPPED;return AC_OK;
}
AcStatus ac_session_destroy(AcSession *s) {
  if(ac_session_admit(s)!=AC_OK||s->state!=AC_STOPPED) return AC_INVALID;
  s->state=AC_DESTROYED;return AC_OK;
}
AcStatus ac_session_restart(AcSession *s,AcLedger *l,int optional) {
  if(!l||ac_session_admit(s)!=AC_OK||s->state!=AC_STOPPED||l->held[0]||l->held[1]) return AC_INVALID;
  if(l->count>AC_EVENTS-4||s->epoch==31) return AC_FULL;
  s->state=AC_NEW;return ac_session_init(s,l,0,optional);
}
AcStatus ac_table_alloc(AcTable *t,int v,AcToken *out) {
  if(!t||!out) return AC_INVALID;
  for(unsigned i=0;i<2;++i) if(!t->slots[i].live&&t->slots[i].generation<3) {
    AcIdentity *s=t->slots+i;++s->generation;s->live=1;s->value=v;
    *out=(AcToken){i,s->generation};return AC_OK;
  }
  return AC_FULL;
}
AcStatus ac_table_get(const AcTable *t,AcToken h,int *out) {
  if(!t||!out||h.slot>=2||!h.generation||h.generation>3) return AC_INVALID;
  const AcIdentity *s=t->slots+h.slot;
#ifndef AC_MUTANT_GENERATION
  if(!s->live||s->generation!=h.generation) return AC_MISSING;
#else
  if(!s->live) return AC_MISSING;
#endif
  *out=s->value;return AC_OK;
}
AcStatus ac_table_free(AcTable *t,AcToken h) {
  int value;AcStatus r=ac_table_get(t,h,&value);if(r!=AC_OK) return r;
  t->slots[h.slot].live=0;return AC_OK;
}
AcStatus ac_shared_retain(AcShared *s) {
  if(!s||!s->alive||!s->strong) return AC_MISSING;
  if(s->strong>=3) return AC_FULL;
  ++s->strong;return AC_OK;
}
AcStatus ac_shared_link(AcShared *parent,AcShared *child) {
  if(!parent||!parent->alive||!child) return AC_INVALID;
  for(unsigned i=0;i<2;++i) if(!parent->children[i]) {
    AcStatus r=ac_shared_retain(child);if(r!=AC_OK) return r;
    parent->children[i]=child;return AC_OK;
  }
  return AC_FULL;
}
AcStatus ac_shared_drop(AcShared *s) {
  if(!s||!s->alive||!s->strong) return AC_MISSING;
  if(--s->strong) return AC_OK;
  s->alive=0;++s->releases;
  for(unsigned i=0;i<2;++i) if(s->children[i]) {
    AcShared *child=s->children[i];s->children[i]=NULL;
    AcStatus r=ac_shared_drop(child);if(r!=AC_OK) return r;
  }
  return AC_OK;
}
AcStatus ac_lease_read(const AcLeaseOwner *o,unsigned token,unsigned now,int *out) {
  if(!o||!out||o->issued>255||token>255||now>255) return AC_INVALID;
  unsigned elapsed=(now-token)&255u;
  if(!o->live||o->issued!=token||elapsed>3) return AC_MISSING;
  *out=o->value;return AC_OK;
}
AcStatus ac_lease_reuse(AcLeaseOwner *o,unsigned now,int value) {
  if(!o||o->issued>255||now>255) return AC_INVALID;
  unsigned age=(now-o->issued)&255u;
  if(o->live||age<=3||age>127) return AC_FULL;
  o->live=1;o->issued=now;o->value=value;return AC_OK;
}
AcStatus ac_epoch_update(AcEpochOwner *o,size_t i,uint8_t value) {
  if(!o||!o->live||o->bytes.length>32||i>=o->bytes.length) return AC_INVALID;
  if(o->values==UINT32_MAX) return AC_FULL;
  o->bytes.data[i]=value;++o->values;return AC_OK;
}
AcStatus ac_epoch_snapshot(const AcEpochOwner *o,AcSnapshot *out) {
  if(!o||!o->live||!out) return AC_INVALID;
  AcSnapshot s;FStatus r=f_view(&o->bytes,0,o->bytes.length,&s.bytes);
  if(r!=F_OK) return AC_INVALID;
  s.values=o->values;*out=s;return AC_OK;
}
AcStatus ac_epoch_read(const AcEpochOwner *o,AcSnapshot s,size_t i,uint8_t *out) {
  if(!o||!out) return AC_INVALID;
  if(!o->live||s.bytes.owner!=&o->bytes||s.values!=o->values) return AC_MISSING;
  FStatus r=f_view_read(s.bytes,i,out);
  return r==F_OK?AC_OK:r==F_STALE?AC_MISSING:AC_INVALID;
}
AcStatus ac_epoch_reset(AcEpochOwner *o) {
  if(!o||!o->live||o->bytes.length>32) return AC_INVALID;
  if(o->bytes.generation==UINT32_MAX) return AC_FULL;
  o->bytes.length=0;++o->bytes.generation;return AC_OK;
}
