#include "follow.h"
#ifndef FF_MUTATION
#define FF_MUTATION 0
#endif
enum fd_status ff_interval(size_t offset,size_t count,size_t width,size_t limit,struct ff_interval *out) {
    size_t end;
    if (!out) return FD_INVALID;
#if FF_MUTATION == 1
    out->begin=offset;
#endif
    enum fd_status status=fd_extent(offset,count,width,limit,&end);
    if (status!=FD_OK) return status;
#if FF_MUTATION == 2
    if (end==limit) return FD_RANGE;
#endif
    *out=(struct ff_interval){offset,end};return FD_OK;
}
int ff_copy_initialize(void *context,struct fd_value *destination) {
    if (!context || !destination) return 1;
    const struct fd_value *source=context;
    const unsigned char *from=(const unsigned char *)source;
    unsigned char *to=(unsigned char *)destination;
    for (size_t i=0;i<sizeof *source;++i) to[i]=from[i];
#if FF_MUTATION == 3
    destination->serial=0;
#endif
    return 0;
}
enum fd_status ff_encode(struct fd_record record,unsigned char *out,size_t capacity) {
    enum fd_status status=fd_encode(record,out,capacity);
#if FF_MUTATION == 4
    if (status==FD_OK) { unsigned char temp=out[2];out[2]=out[3];out[3]=temp; }
#endif
    return status;
}
enum fd_status ff_decode(const unsigned char *wire,size_t length,struct fd_record *out) {
#if FF_MUTATION == 4
    if (wire && length==8) {
        unsigned char copied[8];
        for (size_t i=0;i<8;++i) copied[i]=wire[i];
        copied[2]=wire[3];copied[3]=wire[2];
        return fd_decode(copied,8,out);
    }
#endif
    return fd_decode(wire,length,out);
}
