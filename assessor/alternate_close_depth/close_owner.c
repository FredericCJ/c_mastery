#include "close_owner.h"
#include <errno.h>
#ifndef QC_MUTATION
#define QC_MUTATION 0
#endif
enum QCStatus qc_init(QCOwner *o,int fd)
{
    if(!o||fd<0)return QC_INVALID;
    *o=(QCOwner){fd,QC_OWNED};return QC_OK;
}
enum QCStatus qc_once(QCOwner *o,QCClose call,void *context,QCObservation *out)
{
    QCRaw raw;
    enum QCStatus status;
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
    *out=(QCObservation){o->state,raw.result==0?0:raw.error};
    return status;
}
enum QCStatus qc_handoff(QCOwner *o,int *fd)
{
    if(!o||!fd)return QC_INVALID;
    if(o->state!=QC_OWNED && o->state!=QC_RETRY && !(QC_MUTATION==4 && o->state==QC_QUARANTINED))return QC_TERMINAL;
    *fd=o->fd;o->fd=-1;o->state=QC_MOVED;return QC_OK;
}
