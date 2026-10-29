#include "bus.h"
int mb_map(unsigned part,unsigned revision,struct mb_map*out){
 if(!out || part>MB_F407 || revision!=21)return MB_INVALID;
 struct mb_map m={0};m.part=part;m.manual_revision=revision;unsigned memory=(1u<<MB_CPU_S)|(1u<<MB_DMA1_M)|(1u<<MB_DMA2_M)|(1u<<MB_DMA2_P);
 m.regions[MB_CCM]=(struct mb_region){0x10000000,65536,1,1u<<MB_CPU_D};
#ifdef MB_MUTANT_CCM
 m.regions[MB_CCM].masters|=1u<<MB_DMA2_M;
#endif
 m.regions[MB_SRAM1]=(struct mb_region){0x20000000,114688,1,memory};m.regions[MB_SRAM2]=(struct mb_region){0x2001c000,16384,1,memory};
 if(part==MB_F439)m.regions[MB_SRAM3]=(struct mb_region){0x20020000,65536,1,memory};
#ifdef MB_MUTANT_F407
 m.regions[MB_SRAM3]=(struct mb_region){0x20020000,65536,1,memory};
#endif
 m.regions[MB_APB1]=(struct mb_region){0x40000000,0x8000,1,(1u<<MB_CPU_S)|(1u<<MB_DMA1_P)|(1u<<MB_DMA2_P)};m.regions[MB_APB2]=(struct mb_region){0x40010000,0x8000,1,(1u<<MB_CPU_S)|(1u<<MB_DMA2_P)};
 m.ordinary_bytes=114688+16384+(part==MB_F439?65536:0);*out=m;return MB_OK;
}
int mb_place(unsigned part,unsigned revision,unsigned region,unsigned master,uint32_t address,uint32_t bytes,unsigned alignment,uint32_t*out){
 struct mb_map map;if(!out || region>MB_SRAM3 || master>MB_DMA2_P || !bytes || !alignment || alignment>32 || (alignment&(alignment-1)) || address%alignment || mb_map(part,revision,&map)!=MB_OK)return MB_INVALID;
 struct mb_region r=map.regions[region];if(!r.present)return MB_INVALID;if(!(r.masters&(1u<<master)))return MB_UNREACHABLE;if(address<r.base || address-r.base>=r.bytes)return MB_EXTENT;
#ifndef MB_MUTANT_END
 if(bytes>r.bytes-(address-r.base))return MB_EXTENT;
#endif
 *out=address-r.base;return MB_OK;
}
int mb_bind(struct mb_lease*s,unsigned char*source,unsigned char*destination,uint32_t bytes,uint32_t source_address,unsigned source_region,uint32_t destination_address,unsigned destination_region){
 uint32_t offset;if(!s || !source || !destination || source==destination || !bytes || bytes>64)return PD_INVALID;if(s->bound || s->transfer.phase)return PD_BUSY;
 if(mb_place(MB_F439,21,source_region,MB_DMA2_P,source_address,bytes,1,&offset)!=MB_OK || mb_place(MB_F439,21,destination_region,MB_DMA2_M,destination_address,bytes,1,&offset)!=MB_OK)return PD_INVALID;
 struct pd_extent e={source_address,destination_address,bytes,bytes,bytes,1};if(pd_validate(PD_F439,0,&e,1,&offset)!=PD_OK)return PD_INVALID;
 struct mb_lease next={0};next.source=source;next.destination=destination;next.bytes=bytes;next.source_address=source_address;next.destination_address=destination_address;next.bound=1;*s=next;return PD_OK;
}
int mb_give(struct mb_lease*s,struct pd_io io,uint32_t token){if(!s || !s->bound)return PD_INVALID;struct pd_extent e={s->source_address,s->destination_address,s->bytes,s->bytes,s->bytes,1};return pd_submit(&s->transfer,io,PD_F439,0,&e,1,token);}
static int cpu_owned(const struct mb_lease*s){
#ifdef MB_MUTANT_RELEASE
 return !s->transfer.phase || s->transfer.phase==3;
#else
 return !s->transfer.phase;
#endif
}
int mb_cpu_write(struct mb_lease*s,uint32_t i,unsigned char value){if(!s || !s->bound || i>=s->bytes)return PD_INVALID;if(!cpu_owned(s))return PD_BUSY;s->source[i]=value;return PD_OK;}
int mb_cpu_read(const struct mb_lease*s,uint32_t i,unsigned char*out){if(!s || !s->bound || !out || i>=s->bytes)return PD_INVALID;if(!cpu_owned(s))return PD_BUSY;*out=s->destination[i];return PD_OK;}
int mb_poll(struct mb_lease*s,struct pd_io io,uint32_t token){if(!s || !s->bound)return PD_INVALID;return pd_poll(&s->transfer,io,token);}
int mb_finish(struct mb_lease*s,struct pd_io io,uint32_t token,struct pd_result*out){if(!s || !s->bound)return PD_INVALID;return pd_finalize(&s->transfer,io,token,out);}
int mb_arbitrate(struct mb_bus_config c,struct mb_bus_trace*out){
 if(!out || !c.active_mask || c.active_mask>7 || !c.rounds || c.rounds>8 || !c.max_beats || c.max_beats>8 || c.max_wait>8)return MB_INVALID;
 for(unsigned i=0;i<3;++i)if((c.active_mask&(1u<<i)) && (!c.beats[i] || c.beats[i]>c.max_beats || c.wait_cycles[i]>c.max_wait))return MB_INVALID;
 struct mb_bus_trace t={0};unsigned time=0,last[3]={0};t.bound=2*c.max_beats*(1+c.max_wait);
 for(unsigned round=0;round<c.rounds;++round)for(unsigned candidate=0;candidate<3;++candidate)if(c.active_mask&(1u<<candidate)){
  unsigned master=candidate;
#ifdef MB_MUTANT_FAIR
  master=0;
#endif
  unsigned duration=c.beats[master]*(1+c.wait_cycles[master]);
#ifdef MB_MUTANT_WAIT
  duration=c.beats[master];
#endif
  unsigned i=t.count++;t.master[i]=master;t.start[i]=time;t.wait[i]=time-last[master];time+=duration;t.finish[i]=time;last[master]=time;
 }
 *out=t;return MB_OK;
}
int mb_bridge(unsigned width,uint32_t value,uint32_t*out){
 if(!out || (width!=8 && width!=16 && width!=32) || (width==8 && value>255) || (width==16 && value>65535))return MB_INVALID;
 uint32_t word=value;if(width==8)word=value*0x01010101u;else if(width==16)word=value|(value<<16);
#ifdef MB_MUTANT_BRIDGE
 word=value;
#endif
 *out=word;return MB_OK;
}
int mb_usart_width(unsigned width){
#ifdef MB_MUTANT_WIDTH
 return width==8 || width==16 || width==32;
#else
 return width==16 || width==32;
#endif
}
