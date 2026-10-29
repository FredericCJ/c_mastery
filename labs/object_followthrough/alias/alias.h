#ifndef OS_ALIAS_H
#define OS_ALIAS_H
#include "depth.h"

struct os_allocator
{
    struct fd_allocator allocator;
    size_t capacity, alignment;
};

struct os_wide
{
    _Alignas(64) int value;
    unsigned char *resource;
};

typedef int (*os_drop_fn)(void *, struct os_wide *);

enum os_state
{
    OS_EMPTY,
    OS_LIVE,
    OS_BORROWED,
    OS_CLEANUP
};

struct os_wide_slot
{
    struct os_wide *value;
    struct os_allocator allocation;
    os_drop_fn drop;
    void *context;
    enum os_state state;
};

int os_acquire(struct fd_slot *, struct os_allocator);
int os_wide_acquire(struct os_wide_slot *, struct os_allocator, int, os_drop_fn,
                    void *);
int os_wide_borrow(struct os_wide_slot *, const struct os_wide **);
int os_wide_return(struct os_wide_slot *, const struct os_wide **);
int os_wide_reset(struct os_wide_slot *);
int os_clone(void *, const struct fd_value *);
int os_wire(const unsigned char *, size_t, struct fd_value *);
uint32_t os_host_representation(uint32_t);
uint32_t os_wire_integer(const unsigned char *);
void os_transform(unsigned char *restrict, const unsigned char *restrict,
                  size_t, unsigned char);
void os_ordinary(unsigned char *, const unsigned char *, size_t, unsigned char);
void os_nested(unsigned char *restrict, const unsigned char *restrict, size_t,
               unsigned char);
void os_shared(unsigned char *restrict, const unsigned char *restrict,
               const unsigned char *restrict, size_t);
int os_partition(unsigned char *, size_t, size_t, size_t, size_t, unsigned char,
                 int);
enum fd_status os_move(unsigned char *, size_t, size_t, size_t, size_t);

struct os_container
{
    uint32_t prefix;
    int member;
};

int os_member(struct os_container *, int *, int *);
#ifdef UINTPTR_MAX
int os_tag_roundtrip(struct os_container *, unsigned, uintptr_t *, int *);
#endif
#endif
