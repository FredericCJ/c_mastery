#define _POSIX_C_SOURCE 200809L
#include "cache_file.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#ifndef MPB_FILE_MUTATION
#define MPB_FILE_MUTATION 0
#endif
static void names(unsigned id,char final[12],char staged[12]){
    const char a[]="node0.cache",b[]="node0.stage";size_t i;
    for(i=0;i<sizeof a;++i)final[i]=a[i];
    for(i=0;i<sizeof b;++i)staged[i]=b[i];
    final[4]=staged[4]=(char)('0'+id);
}
int mpb_file_recover(int directory,unsigned id){
    char final[12],stage[12];if(directory<0||id>7)return MPB_INVALID;names(id,final,stage);
    if(unlinkat(directory,stage,0)<0&&errno!=ENOENT)return MPB_IO;
    return MPB_OK;
}
int mpb_file_put(int directory,unsigned id,const unsigned char *bytes,size_t length,MpbBoundary boundary,void *context){
    char final[12],stage[12];int fd=-1,renamed=0;size_t offset=0;ssize_t n;
    if(directory<0||id>7||!bytes||!length||length>800)return MPB_INVALID;
    names(id,final,stage);
    fd=openat(directory,MPB_FILE_MUTATION==1?final:stage,O_WRONLY|O_CREAT|O_CLOEXEC|O_NOFOLLOW|(MPB_FILE_MUTATION==1?O_TRUNC:O_EXCL),0600);
    if(fd<0)return MPB_IO;
    if(boundary&&boundary(context,MPB_OPEN,offset))goto failure;
    while(offset<length){size_t take=length-offset;if(take>7)take=7;n=write(fd,bytes+offset,take);if(n<0&&errno==EINTR)continue;if(n<=0)goto failure;offset+=(size_t)n;if(boundary&&boundary(context,MPB_WRITE,offset))goto failure;}
    if(fsync(fd)<0)goto failure;
    if(boundary&&boundary(context,MPB_FILE_SYNC,offset))goto failure;
    {int status=close(fd);fd=-1;if(status<0)goto failure;}
    if(boundary&&boundary(context,MPB_CLOSE,offset))goto failure;
    if(MPB_FILE_MUTATION!=1&&renameat(directory,stage,directory,final)<0)goto failure;
    renamed=1;
    if(boundary&&boundary(context,MPB_RENAME,offset))goto failure;
    if(fsync(directory)<0)goto failure;
    if(boundary&&boundary(context,MPB_DIRECTORY_SYNC,offset))goto failure;
    return MPB_OK;
failure:
    if(fd>=0)(void)close(fd);
    if(!renamed)(void)unlinkat(directory,stage,0);
    return renamed?MPB_UNCERTAIN:MPB_IO;
}
int mpb_file_match(int directory,unsigned id,const unsigned char *expected,size_t length){
    char final[12],stage[12];unsigned char buffer[64];size_t offset=0;int fd,match=1;ssize_t n;
    if(directory<0||id>7||!expected||!length||length>800)return MPB_INVALID;
    names(id,final,stage);
    fd=openat(directory,final,O_RDONLY|O_CLOEXEC|O_NOFOLLOW);if(fd<0)return errno==ENOENT?MPB_MISS:MPB_IO;
    for(;;){n=read(fd,buffer,sizeof buffer);if(n<0&&errno==EINTR)continue;if(n<0){(void)close(fd);return MPB_IO;}if(!n)break;
        if((size_t)n>length-offset){match=0;break;}
        for(size_t i=0;i<(size_t)n;++i)if((MPB_FILE_MUTATION!=2||offset+i<8)&&buffer[i]!=expected[offset+i])match=0;
        offset+=(size_t)n;
    }
    if(close(fd)<0)return MPB_IO;
    return match&&offset==length?MPB_OK:MPB_MISS;
}
