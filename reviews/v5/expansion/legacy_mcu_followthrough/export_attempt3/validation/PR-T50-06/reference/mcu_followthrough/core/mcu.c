#include "mcu.h"
#include <limits.h>

MfHandle mf_handle(const Mf *m, unsigned slot) {
  return ((uint64_t)m->kernel.task[slot].generation << 5) | (slot + 1u);
}
int mf_lookup(const Mf *m, MfHandle handle) {
  unsigned low = (unsigned)(handle & 31u);
  if (!low || low > CMX_TASKS || (handle >> 37)) return -1;
  unsigned slot = low - 1;
  if (m->kernel.task[slot].state == CMX_UNUSED ||
      (handle >> 5) != m->kernel.task[slot].generation) return -1;
  return (int)slot;
}
enum cmx_result mf_add(Mf *m, uint32_t *stack, size_t words, uint32_t entry,
                        uint32_t return_pc, unsigned priority,
                        unsigned detached, unsigned budget, MfHandle *out) {
  uintptr_t low = (uintptr_t)stack;
  if (!out || !stack || words < 128 || words > UINT32_MAX / 4u ||
      (low & 7u) || words > (UINTPTR_MAX - low) / 4u ||
      !(entry & 1u) || !(return_pc & 1u) || detached > 1 || priority > 255)
    return CMX_INVALID;
  size_t bytes = (words * 4u) & ~(size_t)7u;
  if (!budget) return CMX_FULL;
  int candidate = -1;
  for (unsigned i = 0; i < CMX_TASKS; ++i)
    if (m->kernel.task[i].state == CMX_UNUSED) { candidate = (int)i; break; }
  if (candidate < 0) return CMX_FULL;
  cmx_task old = m->kernel.task[candidate];
  int slot = cmx_task_add(&m->kernel, priority, 0, low, bytes);
  if (slot < 0) return CMX_INVALID;
  if (budget == 1) { m->kernel.task[slot] = old; return CMX_FULL; }
  uint32_t *top = stack + bytes / 4u;
  for (size_t i = 0; i < bytes / 4u; ++i) stack[i] = UINT32_C(0xa5a5a5a5);
  *--top = UINT32_C(0x01000000); *--top = entry; *--top = return_pc;
  for (unsigned i = 0; i < 5; ++i) *--top = 0;
  *--top = UINT32_C(0xfffffffd);
  for (unsigned i = 0; i < 8; ++i) *--top = 0;
  m->sp[slot] = top; m->detached[slot] = detached;
  m->consumed[slot] = m->faulted[slot] = m->fault_valid[slot] = 0;
  *out = mf_handle(m, (unsigned)slot);
  return CMX_OK;
}
enum cmx_result mf_init(Mf *m, uint32_t *idle, size_t words, enum cmx_policy p) {
  if (!m || (p != CMX_RR && p != CMX_FIXED && p != CMX_EDF)) return CMX_INVALID;
  /* Failed initialization leaves the destination unchanged. The admission
   * preflight is shared with mf_add; caller supplies real idle PC at port bind. */
  uintptr_t low=(uintptr_t)idle;
  if (!idle || words < 128 || words > UINT32_MAX/4u || (low&7u) ||
      words > (UINTPTR_MAX-low)/4u) return CMX_INVALID;
  *m=(Mf){0}; cmx_init(&m->kernel,p); m->executing=-1;
  MfHandle h=0; return mf_add(m,idle,words,1,1,255,1,2,&h);
}
static int schedule(Mf *m,unsigned reverse) {
  if (m->fatal) return -1;
  cmx_kernel *k=&m->kernel;
  if (k->current>=0 && k->task[k->current].state==CMX_RUNNING)
    k->task[k->current].state=CMX_READY;
  unsigned start=k->current<0?(reverse?CMX_TASKS-1u:1u):
    ((unsigned)k->current+(reverse?CMX_TASKS-1u:1u))%CMX_TASKS;
  int best=-1;
  for (unsigned n=0;n<CMX_TASKS;++n) {
    unsigned i=(start+(reverse?CMX_TASKS-n:n))%CMX_TASKS;
    if (!i || m->faulted[i] || k->task[i].state!=CMX_READY) continue;
    if (best<0 || (k->policy==CMX_FIXED && k->task[i].priority<k->task[best].priority) ||
        (k->policy==CMX_EDF && k->task[i].job_deadline!=k->task[best].job_deadline &&
         (uint32_t)(k->task[best].job_deadline-k->task[i].job_deadline)<UINT32_C(0x80000000))) best=(int)i;
  }
  if (best<0) best=0;
  k->current=best; k->task[best].state=CMX_RUNNING; return best;
}
int mf_schedule(Mf *m){return schedule(m,0);}
int mf_schedule_reverse(Mf *m){return schedule(m,1);}
enum cmx_result mf_exit(Mf *m,MfHandle handle,int result) {
  int i=mf_lookup(m,handle); if (i<=0) return CMX_INVALID;
  return cmx_exit(&m->kernel,(unsigned)i,result);
}
enum cmx_result mf_join_result(Mf *m,MfHandle caller,MfHandle target,int *out) {
  int a=mf_lookup(m,caller),b=mf_lookup(m,target);
  if (!out || a<=0 || b<=0 || a==b || m->detached[b] || m->consumed[b] ||
      m->kernel.task[b].state!=CMX_EXITED || m->kernel.task[b].joiner!=a ||
      m->kernel.task[a].result!=CMX_OK || m->kernel.task[a].state==CMX_WAITING)
    return CMX_INVALID;
  *out=m->kernel.task[b].exit_code; m->consumed[b]=1; return CMX_OK;
}
enum cmx_result mf_join(Mf *m,MfHandle caller,MfHandle target,cmx_deadline d,int *out) {
  int a=mf_lookup(m,caller),b=mf_lookup(m,target);
  if (!out || a<=0 || b<=0 || a==b || m->detached[b] || m->consumed[b]) return CMX_INVALID;
  enum cmx_result r=cmx_join(&m->kernel,(unsigned)a,(unsigned)b,d);
  if (r==CMX_OK) { *out=m->kernel.task[b].exit_code; m->consumed[b]=1; }
  return r;
}
enum cmx_result mf_reap(Mf *m,MfHandle handle) {
  int i=mf_lookup(m,handle);
  if (i<=0 || i==m->executing || (!m->detached[i] && !m->consumed[i])) return CMX_INVALID;
  enum cmx_result r=cmx_reap(&m->kernel,(unsigned)i);
  if (r==CMX_OK) { m->sp[i]=0; m->detached[i]=m->consumed[i]=m->faulted[i]=0; }
  return r;
}
enum cmx_result mf_fault(Mf *m,MfHandle handle,uint32_t code,uint32_t pc) {
  int i=mf_lookup(m,handle); if (i<=0) return CMX_INVALID;
  if (!m->fault_valid[i]) { m->fault_valid[i]=1; m->fault_code[i]=code; m->fault_pc[i]=pc; }
  if (m->faulted[i]) return CMX_INVALID;
  m->faulted[i]=1;
  cmx_task *t=&m->kernel.task[i];
  if (t->owned_sync) { m->fatal=1; return CMX_NOT_OWNER; }
  for (unsigned n=0;n<m->kernel.mutex_count;++n)
    if (m->kernel.mutex[n]->owner==i) { m->fatal=1; return CMX_NOT_OWNER; }
  if (t->state==CMX_WAITING) {
    if (cmx_cancel(&m->kernel,(unsigned)i)!=CMX_OK || t->state==CMX_WAITING) {
      m->fatal=1; return CMX_NOT_OWNER;
    }
  }
  enum cmx_result r=cmx_exit(&m->kernel,(unsigned)i,-1);
  if (r!=CMX_OK) m->fatal=1;
  return r;
}
int mf_invariant(const Mf *m) {
  if (!cmx_invariant(&m->kernel) || m->kernel.task[0].state==CMX_UNUSED ||
      m->kernel.task[0].state==CMX_EXITED) return 0;
  for (unsigned i=0;i<CMX_TASKS;++i) {
    const cmx_task *t=&m->kernel.task[i];
    if (t->state!=CMX_UNUSED && (!m->sp[i] || (uintptr_t)m->sp[i]<t->stack_low ||
        (uintptr_t)m->sp[i]>=t->stack_high || (t->stack_high&7u))) return 0;
    if (m->faulted[i] && !m->fatal && t->state!=CMX_EXITED) return 0;
  }
  return 1;
}
int mf_due32(uint32_t now,uint32_t at) { return (uint32_t)(now-at)<UINT32_C(0x80000000); }
int mf_due64(uint64_t now,uint64_t at) { return (uint64_t)(now-at)<(UINT64_C(1)<<63); }
void mf_timers_init(MfTimers *t,uint32_t now) { *t=(MfTimers){0}; t->now=now; }
enum cmx_result mf_timer_arm(MfTimers *t,uint32_t at,MfHandle *out) {
  if (!out || (uint32_t)(at-t->now)>=UINT32_C(0x80000000)) return CMX_INVALID;
  if (t->next_sequence==UINT64_MAX) return CMX_FULL;
  for (unsigned i=0;i<MF_TIMERS;++i) if (!t->timer[i].active && !t->timer[i].retired) {
    MfTimer *p=&t->timer[i]; p->at=at; p->sequence=t->next_sequence++; p->active=1;
    *out=((uint64_t)p->generation<<5)|(i+1u); return CMX_OK;
  }
  return CMX_FULL;
}
static void timer_release(MfTimer *p) {
  p->active=0; if (p->generation==UINT32_MAX) p->retired=1; else ++p->generation;
}
enum cmx_result mf_timer_cancel(MfTimers *t,MfHandle h) {
  unsigned low=(unsigned)(h&31u);
  if (!low || low>MF_TIMERS || (h>>37)) return CMX_INVALID;
  MfTimer *p=&t->timer[low-1];
  if (!p->active || (h>>5)!=p->generation) return CMX_INVALID;
  timer_release(p); return CMX_OK;
}
enum cmx_result mf_timer_poll(MfTimers *t,uint32_t now,MfHandle *out,size_t *count) {
  if (!out || !count || *count<MF_TIMERS || (uint32_t)(now-t->now)>=UINT32_C(0x80000000)) return CMX_INVALID;
  t->now=now; size_t n=0;
  for (;;) {
    int first=-1;
    for (unsigned i=0;i<MF_TIMERS;++i) {
      MfTimer *p=&t->timer[i]; if (!p->active || !mf_due32(now,p->at)) continue;
      if (first<0 || (uint32_t)(now-p->at)>(uint32_t)(now-t->timer[first].at) ||
          (p->at==t->timer[first].at && p->sequence<t->timer[first].sequence)) first=(int)i;
    }
    if (first<0) break;
    MfTimer *p=&t->timer[first]; out[n++]=((uint64_t)p->generation<<5)|((unsigned)first+1u); timer_release(p);
  }
  *count=n; return CMX_OK;
}
uint64_t mf_div64(uint64_t a,uint64_t b) {
  if (!b) return UINT64_MAX;
  uint64_t q=0,r=0;
  for (unsigned bit=64;bit;--bit) {
    unsigned high=(unsigned)(r>>63); r=(r<<1)|((a>>(bit-1))&1u);
    if (high || r>=b) { r-=b; q|=UINT64_C(1)<<(bit-1); }
  }
  return q;
}
enum cmx_result mf_periodic(uint32_t phase,uint32_t period,uint32_t now,uint32_t *next,uint32_t *skipped) {
  if (!next || !skipped || !period || period>=UINT32_C(0x80000000) || (uint32_t)(now-phase)>=UINT32_C(0x80000000)) return CMX_INVALID;
  uint32_t steps=(uint32_t)mf_div64((uint32_t)(now-phase),period)+1u;
  *next=phase+steps*period; *skipped=steps-1u; return CMX_OK;
}
enum cmx_result mf_counts(uint64_t us,uint32_t hz,uint32_t maximum,uint32_t *counts,uint64_t *remaining) {
  if (!counts || !remaining || !hz || !maximum || maximum>=UINT32_C(0x80000000) || us>mf_div64(UINT64_MAX,hz)) return CMX_INVALID;
  uint64_t product=us*hz,q=mf_div64(product,1000000);
  if (q*1000000u!=product) ++q;
  uint32_t taken=q>maximum?maximum:(uint32_t)q;
  *counts=taken;
  uint64_t completed_us=mf_div64((uint64_t)taken*1000000u,hz);
  *remaining=completed_us>=us?0:us-completed_us;
  return CMX_OK;
}
enum cmx_result mf_basepri(unsigned incoming,unsigned requested,unsigned bits,unsigned *effective) {
  if (!effective || !bits || bits>8 || !requested || requested>255 || incoming>255) return CMX_INVALID;
  unsigned mask=(1u<<(8u-bits))-1u;
  if ((incoming&mask) || (requested&mask)) return CMX_INVALID;
  *effective=!incoming || requested<incoming?requested:incoming; return CMX_OK;
}
void mf_sleep_init(MfSleep *s) { *s=(MfSleep){0}; }
enum cmx_result mf_sleep_arm(MfSleep *s,uint32_t start,uint32_t counts) {
  if (s->armed || !counts || counts>=UINT32_C(0x80000000)) return CMX_INVALID;
  s->start=start; s->counts=counts; s->armed=1; s->slept=s->credited=0; return CMX_OK;
}
void mf_sleep_wake(MfSleep *s) { s->ready=s->event=1; }
int mf_sleep_enter(MfSleep *s) {
  if (!s->armed || s->ready || s->event) return 0;
  s->slept=1; return 1;
}
enum cmx_result mf_sleep_finish(MfSleep *s,uint32_t now) {
  uint32_t delta=now-s->start;
  if (!s->armed || s->credited || delta>s->counts || UINT32_MAX-s->elapsed<delta) return CMX_INVALID;
  s->elapsed+=delta; s->credited=1; s->armed=0; return CMX_OK;
}
