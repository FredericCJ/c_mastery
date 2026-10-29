#ifndef CMASTERY_OWNED_COMPOSITION_H
#define CMASTERY_OWNED_COMPOSITION_H
#include "owning_option.h"

enum
{
    ERC_OK,
    ERC_INVALID,
    ERC_BUSY,
    ERC_NOMEM,
    ERC_FULL,
    ERC_NOT_IMPLEMENTED
};

enum
{
    ERC_EMPTY,
    ERC_VALUE,
    ERC_ERROR
};

struct erc_cause
{
    uint32_t domain, code;
    size_t offset;
};

struct erc_error
{
    unsigned count;
    struct erc_cause cause[4];
};

struct erc_result
{
    unsigned tag;

    union
    {
        struct eop_owner owner;
        struct erc_error error;
    } as;
};

typedef int32_t (*erc_map_fn)(int32_t, void *);
/* All controls live, disjoint, serialized, noncopied while owning; see COMPOSITION.md. */
int erc_cause(struct erc_cause root, struct erc_error *out);
int erc_wrap(const struct erc_error *inner, struct erc_cause outer,
             struct erc_error *out);
int erc_error(const struct erc_error *error, struct erc_result *empty);
int erc_adopt(struct eop_owner *source, struct erc_result *empty);
int erc_map_clone(const struct erc_result *source, erc_map_fn fn, void *context,
                  struct erc_result *empty);
int erc_drop(struct erc_result *result);
#endif
