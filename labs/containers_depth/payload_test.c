#include "containers.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "CONTAINERS_PAYLOAD_ASSERT:%d:%s\n", __LINE__, \
                    #x);                                                   \
            exit(1);                                                       \
        }                                                                  \
    } while (0)

/* Test double shared by the allocator and the payload callbacks.  `fail` and
 * `clone_fail` are 1-based call ordinals: the take or clone call with that
 * ordinal fails. */
struct cd_state
{
    struct cd_payload_vector *owner;
    unsigned blocks;
    unsigned calls;
    unsigned fail;
    unsigned clones;
    unsigned clone_fail;
    unsigned resources;
    unsigned expected_count;
};

static struct cd_payload *take(void *opaque, size_t n)
{
    struct cd_state *s = opaque;
    CHECK(s->owner->count == s->expected_count);
    if (++s->calls == s->fail)
    {
        return NULL;
    }
    struct cd_payload *p = malloc(n * sizeof *p);
    if (p)
    {
        ++s->blocks;
    }
    return p;
}

static void release(void *opaque, struct cd_payload *p)
{
    struct cd_state *s = opaque;
    CHECK(s->blocks);
    --s->blocks;
    free(p);
}

static int clone(void *opaque, const struct cd_payload *input,
                 struct cd_payload *out)
{
    struct cd_state *s = opaque;
    CHECK(s->owner->count == s->expected_count);
    if (++s->clones == s->clone_fail)
    {
        return 1;
    }
    /* Resource ids are bit positions in a 32-bit ledger. */
    CHECK(s->clones < 32);
    *out = (struct cd_payload){input->value, s->clones};
    s->resources |= 1u << s->clones;
    return 0;
}

static void destroy(void *opaque, struct cd_payload *p)
{
    struct cd_state *s = opaque;
    CHECK(s->resources & (1u << p->resource));
    s->resources &= ~(1u << p->resource);
    p->resource = 0;
}

/* failure == 0 makes the replacement block's acquisition fail (CD_NOMEM);
 * failure == k > 0 makes the k-th new clone fail (CD_FAILED).  Either way the
 * original block, count and live resources must be exactly as before. */
static void check_append_failure(unsigned failure)
{
    struct cd_state s = {0};
    struct cd_payload_vector v;
    s.owner = &v;
    CHECK(cd_payload_init(&v, (struct cd_payload_allocator){&s, take, release},
                          (struct cd_payload_ops){&s, clone, destroy}) ==
          CD_OK);
    struct cd_payload first = {4, 0};
    struct cd_payload input[4] = {{7, 0}, {8, 0}, {9, 0}, {10, 0}};
    CHECK(cd_payload_append(&v, &first, 1) == CD_OK && v.count == 1 &&
          s.blocks == 1);

    s.expected_count = 1;
    struct cd_payload *old = v.data;
    unsigned old_resources = s.resources;
    if (failure == 0)
    {
        s.fail = s.calls + 1;
    }
    else
    {
        s.clone_fail = s.clones + failure;
    }
    CHECK(cd_payload_append(&v, input, 4) == (failure ? CD_FAILED : CD_NOMEM) &&
          v.data == old && v.count == 1 && v.data[0].value == 4 &&
          s.blocks == 1 && s.resources == old_resources);

    s.fail = 0;
    s.clone_fail = 0;
    CHECK(cd_payload_append(&v, input, 4) == CD_OK && v.count == 5 &&
          s.blocks == 1 && v.data[0].value == 4 && v.data[4].value == 10);
    CHECK(cd_payload_destroy(&v) == CD_OK && !s.blocks && !s.resources &&
          cd_payload_destroy(&v) == CD_INVALID);
}

int main(void)
{
    for (unsigned failure = 0; failure <= 4; ++failure)
    {
        check_append_failure(failure);
    }
    puts(
        "CONTAINERS_PAYLOAD_PASS storage_then_initializer_prefix_failure_exact_cleanup");
    return 0;
}
