#ifndef CMASTERY_MESSAGING_WEAK_H
#define CMASTERY_MESSAGING_WEAK_H
#include <stdatomic.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

enum mfw_status
{
    MFW_OK,
    MFW_EMPTY,
    MFW_FULL,
    MFW_CONFLICT,
    MFW_OVERFLOW,
    MFW_BUSY,
    MFW_INVALID
};

struct mfw_deque
{
    atomic_size_t top, bottom;
    atomic_uint slots[8];
};

struct mfw_sequence
{
    atomic_uint sequence, first, second;
};

void mfw_deque_init(struct mfw_deque *);
enum mfw_status mfw_push(struct mfw_deque *, unsigned);
enum mfw_status mfw_pop(struct mfw_deque *, unsigned *);
enum mfw_status mfw_steal(struct mfw_deque *, unsigned *);
void mfw_sequence_init(struct mfw_sequence *);
enum mfw_status mfw_write(struct mfw_sequence *, unsigned, unsigned);
enum mfw_status mfw_read(struct mfw_sequence *, size_t, unsigned[2]);
#endif
