#ifndef PM_MAP_H
#define PM_MAP_H
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct pm_map
{
    unsigned char *base;
    size_t mapped, logical;
    off_t offset;
    dev_t device;
    ino_t inode;
    uint64_t generation;
    int active;
};

struct pm_borrow
{
    size_t offset, length;
    uint64_t generation;
};

struct pm_record
{
    uint64_t session;
    size_t length;
    unsigned char payload[64];
};

typedef void (*pm_cut_fn)(unsigned, void *);
int pm_map(int, off_t, size_t, size_t, struct pm_map *);
int pm_remap(struct pm_map *, int, size_t, size_t);
int pm_copy(const struct pm_map *, struct pm_borrow, void *, size_t);
int pm_unmap(struct pm_map *);
int pm_encode(unsigned char[128], const struct pm_record *, unsigned);
int pm_decode(const unsigned char *, size_t, struct pm_record *);
int pm_publish(int, uint64_t, const void *, size_t, pm_cut_fn, void *);
int pm_read_record(int, uint64_t, struct pm_record *);
int pm_replace(int, uint64_t, uint64_t, const void *, size_t);
#endif
