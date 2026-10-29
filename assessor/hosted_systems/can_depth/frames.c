#include "frames.h"
#include <linux/can/error.h>
#include <string.h>
#include <sys/socket.h>

static int valid(const struct ca_frame *f)
{
    if (!f || f->extended > 1 || f->length > 8 || f->local > 1 || f->own > 1)
    {
        return 0;
    }
    if (f->raw_dlc && (f->length != 8 || f->raw_dlc < 9 || f->raw_dlc > 15))
    {
        return 0;
    }
    if (f->kind == CA_ERROR)
    {
        return !f->extended && f->length == CAN_ERR_DLC && !f->raw_dlc &&
               !(f->id & ~CAN_ERR_MASK);
    }
    if (f->kind != CA_DATA && f->kind != CA_REMOTE)
    {
        return 0;
    }
    return !(f->id & ~(f->extended ? CAN_EFF_MASK : CAN_SFF_MASK));
}

int ca_decode(const void *bytes, size_t length, int flags, struct ca_frame *out)
{
    if (!bytes || !out || length != CAN_MTU ||
        (flags & (MSG_TRUNC | MSG_CTRUNC)))
    {
        return -1;
    }
    struct can_frame raw;
    /* The raw address may be unaligned; no typed overlay or aliasing assumption.
     */
    memcpy(&raw, bytes, sizeof raw);
    struct ca_frame f = {0};
    f.length = raw.len;
    f.raw_dlc = raw.len8_dlc;
    f.extended = !!(raw.can_id & CAN_EFF_FLAG);
    f.local = !!(flags & MSG_DONTROUTE);
    f.own = !!(flags & MSG_CONFIRM);
    if (raw.can_id & CAN_ERR_FLAG)
    {
        if (raw.can_id & (CAN_EFF_FLAG | CAN_RTR_FLAG))
        {
            return -1;
        }
        f.kind = CA_ERROR;
        f.id = raw.can_id & CAN_ERR_MASK;
    }
    else
    {
        f.kind = (raw.can_id & CAN_RTR_FLAG) ? CA_REMOTE : CA_DATA;
        f.id = raw.can_id & CAN_EFF_MASK;
    }
#ifdef CA_MUTANT_LENGTH
    if (f.length > 8)
    {
        f.length = 8;
    }
#endif
    if (!valid(&f))
    {
        return -1;
    }
    if (f.kind != CA_REMOTE)
    {
        memcpy(f.data, raw.data, f.length);
    }
    *out = f;
    return 0;
}

int ca_encode(const struct ca_frame *f, struct can_frame *out)
{
    if (!out || !valid(f) || f->kind == CA_ERROR)
    {
        return -1;
    }
    struct can_frame raw;
    memset(&raw, 0, sizeof raw);
    raw.can_id = f->id | (f->extended ? CAN_EFF_FLAG : 0) |
                 (f->kind == CA_REMOTE ? CAN_RTR_FLAG : 0);
    raw.len = (unsigned char)f->length;
    raw.len8_dlc = (unsigned char)f->raw_dlc;
    if (f->kind == CA_DATA)
    {
        memcpy(raw.data, f->data, f->length);
    }
    *out = raw;
    return 0;
}

int ca_filter(uint32_t id, unsigned extended, unsigned remote,
              struct can_filter *out)
{
    if (!out || extended > 1 || remote > 1 ||
        (id & ~(extended ? CAN_EFF_MASK : CAN_SFF_MASK)))
    {
        return -1;
    }
    struct can_filter f = {
        id | (extended ? CAN_EFF_FLAG : 0) | (remote ? CAN_RTR_FLAG : 0),
        CAN_EFF_FLAG | CAN_RTR_FLAG | (extended ? CAN_EFF_MASK : CAN_SFF_MASK)};
#ifdef CA_MUTANT_FILTER
    f.can_mask &= ~CAN_EFF_FLAG;
#endif
    *out = f;
    return 0;
}

int ca_accept(const struct ca_frame *f, const struct can_filter *filter,
              uint32_t errors)
{
    if (!valid(f) || !filter || (errors & ~CAN_ERR_MASK))
    {
        return -1;
    }
    if (f->kind == CA_ERROR)
    {
#ifdef CA_MUTANT_ERROR
        return 1;
#else
        return !!(f->id & errors);
#endif
    }
    uint32_t key = f->id | (f->extended ? CAN_EFF_FLAG : 0) |
                   (f->kind == CA_REMOTE ? CAN_RTR_FLAG : 0);
    return (key & filter->can_mask) == (filter->can_id & filter->can_mask);
}

int ca_loopback(unsigned sender, unsigned receiver, unsigned enabled,
                unsigned own, const struct ca_frame *f,
                const struct can_filter *filter, uint32_t errors,
                struct ca_frame *out)
{
    if (sender > 1 || receiver > 1 || enabled > 1 || own > 1 || !out)
    {
        return -1;
    }
    int accept = ca_accept(f, filter, errors);
    if (accept < 0)
    {
        return -1;
    }
    if (!enabled || !accept)
    {
        return 0;
    }
#ifndef CA_MUTANT_OWN
    if (sender == receiver && !own)
    {
        return 0;
    }
#endif
    *out = *f;
    out->local = 1;
    out->own = sender == receiver;
    return 1;
}
