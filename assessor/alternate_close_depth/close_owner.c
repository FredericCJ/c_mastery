#include "close_owner.h"
#include <errno.h>
#ifndef QC_MUTATION
#define QC_MUTATION 0
#endif
enum qc_status qc_init(struct qc_owner *o,int fd)
{
    if(!o||fd<0)return QC_INVALID;
    *o=(struct qc_owner){fd,QC_OWNED};return QC_OK;
}
enum qc_status qc_once(struct qc_owner *o,qc_close_fn call,void *context,struct qc_observation *out)
{
    struct qc_raw raw;
    enum qc_status status;
    if(!o||!call||!out)return QC_INVALID;
    if(QC_MUTATION!=5 && o->state!=QC_OWNED && o->state!=QC_RETRY)return QC_TERMINAL;
    raw=call(context,o->fd);
    if(QC_MUTATION==2 && raw.result==-1 && raw.error==EINTR)raw=call(context,o->fd);
    if(raw.result==0 || (QC_MUTATION==1 && raw.error==EINTR) || (QC_MUTATION==3 && raw.result==-1)) {
        o->fd=-1;o->state=QC_CLOSED;status=QC_OK;
    } else if((raw.result==-1 && raw.error==EINTR) || (QC_MUTATION==6 && raw.result!=0)) {
        o->state=QC_RETRY;status=QC_AGAIN;
    } else {
        /* No ownership inference for EIO/ENOSPC/ENOSYS, EBADF or malformed seam. */
        o->state=QC_QUARANTINED;status=QC_UNKNOWN;
    }
    *out=(struct qc_observation){o->state,raw.result==0?0:raw.error};
    return status;
}
enum qc_status qc_handoff(struct qc_owner *o,int *fd)
{
    if(!o||!fd)return QC_INVALID;
    if(o->state!=QC_OWNED && o->state!=QC_RETRY && !(QC_MUTATION==4 && o->state==QC_QUARANTINED))return QC_TERMINAL;
    *fd=o->fd;o->fd=-1;o->state=QC_MOVED;return QC_OK;
}
