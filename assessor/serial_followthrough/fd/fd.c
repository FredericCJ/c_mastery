#include "fd.h"
#ifndef FD_MUTANT
#define FD_MUTANT 0
#endif
static int metadata(const FdInput *i,SDFd *m){
 if(!i||i->dlc>15||i->fdf!=1||i->brs>1||i->esi>1||i->remote>1||i->capable>1)return FD_INVALID;
 if(i->remote)return FD_INVALID;
 if(!i->capable)return FD_CAPABILITY;
 if(sd_fd(i->dlc,i->fdf,i->brs,i->esi,i->remote,i->capable,m)!=SD_OK)return FD_INVALID;
 return m->length==i->length?FD_OK:FD_LENGTH;
}
int fd_admit(const FdInput *i,const uint8_t *data,size_t size,FdPacket *out){
 if(!out||(!data&&size))return FD_INVALID;
 FdPacket result={0};int status=metadata(i,&result.metadata);if(status!=FD_OK)return status;
 if(size!=i->length)return FD_LENGTH;
 result.payload_length=(unsigned)size;for(unsigned n=0;n<size;++n)result.payload[n]=data[n];*out=result;return FD_OK;
}
int fd_length(unsigned pad,const uint8_t *data,size_t size,unsigned brs,unsigned esi,FdPacket *out){
 if(!out||(!data&&size)||pad>1||brs>1||esi>1)return FD_INVALID;
 if(size>64)return FD_LENGTH;
 FdPacket r={0};unsigned dlc;
 for(dlc=0;dlc<16;++dlc){if(sd_fd(dlc,1,brs,esi,0,1,&r.metadata)!=SD_OK)return FD_INVALID;if(r.metadata.length>=size)break;}
 if(dlc==16||(!pad&&r.metadata.length!=size))return FD_LENGTH;
 r.payload_length=(unsigned)size;r.padding=r.metadata.length-(unsigned)size;
 for(unsigned n=0;n<size;++n)r.payload[n]=data[n];
 if(FD_MUTANT==1&&r.padding)r.payload[size]=0xa5;
 *out=r;return FD_OK;
}
int fd_esi(unsigned passive,unsigned requested,unsigned *bit){if(!bit||passive>1||requested>1)return FD_INVALID;*bit=passive|requested;return FD_OK;}
static unsigned crc_step(unsigned crc,unsigned bit,unsigned width){unsigned top=(crc>>(width-1))^bit;crc=(crc<<1)&((1u<<width)-1);if(top&1)crc^=width==17?0x1685bu:0x102899u;return crc;}
static void put(FdWire *w,unsigned bit){w->bits[w->count++]=(uint8_t)bit;}
int fd_wire(unsigned id,const FdPacket *p,unsigned current,FdWire *out){
 if(!p||!out||id>0x7ff||current>1||p->metadata.length>64||p->metadata.brs>1||p->metadata.esi>1)return FD_INVALID;
 unsigned dlc;SDFd m;
 for(dlc=0;dlc<16;++dlc){if(sd_fd(dlc,1,p->metadata.brs,p->metadata.esi,0,1,&m)!=SD_OK)return FD_INVALID;if(m.length==p->metadata.length)break;}
 if(dlc==16||m.crc_width!=p->metadata.crc_width)return FD_INVALID;
 uint8_t raw[534];unsigned n=0;raw[n++]=0;
 for(unsigned j=11;j>0;--j)raw[n++]=(uint8_t)((id>>(j-1))&1u);
 raw[n++]=0;raw[n++]=0;raw[n++]=1;raw[n++]=0;raw[n++]=m.brs;raw[n++]=m.esi;
 for(unsigned j=4;j>0;--j)raw[n++]=(uint8_t)((dlc>>(j-1))&1u);
 for(unsigned j=0;j<m.length;++j)for(unsigned k=8;k>0;--k)raw[n++]=(uint8_t)((p->payload[j]>>(k-1))&1u);
 FdWire w={0};w.width=m.crc_width;unsigned crc=current&&FD_MUTANT!=5?1u<<(w.width-1):0,last=2,run=0;
 for(unsigned j=0;j<n;++j){
  unsigned bit=raw[j];put(&w,bit);crc=crc_step(crc,bit,w.width);run=bit==last?run+1:1;last=bit;
  /* At the data/CRC boundary the single fixed bit replaces a possible dynamic
     bit. It is not counted or included in CRC. */
  if(run==5&&(j+1<n||FD_MUTANT==6)){last^=1;put(&w,last);crc=crc_step(crc,last,w.width);++w.dynamic_stuff;run=1;}
 }
 w.crc_start=w.count;uint8_t field[25];unsigned f=0;
 if(current){unsigned value=w.dynamic_stuff%8,gray=value^(value>>1),parity=((gray>>2)^(gray>>1)^gray)&1u,sbc=(gray<<1)|parity;
  if(FD_MUTANT==2)sbc^=1;
  for(unsigned j=4;j>0;--j){field[f]=(uint8_t)((sbc>>(j-1))&1u);crc=crc_step(crc,field[f++],w.width);}
 }
 w.crc=crc;for(unsigned j=w.width;j>0;--j)field[f++]=(uint8_t)((crc>>(j-1))&1u);
 put(&w,last^1u);
 for(unsigned j=0;j<f;++j){put(&w,field[j]);if((j+1)%4==0)put(&w,field[j]^1u);}
 for(unsigned j=0;j<10;++j)put(&w,1); /* transmitter's recessive ACK slot */
 *out=w;return FD_OK;
}
int fd_transaction(unsigned device,unsigned ram,unsigned address,unsigned bytes,unsigned done,FdTransaction *out){
 if(!out||device>1||ram>1||bytes>64)return FD_INVALID;
 unsigned header=device==FD_TCAN4550?4:2;
 if(done>header+bytes)return FD_INVALID;
 if(device==FD_TCAN4550){
  if(address>65535u||(address&3u)||(bytes&3u)||bytes>65536u-address)return FD_INVALID;
  if(ram&&(address<0x8000||address>0x87fc||bytes>0x8800-address))return FD_INVALID;
 }
 else if(address>4095u||(ram&&((address&3u)||(bytes&3u)||address<0x400||address>0xbfc||bytes>0xc00-address)))return FD_INVALID;
 unsigned payload=done>header?done-header:0,unit=(device==FD_TCAN4550||ram)?4:1;
 FdTransaction r={header,bytes,header+bytes,payload/unit*unit,payload%unit};
 if(FD_MUTANT==3)r.committed_bytes=payload;
 *out=r;return FD_OK;
}
int fd_header(unsigned device,unsigned write,unsigned address,unsigned words,uint8_t *out,size_t capacity,size_t *used){
 if(!out||!used||device>1||write>1||!words||words>16||(address&3u))return FD_INVALID;
 unsigned base=device==FD_TCAN4550?0x8000:0x400,end=device==FD_TCAN4550?0x8800:0xc00,n=device==FD_TCAN4550?4:2;
 if(address<base||address>=end||4*words>end-address)return FD_INVALID;
 if(capacity<n)return FD_CAPACITY;
 uint8_t h[4];
 if(device==FD_TCAN4550){h[0]=(uint8_t)(write?0x61:0x41);h[1]=(uint8_t)(address>>8);h[2]=(uint8_t)address;h[3]=(uint8_t)words;}
 else{h[0]=(uint8_t)(((write?2u:3u)<<4)|(address>>8));h[1]=(uint8_t)address;}
 for(unsigned i=0;i<n;++i)out[i]=h[i];
 *used=n;return FD_OK;
}
int fd_power(unsigned device,unsigned supply,unsigned logic,unsigned transceiver,unsigned transceiver_logic){
 if(device>1)return FD_INVALID;
 if(device==FD_TCAN4550)return supply>=6000&&supply<=24000&&logic>=3135&&logic<=5250&&transceiver==0&&transceiver_logic==0?FD_OK:FD_INVALID;
 /* Explicit typical application contract: same3.3V logic/controller domain,
    separate5V transceiver supply. This is not all MCP2518FD legal circuits. */
 return supply==3300&&logic==3300&&transceiver==5000&&transceiver_logic==3300?FD_OK:FD_INVALID;
}
int fd_tdc(const uint8_t *tx,size_t n,const uint8_t *rx,size_t samples,unsigned ticks,unsigned sample,unsigned delay,unsigned offset,FdTdc *out){
 if(!out||!tx||!rx||!n||n>64||!ticks||ticks>64||!sample||sample>=ticks||delay>63||!offset||offset>63)return FD_INVALID;
 unsigned end=(unsigned)n*ticks+sample;
 if(samples!=end)return FD_LENGTH;
 for(unsigned i=0;i<n;++i)if(tx[i]>1)return FD_INVALID;
 for(unsigned i=0;i<samples;++i)if(rx[i]>1)return FD_INVALID;
 FdTdc r={0};r.first_reaction=end;unsigned position=delay+offset;
 for(unsigned i=0;i<n;++i){
  unsigned at=i*ticks+position;
  if(at>=end){++r.skipped;continue;}
  ++r.checked;
  if(rx[at]!=tx[i]){++r.mismatches;unsigned next=at<sample?sample:((at-sample)/ticks+1)*ticks+sample;if(r.mismatches==1)r.first_reaction=next;}
 }
 if(FD_MUTANT==4&&r.skipped){r.mismatches+=r.skipped;}
 *out=r;return FD_OK;
}
