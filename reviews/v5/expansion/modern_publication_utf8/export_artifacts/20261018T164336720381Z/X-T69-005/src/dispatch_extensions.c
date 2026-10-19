#include "dispatch_extensions.h"
DSIntInterface ds_int_interface(const DSIntReceiver *r){(void)r;return (DSIntInterface){0};}
DSByteInterface ds_byte_interface(const DSBytesReceiver *r){(void)r;return (DSByteInterface){0};}
DSProgress ds_read_progress(DSReader r,unsigned char *p,size_t n){(void)r;(void)p;(void)n;return (DSProgress){DS_FAILED,0};}
int ds_borrow_closure_init(DSBorrowClosure *c,const int32_t *p,size_t n,unsigned once){(void)c;(void)p;(void)n;(void)once;return DS_FAILED;}
int ds_borrow_closure_call(DSBorrowClosure *c,int a,int *out){(void)c;(void)a;(void)out;return DS_FAILED;}
int ds_owned_closure_init(DSOwnedClosure *c,DSResource r){(void)c;(void)r;return DS_FAILED;}
int ds_owned_closure_call(DSOwnedClosure *c,DSOwnedBody b,int a,DSOwnedResult *out){(void)c;(void)b;(void)a;(void)out;return DS_FAILED;}
void ds_owned_closure_drop(DSOwnedClosure *c){(void)c;}
void ds_clone_capture_init(DSCloneCapture *c,DAllocator a,DSCloneOps o){(void)c;(void)a;(void)o;}
int ds_clone_capture_adopt(DSCloneCapture *c,const unsigned char *p,size_t n,DSThunk call,void *a,void *b){(void)c;(void)p;(void)n;(void)call;(void)a;(void)b;return DS_FAILED;}
int ds_clone_capture(DSCloneCapture *d,const DSCloneCapture *s){(void)d;(void)s;return DS_FAILED;}
void ds_clone_capture_drop(DSCloneCapture *c){(void)c;}
