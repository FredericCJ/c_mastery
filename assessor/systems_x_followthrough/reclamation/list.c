#include "list.h"
#ifndef SX_LIST_MUTANT
#define SX_LIST_MUTANT 0
#endif
static int lock(struct sx_list*l){return l&&!atomic_flag_test_and_set_explicit(&l->gate,memory_order_acquire);}
static int done(struct sx_list*l,int status){atomic_flag_clear_explicit(&l->gate,memory_order_release);return status;}
static int lease(const struct sx_list*l,struct sx_participant p){return p.slot<2&&l->reader[p.slot].registered&&l->reader[p.slot].generation==p.generation;}
static unsigned key(const struct sx_list*l,unsigned i){return l->node[i].generation*4+i+1;}
static int index(const struct sx_list*l,unsigned k){unsigned rem=k%4,i;if(!rem)return -1;i=rem-1;return i<3&&l->node[i].state&&key(l,i)==k?(int)i:-1;}
static int protected(const struct sx_list*l,unsigned reader,unsigned k){const struct sx_reader*r=&l->reader[reader];int i=index(l,k);
 if(i<0||!r->active)return 0;
 if(l->mode==SX_HAZARD)return r->hazard[0]==k||r->hazard[1]==k;
 return l->node[i].state==1||l->epoch.reader_epoch[reader]<=l->epoch.retired_at[i];
}
int sx_list_init(struct sx_list*l,unsigned mode){if(!l||mode>SX_EPOCH)return MB_INVALID;*l=(struct sx_list){.gate=ATOMIC_FLAG_INIT};l->mode=mode;for(unsigned i=0;i<3;i++)l->node[i].generation=1;mb_epoch_init(&l->epoch);return MB_OK;}
int sx_register(struct sx_list*l,struct sx_participant*out){if(!l||!out)return MB_INVALID;if(!lock(l))return MB_BUSY;
 for(unsigned i=0;i<2;i++)if(!l->reader[i].registered&&l->reader[i].generation<3){l->reader[i].generation++;l->reader[i].registered=1;*out=(struct sx_participant){i,l->reader[i].generation};return done(l,MB_OK);}
 return done(l,MB_FULL);
}
int sx_depart(struct sx_list*l,struct sx_participant p){if(!l)return MB_INVALID;if(!lock(l))return MB_BUSY;
 if(!lease(l,p))return done(l,MB_STALE);
#if SX_LIST_MUTANT != 3
 if(l->reader[p.slot].active)return done(l,MB_BUSY);
#endif
 l->reader[p.slot].registered=0;l->reader[p.slot].hazard[0]=0;l->reader[p.slot].hazard[1]=0;return done(l,MB_OK);
}
int sx_enter(struct sx_list*l,struct sx_participant p){struct sx_reader*r;int rc;if(!l)return MB_INVALID;if(!lock(l))return MB_BUSY;if(!lease(l,p))return done(l,MB_STALE);r=&l->reader[p.slot];
 if(r->active)return done(l,MB_BUSY);
 if(r->section==UINT_MAX)return done(l,MB_FULL);
 if(l->mode==SX_EPOCH){rc=mb_epoch_enter(&l->epoch,p.slot);if(rc!=MB_OK)return done(l,rc);}
 r->active=1;r->section++;return done(l,MB_OK);
}
int sx_leave(struct sx_list*l,struct sx_participant p){struct sx_reader*r;int rc;if(!l)return MB_INVALID;if(!lock(l))return MB_BUSY;if(!lease(l,p)||!l->reader[p.slot].active)return done(l,MB_STALE);r=&l->reader[p.slot];
 if(l->mode==SX_EPOCH){rc=mb_epoch_leave(&l->epoch,p.slot);if(rc!=MB_OK)return done(l,rc);}
 r->hazard[0]=0;r->hazard[1]=0;r->active=0;return done(l,MB_OK);
}
int sx_append(struct sx_list*l,unsigned value){unsigned i,k;if(!l)return MB_INVALID;if(!lock(l))return MB_BUSY;
 for(i=0;i<3;i++)if(!l->node[i].state&&l->node[i].generation<=3&&(l->mode==SX_HAZARD||!l->epoch.reclaimed[i]))break;
 if(i==3)return done(l,MB_FULL);
 k=key(l,i);l->node[i].value=value;l->node[i].next=0;l->node[i].state=1;
 if(l->tail){int tail=index(l,l->tail);if(tail<0)return done(l,MB_INVALID);l->node[tail].next=k;}else l->head=k;
 l->tail=k;return done(l,MB_OK);
}
int sx_remove(struct sx_list*l,unsigned*out){int i,rc;unsigned k;if(!l||!out)return MB_INVALID;if(!lock(l))return MB_BUSY;if(!l->head)return done(l,MB_EMPTY);k=l->head;i=index(l,k);if(i<0)return done(l,MB_INVALID);
 if(l->mode==SX_EPOCH){rc=mb_epoch_retire(&l->epoch,(unsigned)i);if(rc!=MB_OK)return done(l,rc);}
 l->head=l->node[i].next;if(!l->head)l->tail=0;l->node[i].state=2;*out=l->node[i].value;return done(l,MB_OK);
}
int sx_observe(struct sx_list*l,struct sx_participant p,unsigned from,struct sx_observed*out){unsigned target;if(!l||!out)return MB_INVALID;if(!lock(l))return MB_BUSY;if(!lease(l,p)||!l->reader[p.slot].active)return done(l,MB_STALE);
 if(from){int i=index(l,from);if(i<0||!protected(l,p.slot,from))return done(l,MB_STALE);target=l->node[i].next;}else target=l->head;
 if(!target)return done(l,MB_EMPTY);
 *out=(struct sx_observed){l->reader[p.slot].section,from,target};return done(l,MB_OK);
}
int sx_protect(struct sx_list*l,struct sx_participant p,unsigned slot,struct sx_observed observation,struct sx_protected*out){unsigned current;int i;
 if(!l||!out||slot>=2)return MB_INVALID;
 if(!lock(l))return MB_BUSY;
 if(!lease(l,p)||!l->reader[p.slot].active||observation.section!=l->reader[p.slot].section)return done(l,MB_STALE);
 if(l->reader[p.slot].hazard[slot])return done(l,MB_BUSY);
 if(observation.from){i=index(l,observation.from);if(i<0||!protected(l,p.slot,observation.from))return done(l,MB_STALE);current=l->node[i].next;}else current=l->head;
 l->reader[p.slot].hazard[slot]=observation.target;
#if SX_LIST_MUTANT != 1
 if(current!=observation.target||index(l,observation.target)<0){l->reader[p.slot].hazard[slot]=0;return done(l,MB_RETRY);}
#else
 (void)current;
#endif
 *out=(struct sx_protected){observation.section,observation.target};return done(l,MB_OK);
}
int sx_read(struct sx_list*l,struct sx_participant p,struct sx_protected reference,unsigned*out){int i;if(!l||!out)return MB_INVALID;if(!lock(l))return MB_BUSY;
 if(!lease(l,p)||reference.section!=l->reader[p.slot].section||!protected(l,p.slot,reference.key))return done(l,MB_STALE);
 i=index(l,reference.key);if(i<0)return done(l,MB_STALE);*out=l->node[i].value;return done(l,MB_OK);
}
int sx_clear(struct sx_list*l,struct sx_participant p,unsigned slot){if(!l||slot>=2)return MB_INVALID;if(!lock(l))return MB_BUSY;if(!lease(l,p)||!l->reader[p.slot].active)return done(l,MB_STALE);l->reader[p.slot].hazard[slot]=0;return done(l,MB_OK);}
int sx_collect(struct sx_list*l,unsigned*out){unsigned count=0;if(!l||!out)return MB_INVALID;if(!lock(l))return MB_BUSY;
 if(l->mode==SX_EPOCH){count=mb_epoch_collect(&l->epoch);for(unsigned i=0;i<3;i++)if(l->epoch.reclaimed[i])l->node[i].state=0;}
 else for(unsigned i=0;i<3;i++)if(l->node[i].state==2){unsigned held=0,k=key(l,i);
  for(unsigned r=0;r<(SX_LIST_MUTANT==2?1u:2u);r++)for(unsigned s=0;s<2;s++)held|=l->reader[r].hazard[s]==k;
  if(!held){l->node[i].state=0;l->node[i].generation++;count++;}
 }
 *out=count;return done(l,MB_OK);
}
int sx_advance(struct sx_list*l){int rc;if(!l)return MB_INVALID;if(!lock(l))return MB_BUSY;if(l->mode!=SX_EPOCH)return done(l,MB_INVALID);rc=mb_epoch_advance(&l->epoch);return done(l,rc);}
