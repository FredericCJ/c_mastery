#ifndef CMASTERY_DISPATCH_EXTENSIONS_H
#define CMASTERY_DISPATCH_EXTENSIONS_H
#include "capture.h"
struct ds_int_receiver {const int *object;};
struct ds_bytes_receiver {const unsigned char *bytes;size_t length;};
struct ds_int_interface {struct ds_view view;};
struct ds_byte_interface {struct ds_view view;};
struct ds_int_interface ds_int_interface(const struct ds_int_receiver *);
struct ds_byte_interface ds_byte_interface(const struct ds_bytes_receiver *);
#define DS_RECEIVER(p) (_Generic((p),struct ds_int_receiver *:ds_int_interface,const struct ds_int_receiver *:ds_int_interface,struct ds_bytes_receiver *:ds_byte_interface,const struct ds_bytes_receiver *:ds_byte_interface)(p).view)
struct ds_progress {int status;size_t count;};
struct ds_reader {void *context;int (*read)(void *,unsigned char *,size_t,size_t *);};
struct ds_progress ds_read_progress(struct ds_reader,unsigned char *,size_t);
struct ds_borrow_closure {const int32_t *values;size_t count;unsigned once,active;};
int ds_borrow_closure_init(struct ds_borrow_closure *,const int32_t *,size_t,unsigned once);
int ds_borrow_closure_call(struct ds_borrow_closure *,int,int *);
struct ds_resource {void *object;void (*drop)(void *);};
struct ds_owned_result {int status,value;struct ds_resource retry;};
typedef struct ds_owned_result (*ds_owned_body_fn)(struct ds_resource,int);
struct ds_owned_closure {struct ds_resource resource;unsigned active;};
int ds_owned_closure_init(struct ds_owned_closure *,struct ds_resource);
int ds_owned_closure_call(struct ds_owned_closure *,ds_owned_body_fn,int,struct ds_owned_result *);
void ds_owned_closure_drop(struct ds_owned_closure *);
struct ds_clone_ops {void *context;int (*clone)(void *,const void *,void **);void (*drop)(void *,void *);};
struct ds_clone_capture {struct ds_erased erased;struct ds_clone_ops ops;void *parts[2];};
void ds_clone_capture_init(struct ds_clone_capture *,struct d_allocator,struct ds_clone_ops);
int ds_clone_capture_adopt(struct ds_clone_capture *,const unsigned char *,size_t,ds_thunk_fn,void *,void *);
int ds_clone_capture(struct ds_clone_capture *,const struct ds_clone_capture *);
void ds_clone_capture_drop(struct ds_clone_capture *);
#endif
