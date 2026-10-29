#include "acpi.h"
#ifndef SA_MUTANT
#define SA_MUTANT 0
#endif
static uint32_t le32(const unsigned char *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
int sa_parse(const void *bytes,size_t available,unsigned capacity,unsigned profile,struct sa_topology *out){
 const unsigned char *p=bytes;
 if(!p||!out||available<44||capacity<1||capacity>16||profile>SA_X2_RECORDS)return SA_INVALID;
 if(p[0]!='A'||p[1]!='P'||p[2]!='I'||p[3]!='C')return SA_INVALID;
 uint32_t n=le32(p+4);if(n<44||n>65536||n>available)return SA_INVALID;
 unsigned sum=0;for(uint32_t i=0;i<n;++i)sum=(sum+p[i])&255u;
 if(sum&&SA_MUTANT!=1)return SA_CHECKSUM;
 struct sa_topology next={0};next.lapic=le32(p+36);next.flags=le32(p+40);
 if(!next.lapic||(next.lapic&4095u))return SA_UNSUPPORTED;
 for(uint32_t at=44;at<n;){
  if(n-at<2||p[at+1]<2||p[at+1]>n-at){if(SA_MUTANT==4){*out=next;return SA_OK;}return SA_INVALID;}
  unsigned kind=p[at],length=p[at+1];uint32_t uid=0,id=0,flags=0;
  if(kind==5)return SA_UNSUPPORTED;
  if(kind==0){if(length!=8)return SA_INVALID;uid=p[at+2];id=p[at+3];flags=le32(p+at+4);}
  else if(kind==9){if(profile!=SA_X2_RECORDS)return SA_UNSUPPORTED;if(length!=16)return SA_INVALID;id=le32(p+at+4);flags=le32(p+at+8);uid=le32(p+at+12);if(SA_MUTANT==3){id&=255;uid&=255;}}
  else{++next.unknown;at+=length;continue;}
  if(flags&1u){
   if(next.count==capacity)return SA_FULL;
   for(unsigned i=0;i<next.count;++i)if(SA_MUTANT!=2&&(next.cpu[i].apic==id||next.cpu[i].uid==uid))return SA_DUPLICATE;
   next.cpu[next.count++]=(struct sa_cpu){uid,id,kind};
  }else if(flags&2u)++next.online_capable;else ++next.disabled;
  at+=length;
 }
 if(!next.count)return SA_EMPTY;
 *out=next;return SA_OK;
}
static uint64_t le64(const unsigned char *p){return le32(p)|((uint64_t)le32(p+4)<<32);}
static int sig(const unsigned char *p,const char *s,unsigned n){for(unsigned i=0;i<n;++i)if(p[i]!=(unsigned char)s[i])return 0;return 1;}
static int sum_ok(const unsigned char *p,size_t n){unsigned sum=0;for(size_t i=0;i<n;++i)sum=(sum+p[i])&255u;return sum==0;}
static const unsigned char *mapped(const struct sa_region *r,unsigned nr,uint64_t address,size_t n){
 for(unsigned i=0;i<nr;++i)if(address>=r[i].address){uint64_t d=address-r[i].address;if(d<=r[i].length&&n<=r[i].length-(size_t)d)return r[i].bytes+(size_t)d;}
 return 0;
}
static const unsigned char *table(const struct sa_region *r,unsigned nr,uint64_t address,uint32_t *length){
 const unsigned char *p=mapped(r,nr,address,36);if(!p)return 0;
 uint32_t n=le32(p+4);if(n<36||n>65536)return 0;
 p=mapped(r,nr,address,n);if(!p||!sum_ok(p,n))return 0;
 *length=n;return p;
}
int sa_discover(const void *rsdp,size_t available,const struct sa_region *r,unsigned nr,unsigned capacity,unsigned profile,struct sa_topology *out){
 const unsigned char *p=rsdp;
 if(!p||!r||!out||available<20||!nr||nr>16||!sig(p,"RSD PTR ",8)||!sum_ok(p,20))return SA_INVALID;
 for(unsigned i=0;i<nr;++i){
  if(!r[i].bytes||!r[i].length||r[i].length>UINT64_MAX-r[i].address)return SA_INVALID;
  for(unsigned j=0;j<i;++j)if(r[i].address<r[j].address+r[j].length&&r[j].address<r[i].address+r[i].length)return SA_INVALID;
 }
 unsigned stride=4;uint64_t root=le32(p+16);
 if(p[15]==2){if(available<36||le32(p+20)!=36||!sum_ok(p,36))return SA_INVALID;root=le64(p+24);stride=8;}
 else if(p[15]!=0)return SA_UNSUPPORTED;
 uint32_t n=0;const unsigned char *q=table(r,nr,root,&n);
 if(!q||!sig(q,stride==4?"RSDT":"XSDT",4)||(n-36)%stride)return SA_INVALID;
 const unsigned char *madt=0;uint32_t size=0;
 for(uint32_t at=36;at<n;at+=stride){uint32_t z=0;const unsigned char *t=table(r,nr,stride==4?le32(q+at):le64(q+at),&z);if(!t)return SA_INVALID;
  if(sig(t,"APIC",4)){if(madt)return SA_DUPLICATE;madt=t;size=z;}}
 if(!madt)return SA_EMPTY;
 return sa_parse(madt,size,capacity,profile,out);
}
