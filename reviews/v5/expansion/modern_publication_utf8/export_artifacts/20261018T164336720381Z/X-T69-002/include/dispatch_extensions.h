#ifndef CMASTERY_DISPATCH_EXTENSIONS_H
#define CMASTERY_DISPATCH_EXTENSIONS_H
#include "capture.h"
typedef struct {const int *object;} DSIntReceiver;
typedef struct {const unsigned char *bytes;size_t length;} DSBytesReceiver;
typedef struct {DSView view;} DSIntInterface;
typedef struct {DSView view;} DSByteInterface;
DSIntInterface ds_int_interface(const DSIntReceiver *);
DSByteInterface ds_byte_interface(const DSBytesReceiver *);
#define DS_RECEIVER(p) (_Generic((p),DSIntReceiver *:ds_int_interface,const DSIntReceiver *:ds_int_interface,DSBytesReceiver *:ds_byte_interface,const DSBytesReceiver *:ds_byte_interface)(p).view)
typedef struct {int status;size_t count;} DSProgress;
typedef struct {void *context;int (*read)(void *,unsigned char *,size_t,size_t *);} DSReader;
DSProgress ds_read_progress(DSReader,unsigned char *,size_t);
typedef struct {const int32_t *values;size_t count;unsigned once,active;} DSBorrowClosure;
int ds_borrow_closure_init(DSBorrowClosure *,const int32_t *,size_t,unsigned once);
int ds_borrow_closure_call(DSBorrowClosure *,int,int *);
typedef struct {void *object;void (*drop)(void *);} DSResource;
typedef struct {int status,value;DSResource retry;} DSOwnedResult;
typedef DSOwnedResult (*DSOwnedBody)(DSResource,int);
typedef struct {DSResource resource;unsigned active;} DSOwnedClosure;
int ds_owned_closure_init(DSOwnedClosure *,DSResource);
int ds_owned_closure_call(DSOwnedClosure *,DSOwnedBody,int,DSOwnedResult *);
void ds_owned_closure_drop(DSOwnedClosure *);
typedef struct {void *context;int (*clone)(void *,const void *,void **);void (*drop)(void *,void *);} DSCloneOps;
typedef struct {DSErased erased;DSCloneOps ops;void *parts[2];} DSCloneCapture;
void ds_clone_capture_init(DSCloneCapture *,DAllocator,DSCloneOps);
int ds_clone_capture_adopt(DSCloneCapture *,const unsigned char *,size_t,DSThunk,void *,void *);
int ds_clone_capture(DSCloneCapture *,const DSCloneCapture *);
void ds_clone_capture_drop(DSCloneCapture *);
#endif
