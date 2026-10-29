#include "schema.h"
#include "generated/config.h"
#include "generated/codec.h"
#ifndef LS_MUTANT
#define LS_MUTANT 0
#endif
static int equal(const char *a, const char *b)
{
    if (!a || !b)
    {
        return 0;
    }
    while (*a && *a == *b)
    {
        ++a;
        ++b;
    }
    return *a == *b;
}

int ls_pack(const struct ls_fields *v, uint16_t *out)
{
#if LS_MUTANT == 3
    if (out)
    {
        *out = 0;
    }
#endif
#if LS_MUTANT == 1
    if (v && out && v->tag < 8 && v->length < 32 && v->flags < 4)
    {
        *out = (uint16_t)((v->tag << 1) | (v->length << 4) | (v->flags << 9));
        return 0;
    }
#endif
    return lsg_pack(v, out);
}

int ls_unpack(uint16_t w, struct ls_fields *out)
{
#if LS_MUTANT == 1
    if (out && !(w & UINT16_C(0xf801)))
    {
        out->tag = (w >> 1) & 7;
        out->length = (w >> 4) & 31;
        out->flags = (w >> 9) & 3;
        return 0;
    }
#endif
    return lsg_unpack(w, out);
}

int ls_require(const char *wire, const char *abi)
{
#if LS_MUTANT == 2
    (void)wire;
    return equal(abi, LS_ABI_ID) ? 0 : -1;
#else
    return equal(wire, LS_WIRE_ID) && equal(abi, LS_ABI_ID) ? 0 : -1;
#endif
}

const char *ls_wire_identity(void)
{
    return LS_WIRE_ID;
}

const char *ls_abi_identity(void)
{
    return LS_ABI_ID;
}

const char *ls_build_identity(void)
{
    return LS_BUILD_ID;
}

size_t ls_record_size(void)
{
    return sizeof(LsApiRecord);
}
