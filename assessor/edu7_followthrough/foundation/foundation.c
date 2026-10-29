#include "foundation.h"
#include <string.h>
#ifndef EF_MUTANT
#define EF_MUTANT 0
#endif
int ef_budget(unsigned schema,unsigned extra,struct ef_budget *out) {
  if(!out || (schema!=1 && schema!=2) || extra>8 || (schema==1 && extra)) return EF_INVALID;
  unsigned n=schema==1?33:35+(extra?2+extra:0);
  struct ef_budget b={n,n+7,(n+7+23)/24,(n+7)%24,32,38,45,96};
  if(!b.last)b.last=24;
  if(EF_MUTANT==1)b.canonical=80;
  *out=b;return EF_OK;
}
int ef_enqueue(struct ef_route *s,const uint8_t *p,size_t n,unsigned ingress,unsigned copy,unsigned version,uint32_t *token) {
  struct edu_datagram in,out;unsigned port;uint8_t next;
  if(!s||!p||!token||copy>1||(version!=1&&version!=2)||ingress>1) return EF_INVALID;
  if(s->active)return EF_BUSY;
  if(s->generation==UINT32_MAX)return EF_FULL;
  if(edu_network_decode(p,n,&in))return EF_DROP;
  if(version==2 && in.destination==68 && in.source!=34 && in.hop>1 && ingress==0) {
    out=in;out.hop--;port=1;next=2;
  } else if(edu_route(&in,ingress,&out,&port,&next)) return EF_DROP;
  s->packet=out;s->mode=copy;s->borrow=copy?0:p+6;
  if(EF_MUTANT==2)s->borrow=p+6;
  s->egress=port;s->next=next;s->active=1;*token=++s->generation;return EF_OK;
}
int ef_peek(const struct ef_route *s,uint32_t token,struct edu_frame *out) {
  if(!s||!out)return EF_INVALID;
  if(!s->active||!token||token!=s->generation)return EF_STALE;
  struct edu_datagram d=s->packet;struct edu_frame f={0};size_t n;
  if(s->borrow)memcpy(d.payload,s->borrow,d.length);
  if(edu_network_encode(&d,f.payload,&n))return EF_INVALID;
  f.kind=0;f.source=(uint8_t)(s->egress?1:2);f.destination=(uint8_t)s->next;f.length=(uint8_t)n;
  if(EF_MUTANT==3)f.destination=d.destination;
  *out=f;return EF_OK;
}
int ef_release(struct ef_route *s,uint32_t token) {
  if(!s)return EF_INVALID;
  if(!s->active||!token||(EF_MUTANT!=4&&token!=s->generation))return EF_STALE;
  s->active=0;s->borrow=0;return EF_OK;
}
int ef_local(const uint8_t *p,size_t n,unsigned node,struct edu_datagram *out) {
  struct edu_datagram d;
  if(!p||!out||(node!=17&&node!=51&&node!=68))return EF_INVALID;
  if(edu_network_decode(p,n,&d)||d.destination!=node)return EF_DROP;
  *out=d;return EF_OK;
}
int ef_encode(const struct edu_record *r,uint8_t *out,size_t cap,size_t *length) {
  uint8_t b[33];size_t n;
  if(!r||!out||!length||r->has_value>1||r->has_unit>1)return EF_INVALID;
  if(edu_record_encode(r,b,&n))return EF_INVALID;
  if(EF_MUTANT==5 && n)out[0]=b[0];
  if(cap<n)return EF_FULL;
  memcpy(out,b,n);*length=n;return EF_OK;
}
int ef_wide_encode(const struct ef_wide *w,uint8_t *out,size_t cap,size_t *length) {
  uint8_t base[33],b[45];size_t n,z=1;
  if(!w||!out||!length||w->extra_length>8||w->value<INT32_MIN||w->value>INT32_MAX)return EF_INVALID;
  struct edu_record r=w->base;r.value=0;
  if(ef_encode(&r,base,sizeof base,&n))return EF_INVALID;
  b[0]=2;
  for(size_t i=1;i<n;) {
    unsigned tag=base[i++],len=base[i++];b[z++]=(uint8_t)tag;
    if(tag==3) {
      uint32_t u=(uint32_t)w->value;b[z++]=4;
      for(unsigned k=0;k<4;k++)b[z++]=(uint8_t)(u>>(24-8*k));
    } else {b[z++]=(uint8_t)len;memcpy(b+z,base+i,len);z+=len;}
    i+=len;
  }
  if(w->extra_length){b[z++]=6;b[z++]=w->extra_length;memcpy(b+z,w->extra,w->extra_length);z+=w->extra_length;}
  if(z>cap)return EF_FULL;
  memcpy(out,b,z);*length=z;return EF_OK;
}
int ef_wide_decode(const uint8_t *p,size_t n,struct ef_wide *out) {
  uint8_t base[33];size_t z=1;unsigned prior=0;struct ef_wide w={0};
  if(!p||!out||n<1||n>45||p[0]!=2)return EF_INVALID;
  base[0]=1;
  for(size_t i=1;i<n;) {
    if(n-i<2)return EF_INVALID;
    unsigned tag=p[i++],len=p[i++];if(tag<=prior||tag>6||len>n-i)return EF_INVALID;prior=tag;
    if(tag==6) {if(!len||len>8)return EF_INVALID;w.extra_length=(uint8_t)len;memcpy(w.extra,p+i,len);}
    else {
      size_t need=2+(tag==3?2:len);if(need>sizeof base-z)return EF_INVALID;
      base[z++]=(uint8_t)tag;
      if(tag==3) {
        if(len!=4)return EF_INVALID;
        uint32_t u=(uint32_t)p[i]<<24|(uint32_t)p[i+1]<<16|(uint32_t)p[i+2]<<8|p[i+3];
        w.value=u<=INT32_MAX?(int64_t)u:-1-(int64_t)(UINT32_MAX-u);
        if(EF_MUTANT==6)w.value=u;
        base[z++]=2;base[z++]=0;base[z++]=0;
      } else {base[z++]=(uint8_t)len;memcpy(base+z,p+i,len);z+=len;}
    }
    i+=len;
  }
  if(edu_record_decode(base,z,&w.base))return EF_INVALID;
  *out=w;return EF_OK;
}
int ef_frame_encode(const struct edu_frame *f,unsigned version,uint8_t *out,size_t cap,size_t *length) {
  if(!f||!out||!length||(version!=1&&version!=2))return EF_INVALID;
  if(version==1){uint8_t b[96];size_t n;if(edu_frame_encode(f,b,sizeof b,&n))return EF_INVALID;if(cap<n)return EF_FULL;memcpy(out,b,n);*length=n;return EF_OK;}
  if(f->kind>2||f->length>40||f->source<1||f->source>2||f->destination<1||f->destination>2||f->source==f->destination||(f->kind&&f->length))return EF_INVALID;
  uint8_t b[47]={2,f->kind,f->source,f->destination,f->length};size_t n=f->length+7,need=2,z=0;
  memcpy(b+5,f->payload,f->length);uint16_t crc=edu_crc(b,n-2);b[n-2]=(uint8_t)(crc>>8);b[n-1]=(uint8_t)crc;
  for(size_t i=0;i<n;i++)need+=(b[i]==0x7e||b[i]==0x7d)?2:1;
  if(cap<need)return EF_FULL;
  out[z++]=0x7e;
  for(size_t i=0;i<n;i++){if(b[i]==0x7e||b[i]==0x7d){out[z++]=0x7d;out[z++]=b[i]^(EF_MUTANT==7?0x20:0x40);}else out[z++]=b[i];}
  out[z++]=0x7e;*length=z;return EF_OK;
}
int ef_frame_byte(struct ef_framer *s,unsigned version,uint8_t b,unsigned fault,struct edu_frame *out) {
  if(!s||!out||fault>1||(version!=1&&version!=2))return -1;
  if(version==1){
    if(EF_MUTANT!=8 && !fault && s->legacy.active && !s->legacy.discard && s->legacy.escape && b!=0x7e && b!=0x5e && b!=0x5d)fault=1;
    return edu_frame_byte(&s->legacy,b,(int)fault,out);
  }
  if(fault){s->discard=1;s->length=0;return -1;}
  if(b==0x7e) {
    int result=0;unsigned n=s->length;
    if(s->active&&(n||s->discard||s->escape)) {
      uint8_t *p=s->bytes;
      if(s->discard||s->escape||n<7||p[0]!=version||p[1]>2||p[2]<1||p[2]>2||p[3]<1||p[3]>2||p[2]==p[3]||p[4]>40||n!=7u+p[4]||(p[1]&&p[4])||edu_crc(p,n-2)!=((unsigned)p[n-2]*256+p[n-1]))result=-1;
      else {struct edu_frame f={p[1],p[2],p[3],p[4],{0}};memcpy(f.payload,p+5,f.length);*out=f;result=1;}
    }
    s->active=1;s->length=s->escape=s->discard=0;return result;
  }
  if(!s->active||s->discard)return 0;
  if(s->escape){if(EF_MUTANT!=8 && b!=0x3e && b!=0x3d){s->discard=1;return -1;}b^=0x40;s->escape=0;}else if(b==0x7d){s->escape=1;return 0;}
  if(s->length==47){s->discard=1;return -1;}
  s->bytes[s->length++]=b;return 0;
}
