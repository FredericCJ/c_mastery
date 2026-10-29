#ifndef CMASTERY_CAN_DEPTH_H
#define CMASTERY_CAN_DEPTH_H
#include <linux/can.h>
#include <stddef.h>
#include <stdint.h>

enum ca_kind
{
    CA_DATA,
    CA_REMOTE,
    CA_ERROR
};

struct ca_frame
{
    enum ca_kind kind;
    uint32_t id;
    unsigned extended, length, raw_dlc, local, own;
    unsigned char data[8];
};

/* Output unchanged on rejection. Classical CAN only; no socket is opened. */
int ca_decode(const void *, size_t, int message_flags, struct ca_frame *);
int ca_encode(const struct ca_frame *, struct can_frame *);
int ca_filter(uint32_t id, unsigned extended, unsigned remote,
              struct can_filter *);
int ca_accept(const struct ca_frame *, const struct can_filter *,
              uint32_t error_mask);
/* Explicit local-bus MODEL: no arbitration, controller or kernel execution. */
int ca_loopback(unsigned sender, unsigned receiver, unsigned enabled,
                unsigned receive_own, const struct ca_frame *,
                const struct can_filter *, uint32_t error_mask,
                struct ca_frame *delivered);
#endif
