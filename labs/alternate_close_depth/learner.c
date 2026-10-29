#include "close_owner.h"
enum qc_status qc_init(struct qc_owner *o,int fd){(void)o;(void)fd;return QC_INVALID;}
enum qc_status qc_once(struct qc_owner *o,qc_close_fn c,void *x,struct qc_observation *v){(void)o;(void)c;(void)x;(void)v;return QC_INVALID;}
enum qc_status qc_handoff(struct qc_owner *o,int *fd){(void)o;(void)fd;return QC_INVALID;}
