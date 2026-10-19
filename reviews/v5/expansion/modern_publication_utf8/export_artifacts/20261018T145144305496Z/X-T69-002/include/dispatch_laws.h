#ifndef CMASTERY_DISPATCH_LAWS_H
#define CMASTERY_DISPATCH_LAWS_H
#include "dispatch_extensions.h"
typedef struct {
 int (*length)(const void *,size_t *);
 int (*copy)(const void *,unsigned char *,size_t,size_t *);
} DSCopyOps;
typedef struct {const void *object;const DSCopyOps *ops;} DSCopyView;
int ds_copy_checked(DSCopyView,unsigned char *,size_t,size_t *);
typedef struct {unsigned marker;DSBytesReceiver bytes;DSBox scalar;} DSMulti;
int ds_multi_views(const DSMulti *,DSView [2]);
enum DSCaptureKind {DS_CAPTURE_I32,DS_CAPTURE_SELF_REFERENCE,DS_CAPTURE_TYPED_OBJECT};
typedef struct {enum DSCaptureKind kind;size_t required_alignment;unsigned returns_borrow;} DSCaptureSpec;
int ds_capture_checked(DSErased *,DSCaptureSpec,const int32_t *,size_t,void (*)(void *),void *);
#endif
