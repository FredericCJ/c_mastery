#ifndef OB_BUFFER_H
#define OB_BUFFER_H
#include <stddef.h>
#include <stdint.h>

struct ob_allocator
{
    void *context;
    void *(*allocate)(void *, size_t);
    void *(*resize)(void *, void *, size_t, size_t);
    void (*release)(void *, void *);
};

struct ob_record
{
    int number;
    void *pointer;
    char tag;
};

struct ob_construction
{
    struct ob_record value;
    unsigned initialized;
};

int ob_construct(struct ob_construction *, int, void *, char, unsigned);
int ob_record_equal(const struct ob_record *, const struct ob_record *);
int ob_record_encode(const struct ob_record *, unsigned char *, size_t);
int ob_cleanup(struct ob_construction *, struct ob_allocator);
int ob_size(size_t, size_t, size_t, size_t, size_t *);

struct ob_flex
{
    size_t length, capacity;
    unsigned char payload[];
};

struct ob_padded
{
    double alignment;
    unsigned char tag;
    unsigned char payload[];
};

struct ob_table
{
    struct ob_flex *value;
    struct ob_allocator allocation;
    uint32_t identity, generation;
    int retired;
};

struct ob_handle
{
    uint32_t identity, generation;
    size_t index;
};

int ob_create(struct ob_table *, struct ob_allocator, uint32_t, size_t);
int ob_resize(struct ob_table *, size_t);
int ob_append(struct ob_table *, unsigned char);
int ob_erase(struct ob_table *, size_t);
void ob_reset(struct ob_table *);
int ob_handle(const struct ob_table *, size_t, struct ob_handle *);
int ob_resolve(const struct ob_table *, struct ob_handle, unsigned char *);
int ob_table_move(struct ob_table *, struct ob_table *);

struct ob_split
{
    size_t length, capacity;
    unsigned char *payload;
    struct ob_allocator allocation;
};

int ob_split_create(struct ob_allocator, size_t, struct ob_split **);
void ob_split_drop(struct ob_split **);

struct ob_shared
{
    size_t references, length;
    struct ob_allocator allocation;
    unsigned char payload[];
};

int ob_shared_create(struct ob_allocator, const unsigned char *, size_t,
                     struct ob_shared **);
int ob_shared_retain(struct ob_shared *, struct ob_shared **);
void ob_shared_drop(struct ob_shared **);
int ob_workspace(int, int *, size_t, int *, unsigned *);
#endif
