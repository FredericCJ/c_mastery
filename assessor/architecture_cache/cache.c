#include "cache.h"
static int power(unsigned n){return n && !(n&(n-1));}
enum ACHStatus ach_trace(const int64_t*a,size_t n,ACHConfig c,ACHTrace*out){
 if(!out || n>64 || (n&&!a) || !power(c.lines) || c.lines>8 || !power(c.block_bytes) || c.block_bytes>256 || !power(c.ways) || c.ways>c.lines || c.shadow_fifo>1)return ACH_INVALID;
 for(size_t i=0;i<n;++i)if(a[i]<0 || (uint64_t)a[i]>UINT32_MAX)return ACH_INVALID;
 unsigned ways=c.ways;
#ifdef ACH_MUTANT_WAYS
 ways=1;
#endif
 unsigned sets=c.lines/ways,used[8]={0},shadow_count=0,seen_count=0;uint32_t blocks[8]={0},shadow[8]={0},seen[64]={0};ACHTrace result={0};result.count=(unsigned)n;
 for(size_t i=0;i<n;++i){
  uint32_t address=(uint32_t)a[i],block=address/c.block_bytes,index=block%sets,tag=block/sets;unsigned base=index*ways;int hit=-1,old=0,position=-1;
  for(unsigned j=0;j<used[index];++j)if(blocks[base+j]==block)hit=(int)j;
#ifdef ACH_MUTANT_VALID
  if(blocks[base]==block)hit=0;
#endif
  for(unsigned j=0;j<seen_count;++j)if(seen[j]==block)old=1;
  for(unsigned j=0;j<shadow_count;++j)if(shadow[j]==block)position=(int)j;
  unsigned category=hit>=0?ACH_HIT:!old?ACH_COMPULSORY:position>=0?ACH_CONFLICT:ACH_CAPACITY;
#ifdef ACH_MUTANT_SHADOW
  if(hit<0 && old)category=ACH_CONFLICT;
#endif
  if(category==ACH_HIT)++result.hits;else if(category==ACH_COMPULSORY)++result.compulsory;else if(category==ACH_CONFLICT)++result.conflict;else ++result.capacity;
  if(hit<0){if(used[index]<ways)++used[index];hit=(int)used[index]-1;}
  for(int j=hit;j>0;--j)blocks[base+(unsigned)j]=blocks[base+(unsigned)j-1];
  blocks[base]=block;
  if(!old)seen[seen_count++]=block;
  unsigned fifo=c.shadow_fifo;
#ifdef ACH_MUTANT_FIFO
  fifo=0;
#endif
  if(position<0){if(shadow_count<c.lines)++shadow_count;position=(int)shadow_count-1;}
  else if(fifo)position=-1;
  if(position>=0){for(int j=position;j>0;--j)shadow[j]=shadow[j-1];shadow[0]=block;}
  ACHStep *s=&result.steps[i];s->offset=address%c.block_bytes;s->index=index;s->tag=tag;s->category=category;s->shadow_count=shadow_count;
  for(unsigned j=0;j<c.lines;++j){s->blocks[j]=blocks[j];s->valid[j]=j%ways<used[j/ways];s->shadow[j]=shadow[j];}
 }
 *out=result;return ACH_OK;
}
static uint32_t gcd(uint32_t a,uint32_t b){while(b){uint32_t r=a%b;a=b;b=r;}return a;}
static ACHFraction reduce(ACHFraction a){uint32_t d=gcd(a.numerator,a.denominator);return (ACHFraction){a.numerator/d,a.denominator/d};}
static int multiply(ACHFraction a,ACHFraction b,ACHFraction*out){
 uint32_t x=gcd(a.numerator,b.denominator),y=gcd(b.numerator,a.denominator);a.numerator/=x;b.denominator/=x;b.numerator/=y;a.denominator/=y;uint64_t n=(uint64_t)a.numerator*b.numerator,d=(uint64_t)a.denominator*b.denominator;if(n>UINT32_MAX || d>UINT32_MAX)return 0;*out=reduce((ACHFraction){(uint32_t)n,(uint32_t)d});return 1;
}
static int combine(ACHFraction a,ACHFraction b,int subtract,ACHFraction*out){
 uint32_t g=gcd(a.denominator,b.denominator);uint64_t x=(uint64_t)a.numerator*(b.denominator/g),y=(uint64_t)b.numerator*(a.denominator/g),d=(uint64_t)a.denominator*(b.denominator/g);if((subtract && x<y) || (!subtract && x>UINT64_MAX-y))return 0;uint64_t n=subtract?x-y:x+y;if(n>UINT32_MAX || d>UINT32_MAX)return 0;*out=reduce((ACHFraction){(uint32_t)n,(uint32_t)d});return 1;
}
enum ACHStatus ach_cost(ACHCostInput x,ACHCost*out){
 if(!out || !x.hit_ns.denominator || !x.miss_ns.denominator || !x.cpi.denominator || !x.miss_rate.denominator || x.miss_rate.numerator>x.miss_rate.denominator || !x.references || x.misses>x.references || !x.frequency_hz || x.total_miss_latency>1)return ACH_INVALID;
 ACHFraction hit=reduce(x.hit_ns),penalty=reduce(x.miss_ns),tmp;ACHCost result;
 if(x.total_miss_latency){
#ifndef ACH_MUTANT_ADDITIONAL
  if(!combine(penalty,hit,1,&penalty))return ACH_RANGE;
#endif
 }
 if(!multiply(penalty,(ACHFraction){x.misses,x.references},&tmp) || !combine(hit,tmp,0,&result.amat_ns) || !multiply(reduce(x.cpi),(ACHFraction){x.instructions,1},&result.base_cycles) || !multiply(reduce(x.miss_rate),(ACHFraction){x.memory_references,1},&tmp) || !multiply(tmp,(ACHFraction){x.penalty_cycles,1},&result.stall_cycles) || !combine(result.base_cycles,result.stall_cycles,0,&result.total_cycles))return ACH_RANGE;
 uint32_t scale=1000000000;
#ifdef ACH_MUTANT_UNITS
 scale=1;
#endif
 if(!multiply(result.total_cycles,reduce((ACHFraction){scale,x.frequency_hz}),&result.time_ns))return ACH_RANGE;
 *out=result;return ACH_OK;
}
enum ACHStatus ach_overlap(const ACHMiss*a,size_t n,unsigned slots,ACHOverlap*out){
 if(!out || n>16 || (n&&!a) || !slots || slots>4)return ACH_INVALID;
 for(size_t i=0;i<n;++i)if(a[i].release>256 || !a[i].latency || a[i].latency>256 || a[i].predecessor<-1 || a[i].predecessor>=(int)i)return ACH_INVALID;
#ifdef ACH_MUTANT_OVERLAP
 slots=1;
#endif
 ACHOverlap result={0};unsigned issued[16]={0},done=0;result.count=(unsigned)n;
 for(unsigned tick=0;done<n && tick<=4352;++tick){
  unsigned active=0;done=0;for(unsigned i=0;i<n;++i)if(issued[i]){if(result.finish[i]>tick)++active;else ++done;}
  if(done==n)break;
  if(active==slots)continue;
  for(unsigned i=0;i<n;++i)if(!issued[i] && a[i].release<=tick && (a[i].predecessor<0 || (issued[a[i].predecessor] && result.finish[a[i].predecessor]<=tick))){issued[i]=1;result.start[i]=tick;result.finish[i]=tick+a[i].latency;if(result.finish[i]>result.makespan)result.makespan=result.finish[i];break;}
 }
 if(done!=n)return ACH_RANGE;
 *out=result;return ACH_OK;
}
