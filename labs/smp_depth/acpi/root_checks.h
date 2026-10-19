#ifndef CMASTERY_SMP_DEPTH_ROOT_CHECKS_H
#define CMASTERY_SMP_DEPTH_ROOT_CHECKS_H
#include "checks.h"
static void sa_checksum_at(unsigned char *p,size_t n,size_t at){unsigned sum=0;p[at]=0;for(size_t i=0;i<n;++i)sum=(sum+p[i])&255u;p[at]=(unsigned char)(0u-sum);}
static int sa_root_checks(void){
 unsigned char p[52],root[52],rsdp[36];SaTopology t={0},old={0};
 sa_base(p,52);p[44]=0;p[45]=8;p[46]=17;p[47]=6;sa_w32(p+48,1);sa_sum(p,52);
 sa_base(root,40);root[0]='R';root[1]='S';root[2]='D';root[3]='T';sa_w32(root+36,0x4000);sa_sum(root,40);
 for(unsigned i=0;i<36;++i)rsdp[i]=0;
 const char signature[]="RSD PTR ";for(unsigned i=0;i<8;++i)rsdp[i]=(unsigned char)signature[i];sa_w32(rsdp+16,0x2000);sa_checksum_at(rsdp,20,8);
 SaRegion regions[]={{0x2000,root,40},{0x4000,p,52}};
 SAC(sa_discover(rsdp,20,regions,2,16,0,&t)==SA_OK&&t.count==1&&t.cpu[0].apic==6&&t.cpu[0].uid==17);old=t;
 for(unsigned n=0;n<20;++n){SAC(sa_discover(rsdp,n,regions,2,16,0,&t)==SA_INVALID);SAC(sa_same(&t,&old,sizeof t));}
 for(unsigned n=1;n<52;++n){regions[1].length=n;SAC(sa_discover(rsdp,20,regions,2,16,0,&t)==SA_INVALID&&sa_same(&t,&old,sizeof t));}regions[1].length=52;
 regions[1].address=0x2010;SAC(sa_discover(rsdp,20,regions,2,16,0,&t)==SA_INVALID);regions[1].address=UINT64_MAX-10;SAC(sa_discover(rsdp,20,regions,2,16,0,&t)==SA_INVALID);regions[1].address=0x4000;
 root[9]^=1;SAC(sa_discover(rsdp,20,regions,2,16,0,&t)==SA_INVALID);root[9]^=1;
 sa_w32(root+36,0x4001);sa_sum(root,40);SAC(sa_discover(rsdp,20,regions,2,16,0,&t)==SA_INVALID);sa_w32(root+36,0x4000);sa_sum(root,40);
 rsdp[15]=2;sa_w32(rsdp+20,36);sa_w32(rsdp+24,0x2000);sa_w32(rsdp+28,1);sa_checksum_at(rsdp,20,8);sa_checksum_at(rsdp,36,32);
 root[0]='X';sa_w32(root+4,44);sa_w32(root+36,0x4000);sa_w32(root+40,1);sa_sum(root,44);
 regions[0].address=UINT64_C(0x100002000);regions[0].length=44;regions[1].address=UINT64_C(0x100004000);
 SAC(sa_discover(rsdp,36,regions,2,16,0,&t)==SA_OK&&t.cpu[0].apic==6);
 for(unsigned n=20;n<36;++n)SAC(sa_discover(rsdp,n,regions,2,16,0,&t)==SA_INVALID);
 rsdp[32]^=1;SAC(sa_discover(rsdp,36,regions,2,16,0,&t)==SA_INVALID);rsdp[32]^=1;
 sa_w32(root+4,43);sa_sum(root,43);SAC(sa_discover(rsdp,36,regions,2,16,0,&t)==SA_INVALID);
 sa_w32(root+4,52);sa_w32(root+44,0x4000);sa_w32(root+48,1);sa_sum(root,52);regions[0].length=52;SAC(sa_discover(rsdp,36,regions,2,16,0,&t)==SA_DUPLICATE&&sa_same(&t,&old,sizeof t));
 sa_w32(root+4,36);sa_sum(root,36);SAC(sa_discover(rsdp,36,regions,2,16,0,&t)==SA_EMPTY);
 rsdp[15]=1;sa_checksum_at(rsdp,20,8);SAC(sa_discover(rsdp,36,regions,2,16,0,&t)==SA_UNSUPPORTED);
 return 0;
}
#endif
