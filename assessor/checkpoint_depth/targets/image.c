#include "checkpoint_writer.h"
#include <stdint.h>
volatile unsigned cpw_bss;
volatile unsigned cpw_data=0x13579bdu;
#ifndef CPW_NO_HELPERS
void *memcpy(void *restrict,const void *restrict,size_t);
void *memset(void *,int,size_t);
static int helpers(void){
    unsigned char from[40],to[40];
    for(size_t i=0;i<40;++i)from[i]=(unsigned char)i;
    for(size_t n=0;n<=32;++n)for(size_t offset=0;offset<4;++offset){
        if(memset(to,0xa5,sizeof to)!=to || memcpy(to+offset,from+offset,n)!=to+offset)return 0;
        for(size_t i=0;i<40;++i)if(to[i]!=(i>=offset && i<offset+n?from[i]:0xa5))return 0;
    }
    return 1;
}
#endif
#ifdef CPW_BAD_RELOCATION
volatile unsigned *volatile cpw_absolute=&cpw_data;
#endif
static long raw_write(long fd,const unsigned char *data,size_t n){
    long result;
    __asm__ volatile("syscall":"=a"(result):"a"(1L),"D"(fd),"S"(data),"d"(n):"rcx","r11","memory");
    return result;
}
static CPWResult writer(void *context,const unsigned char *data,size_t n){
    (void)context;return cpw_raw(raw_write(1,data,n));
}
int cpw_image(uintptr_t *initial){
    if(((uintptr_t)initial&15) || initial[0]!=3 || initial[4]!=0 || cpw_bss!=0 || cpw_data!=0x13579bdu)return 70;
#ifndef CPW_NO_HELPERS
    if(!helpers())return 77;
#endif
    const unsigned char *argument=(const unsigned char *)initial[2];
    for(size_t i=0;i<9;++i)if(argument[i]!=(unsigned char)('A'+i))return 71;
    if(argument[9])return 72;
    CPWTransfer transfer;
    if(cpw_begin(&transfer,argument,9,91)!=CPW_PENDING)return 73;
    for(unsigned turns=0;turns<16 && transfer.status==CPW_PENDING;++turns)(void)cpw_step(&transfer,writer,0);
    if(transfer.status!=CPW_DONE || transfer.accepted!=9)return 74;
    /* Actual failure is a raw Linux -EBADF, not the libc -1/errno convention. */
    CPWResult bad=cpw_raw(raw_write(-1,argument,1));
    if(bad.kind!=CPW_ERROR || bad.cause!=9)return 75;
    unsigned char marker[]="CP_ENTRY_OK 0000000000000000\n";
    uintptr_t address=(uintptr_t)&cpw_data;
    for(unsigned i=0;i<16;++i){unsigned digit=(unsigned)((address>>(4*i))&15);marker[27-i]=(unsigned char)(digit<10?'0'+digit:'a'+digit-10);}
    if(raw_write(2,marker,sizeof marker-1)!=(long)(sizeof marker-1))return 76;
    return 0;
}
