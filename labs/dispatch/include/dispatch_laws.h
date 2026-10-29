#ifndef CMASTERY_DISPATCH_LAWS_H
#define CMASTERY_DISPATCH_LAWS_H
#include "dispatch_extensions.h"

struct ds_copy_ops
{
    int (*length)(const void *, size_t *);
    int (*copy)(const void *, unsigned char *, size_t, size_t *);
};

struct ds_copy_view
{
    const void *object;
    const struct ds_copy_ops *ops;
};

int ds_copy_checked(struct ds_copy_view, unsigned char *, size_t, size_t *);

struct ds_multi
{
    unsigned marker;
    struct ds_bytes_receiver bytes;
    struct ds_box scalar;
};

int ds_multi_views(const struct ds_multi *, struct ds_view[2]);

enum ds_capture_kind
{
    DS_CAPTURE_I32,
    DS_CAPTURE_SELF_REFERENCE,
    DS_CAPTURE_TYPED_OBJECT
};

struct ds_capture_spec
{
    enum ds_capture_kind kind;
    size_t required_alignment;
    unsigned returns_borrow;
};

int ds_capture_checked(struct ds_erased *, struct ds_capture_spec,
                       const int32_t *, size_t, void (*)(void *), void *);
#endif
