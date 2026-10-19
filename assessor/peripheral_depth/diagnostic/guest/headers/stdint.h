#ifndef DG_KERNEL_STDINT_H
#define DG_KERNEL_STDINT_H
#include <linux/limits.h>
#include <linux/types.h>
typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;
typedef s32 int32_t;
#define UINT32_C(v) v##U
#define UINT32_MAX U32_MAX
#define INT32_MAX S32_MAX
#define INT32_MIN S32_MIN
#endif
