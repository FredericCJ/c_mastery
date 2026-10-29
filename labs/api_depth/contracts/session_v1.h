#ifndef CMASTERY_SESSION_V1_H
#define CMASTERY_SESSION_V1_H
#include <stddef.h>
#include <stdint.h>

struct ac_exposed_v1
{
    uint32_t identity, count;
};

struct ac_typed_v1
{
    uint32_t private_words[2];
};
struct ac_opaque;

struct ac_allocate
{
    void *context;
    void *(*allocate)(void *, size_t, size_t);
    void (*release)(void *, void *);
};

int ac_typed_v1_set(struct ac_typed_v1 *, uint32_t identity, uint32_t count);
int ac_typed_v1_get(const struct ac_typed_v1 *, struct ac_exposed_v1 *);
int ac_opaque_create(uint32_t identity, uint32_t count, struct ac_allocate,
                     struct ac_opaque **);
int ac_opaque_get(const struct ac_opaque *, struct ac_exposed_v1 *);
void ac_opaque_destroy(struct ac_opaque *);
size_t ac_opaque_size(void);
size_t ac_opaque_alignment(void);
#endif
