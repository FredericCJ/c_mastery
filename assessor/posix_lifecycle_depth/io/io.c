#define _POSIX_C_SOURCE 200809L
#include "io.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#ifndef PC_MUTANT
#define PC_MUTANT 0
#endif
void pc_init(struct pc_copy *copy){if(copy)memset(copy,0,sizeof *copy);}
static enum pc_status fail(struct pc_copy *copy,int error){copy->error=error;return PC_ERROR;}
enum pc_status pc_resume(struct pc_copy *copy,struct pc_ops ops,unsigned budget){
    if(!copy||!ops.read||!ops.write)return PC_ERROR;
    if(copy->error)return PC_ERROR;
    if(copy->next>copy->end||copy->end>sizeof copy->bytes)return fail(copy,EINVAL);
    while(budget--){
        ssize_t n;size_t count;
        if(copy->next<copy->end){
            count=copy->end-copy->next;
            if(copy->committed>UINTMAX_MAX-count)return fail(copy,EOVERFLOW);
            n=ops.write(ops.context,copy->bytes+copy->next,count);
            if(n<0){int error=errno;
                if(error==EINTR)continue;
                if(error==EAGAIN||error==EWOULDBLOCK){
#if PC_MUTANT == 1
                    copy->next=0;
#endif
                    return PC_WAIT_WRITE;
                }return fail(copy,error?error:EIO);
            }
            if(n==0)return fail(copy,EIO);
            if((size_t)n>count)return fail(copy,EPROTO);
            copy->committed+=(size_t)n;
#if PC_MUTANT == 2
            copy->next=copy->end;
#else
            copy->next+=(size_t)n;
#endif
        }else{
            if(copy->eof)return PC_DONE;
            copy->next=copy->end=0;n=ops.read(ops.context,copy->bytes,sizeof copy->bytes);
            if(n<0){int error=errno;if(error==EINTR)continue;if(error==EAGAIN||error==EWOULDBLOCK)return PC_WAIT_READ;return fail(copy,error?error:EIO);}
            if((size_t)n>sizeof copy->bytes)return fail(copy,EPROTO);
            if(n==0){copy->eof=true;return PC_DONE;}copy->end=(size_t)n;
        }
    }return copy->eof&&copy->next==copy->end?PC_DONE:PC_YIELD;
}
uintmax_t pc_off_max(void){
    _Static_assert((off_t)-1<0,"This Linux ABI contract requires signed off_t");
    _Static_assert(sizeof(off_t)<=sizeof(uintmax_t),"off_t wider than course conversion");
    /* Linux selected ABI: no padding bits, conventional signed off_t. */
    return UINTMAX_MAX>>(sizeof(uintmax_t)*CHAR_BIT-sizeof(off_t)*CHAR_BIT+1);
}
int pc_extent(uintmax_t offset,uintmax_t length,uintmax_t file_size,off_t *converted){
    if(!converted)return EINVAL;
#if PC_MUTANT != 3
    if(offset>pc_off_max()||length>pc_off_max()-offset)return EOVERFLOW;
#endif
#if PC_MUTANT != 4
    if(offset>file_size||length>file_size-offset)return ERANGE;
#else
    (void)file_size;
#endif
    *converted=(off_t)offset;return 0;
}
int pc_write_record(int fd,uintmax_t offset,const unsigned char *data,size_t length,uintmax_t file_size,size_t *committed){
    off_t where=0;size_t done=0;int error,flags;
    if(!committed||(!data&&length))return EINVAL;
    error=pc_extent(offset,length,file_size,&where);if(error)return error;
    flags=fcntl(fd,F_GETFL);if(flags<0)return errno;
#if PC_MUTANT != 5
    if(flags&O_APPEND)return EINVAL;
#endif
    *committed=0;
    while(done<length){
        size_t chunk=length-done;if(chunk>4096)chunk=4096;
#if PC_MUTANT == 6
        ssize_t n=write(fd,data+done,chunk);
#else
        ssize_t n=pwrite(fd,data+done,chunk,where+(off_t)done);
#endif
        if(n<0){if(errno==EINTR)continue;*committed=done;return errno;}
        if(n==0){*committed=done;return EIO;}
        done+=(size_t)n;
    }*committed=done;return 0;
}
int pc_close_transition(bool *owned,int error,enum pc_close_policy policy){
    if(!owned||!*owned||(policy!=PC_LINUX_RELEASES&&policy!=PC_EINTR_RETAINS)||error<0)return EINVAL;
#if PC_MUTANT == 7
    policy=PC_EINTR_RETAINS;
#elif PC_MUTANT == 8
    policy=PC_LINUX_RELEASES;
#endif
    if(error!=EINTR||policy==PC_LINUX_RELEASES)*owned=false;
    return error;
}
