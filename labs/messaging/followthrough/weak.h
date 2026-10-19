#ifndef CMASTERY_MESSAGING_WEAK_H
#define CMASTERY_MESSAGING_WEAK_H
#include <stdatomic.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

typedef enum { MFW_OK, MFW_EMPTY, MFW_FULL, MFW_CONFLICT, MFW_OVERFLOW,
               MFW_BUSY, MFW_INVALID } MfwStatus;
typedef struct { atomic_size_t top, bottom; atomic_uint slots[8]; } MfwDeque;
typedef struct { atomic_uint sequence, first, second; } MfwSequence;
void mfw_deque_init(MfwDeque *);
MfwStatus mfw_push(MfwDeque *, unsigned);
MfwStatus mfw_pop(MfwDeque *, unsigned *);
MfwStatus mfw_steal(MfwDeque *, unsigned *);
void mfw_sequence_init(MfwSequence *);
MfwStatus mfw_write(MfwSequence *, unsigned, unsigned);
MfwStatus mfw_read(MfwSequence *, size_t, unsigned [2]);
#endif
