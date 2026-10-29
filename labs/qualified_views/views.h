#ifndef FV_VIEWS_H
#define FV_VIEWS_H
#include <stddef.h>

enum fv_status
{
    FV_OK,
    FV_INVALID,
    FV_FULL,
    FV_RANGE,
    FV_UNFINISHED
};

struct fv_record
{
    int values[4];
    int *link;
};

struct fv_view
{
    const int *data;
    size_t length;
};

struct fv_destination
{
    int *data;
    size_t capacity;
};
enum fv_status fv_reset(struct fv_record *owner);
struct fv_view fv_initialize(struct fv_record *caller_owner);
enum fv_status fv_update(struct fv_record *owner, size_t index, int value);
struct fv_view fv_view(const struct fv_record *owner);
/* Nonempty source/destination accessed ranges must be disjoint. */
enum fv_status fv_copy(struct fv_destination destination,
                       struct fv_view source);
/* All positions refer to the same admitted array; overlap is permitted. */
enum fv_status fv_move(int *base, size_t extent, size_t destination,
                       size_t source, size_t count);
struct fv_owner;
struct fv_owner *fv_owner(void);
struct fv_view fv_owner_view(const struct fv_owner *owner);
int *fv_owner_mutable(struct fv_owner *owner);
/* Ordinary RAM device model only; no hardware or interthread guarantee. */
unsigned fv_device_twice(volatile unsigned *status);
#endif
