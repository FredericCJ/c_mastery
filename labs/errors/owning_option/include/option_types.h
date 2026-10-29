#ifndef CMASTERY_OPTION_TYPES_H
#define CMASTERY_OPTION_TYPES_H
#include <stdint.h>
#include "imports/v4_algo.h"

enum
{
    EOP_OK,
    EOP_INVALID,
    EOP_ABSENT,
    EOP_BUSY,
    EOP_NOMEM
};

enum
{
    EOP_NONE,
    EOP_SOME
};

struct eop_scalar
{
    unsigned tag;
    uint32_t value;
};

struct eop_owner
{
    unsigned tag;

    union
    {
        struct
        {
            uint32_t id;
            int32_t *cell;
            struct v4_allocator allocator;
        } some;
    } as;
};

enum eop_unit
{
    EOP_UNIT = 0
};
#endif
