#ifndef B02_FACADE_H
#define B02_FACADE_H
#define B2_IDENTITY(x) _Generic((x), int: b2_identity_int, unsigned: b2_identity_unsigned)(x)
#ifdef B2_MUTATE_MACRO
#undef B2_IDENTITY
#define B2_IDENTITY(x) _Generic((x), int: b2_identity_int, unsigned: b2_identity_unsigned)((x)+1)
#endif
#define B2_STRING_RAW(x) #x
#define B2_STRING(x) B2_STRING_RAW(x)
#endif
