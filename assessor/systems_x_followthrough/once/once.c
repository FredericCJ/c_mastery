#include "once.h"
#include <limits.h>
#ifndef SX_ONCE_MUTANT
#define SX_ONCE_MUTANT 0
#endif
int sx_once_init(SxOnce*o,SxJournal*j,unsigned id,unsigned tag,int retry){
 if(!o||!j||id<1||id>3||tag>255||(retry!=0&&retry!=1))return MB_INVALID;
 mb_once_init(&o->once,retry!=0);o->journal=j;o->id=id;o->tag=tag;o->owner=3;o->stage=0;o->attempts=0;o->publications=0;o->checksum=0;
 for(unsigned i=0;i<8;i++)o->table[i]=0;
 return MB_OK;
}
int sx_once_enter(SxOnce*o,unsigned participant,unsigned out[8],unsigned*checksum){
 unsigned pair[2];int rc;if(!o||!out||!checksum||participant>=3)return MB_INVALID;
 if(o->attempts==UINT_MAX&&atomic_load(&o->once.state)==0)return MB_FULL;
 rc=mb_once_enter(&o->once,participant,pair);
 if(rc==MB_INIT){o->owner=participant;o->stage=0;o->attempts++;}
 if(rc==MB_OK){for(unsigned i=0;i<8;i++)out[i]=o->table[i];*checksum=o->checksum;}
 return rc;
}
static int owner(SxOnce*o,unsigned p){return o&&p<3&&o->owner==p&&atomic_load(&o->once.state)==p+1;}
int sx_once_cancel(SxOnce*o,unsigned p){int rc;if(!owner(o,p))return MB_STALE;
 rc=mb_once_finish(&o->once,p,false,0,0);if(rc!=MB_OK)return rc;
 o->owner=3;o->stage=0;o->checksum=0;for(unsigned i=0;i<8;i++)o->table[i]=0;return MB_OK;
}
int sx_once_step(SxOnce*o,unsigned p){
 if(!owner(o,p))return MB_STALE;
 if(o->stage==0){
  unsigned i;for(i=0;i<o->journal->count;i++)if(o->journal->id[i]==o->id)break;
#if SX_ONCE_MUTANT == 1
  i=o->journal->count;
#endif
  if(i<o->journal->count){if(o->journal->tag[i]!=o->tag){(void)sx_once_cancel(o,p);return MB_FAILED;}}
  else{
   if(o->journal->count==3){(void)sx_once_cancel(o,p);return MB_FULL;}
   /* Serialized synthetic append+identity: one model transition. */
   o->journal->id[i]=o->id;o->journal->tag[i]=o->tag;o->journal->count++;o->journal->effects++;
  }
  o->stage++;return MB_BUSY;
 }
 if(o->stage<=8){unsigned i=o->stage-1;o->table[i]=17*i+1;o->stage++;
#if SX_ONCE_MUTANT == 2
  if(i==3){o->checksum=484;o->publications++;o->owner=3;return mb_once_finish(&o->once,p,true,8,484);}
#endif
  return MB_BUSY;
 }
 o->checksum=0;for(unsigned i=0;i<8;i++)o->checksum+=o->table[i];
 {int rc=mb_once_finish(&o->once,p,true,8,o->checksum);if(rc!=MB_OK)return rc;}
 o->publications++;o->owner=3;return MB_OK;
}
