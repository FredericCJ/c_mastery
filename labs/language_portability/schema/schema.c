#include "schema.h"

int ls_pack(const struct ls_fields *a, uint16_t *b)
{
    (void)a;
    (void)b;
    return -1;
}

int ls_unpack(uint16_t a, struct ls_fields *b)
{
    (void)a;
    (void)b;
    return -1;
}

int ls_require(const char *a, const char *b)
{
    (void)a;
    (void)b;
    return -1;
}

const char *ls_wire_identity(void)
{
    return "unfinished";
}

const char *ls_abi_identity(void)
{
    return "unfinished";
}

const char *ls_build_identity(void)
{
    return "unfinished";
}

size_t ls_record_size(void)
{
    return 0;
}
