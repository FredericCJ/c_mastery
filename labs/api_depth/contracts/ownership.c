#include "ownership.h"
/* Intentional learner scaffold, not an unfinished assessor solution. */
AcCloseReport ac_close_all(AcLedger *l,unsigned f){(void)l;(void)f;return (AcCloseReport){AC_NOT_IMPLEMENTED,0};}
AcStatus ac_resource_acquire(AcLedger *l,unsigned id,unsigned request,int fail,int lost){(void)l;(void)id;(void)request;(void)fail;(void)lost;return AC_NOT_IMPLEMENTED;}
AcStatus ac_resource_query(const AcLedger *l,unsigned id,unsigned request,int *out){(void)l;(void)id;(void)request;(void)out;return AC_NOT_IMPLEMENTED;}
AcStatus ac_resource_close(AcLedger *l,unsigned id,int fail){(void)l;(void)id;(void)fail;return AC_NOT_IMPLEMENTED;}
AcStatus ac_session_admit(const AcSession *s){(void)s;return AC_NOT_IMPLEMENTED;}
AcStatus ac_session_init(AcSession *s,AcLedger *l,unsigned fail,int optional){(void)s;(void)l;(void)fail;(void)optional;return AC_NOT_IMPLEMENTED;}
AcStatus ac_session_stop(AcSession *s,AcLedger *l){(void)s;(void)l;return AC_NOT_IMPLEMENTED;}
AcStatus ac_session_destroy(AcSession *s){(void)s;return AC_NOT_IMPLEMENTED;}
AcStatus ac_session_restart(AcSession *s,AcLedger *l,int optional){(void)s;(void)l;(void)optional;return AC_NOT_IMPLEMENTED;}
AcStatus ac_table_alloc(AcTable *t,int v,AcToken *out){(void)t;(void)v;(void)out;return AC_NOT_IMPLEMENTED;}
AcStatus ac_table_get(const AcTable *t,AcToken h,int *out){(void)t;(void)h;(void)out;return AC_NOT_IMPLEMENTED;}
AcStatus ac_table_free(AcTable *t,AcToken h){(void)t;(void)h;return AC_NOT_IMPLEMENTED;}
AcStatus ac_shared_retain(AcShared *s){(void)s;return AC_NOT_IMPLEMENTED;}
AcStatus ac_shared_link(AcShared *parent,AcShared *child){(void)parent;(void)child;return AC_NOT_IMPLEMENTED;}
AcStatus ac_shared_drop(AcShared *s){(void)s;return AC_NOT_IMPLEMENTED;}
AcStatus ac_lease_read(const AcLeaseOwner *o,unsigned token,unsigned now,int *out){(void)o;(void)token;(void)now;(void)out;return AC_NOT_IMPLEMENTED;}
AcStatus ac_lease_reuse(AcLeaseOwner *o,unsigned now,int value){(void)o;(void)now;(void)value;return AC_NOT_IMPLEMENTED;}
AcStatus ac_epoch_update(AcEpochOwner *o,size_t i,uint8_t value){(void)o;(void)i;(void)value;return AC_NOT_IMPLEMENTED;}
AcStatus ac_epoch_snapshot(const AcEpochOwner *o,AcSnapshot *out){(void)o;(void)out;return AC_NOT_IMPLEMENTED;}
AcStatus ac_epoch_read(const AcEpochOwner *o,AcSnapshot s,size_t i,uint8_t *out){(void)o;(void)s;(void)i;(void)out;return AC_NOT_IMPLEMENTED;}
AcStatus ac_epoch_reset(AcEpochOwner *o){(void)o;return AC_NOT_IMPLEMENTED;}
