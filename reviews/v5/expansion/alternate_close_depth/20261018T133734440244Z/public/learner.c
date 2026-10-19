#include "close_owner.h"
enum QCStatus qc_init(QCOwner *o,int fd){(void)o;(void)fd;return QC_INVALID;}
enum QCStatus qc_once(QCOwner *o,QCClose c,void *x,QCObservation *v){(void)o;(void)c;(void)x;(void)v;return QC_INVALID;}
enum QCStatus qc_handoff(QCOwner *o,int *fd){(void)o;(void)fd;return QC_INVALID;}
