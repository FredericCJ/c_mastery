#ifndef CHECKPOINT_TRANSFER_HALFDUPLEX_H
#define CHECKPOINT_TRANSFER_HALFDUPLEX_H
#include "checkpoint_firmware.h"
typedef void (*cpx_direction_fn)(void *, int);

struct cpx_half_duplex
{
    struct cpf_tx tx;
    uint16_t turnaround, start;
    unsigned driving, holding;
    cpx_direction_fn direction;
    void *context;
};
enum cpf_status cpx_halfduplex_begin(struct cpx_half_duplex *,
                                     const struct cp_message *, uint16_t,
                                     cpx_direction_fn, void *);
enum cpf_status cpx_halfduplex_step(struct cpx_half_duplex *, uint16_t, int,
                                    int, cpf_write_fn, void *);
#endif
