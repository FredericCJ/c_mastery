#include "can.h"
#ifndef CN_FAULT_MUTANT
#define CN_FAULT_MUTANT 0
#endif
static unsigned add(unsigned value, unsigned delta, unsigned maximum)
{
    return delta > maximum - value ? maximum : value + delta;
}

int cn_fault(struct cn_fault *s, const struct cn_event *e,
             struct cn_effect *out)
{
    if (!s || !e || !out || s->tec > 256 || s->rec > 255 || s->state > CN_OFF ||
        s->enabled > 1 || s->restart > 1 || s->idles > 128 ||
        s->recessive > 10 || e->kind > CN_RECOVERY_BIT || e->transmitter > 1 ||
        e->active_or_overload > 1 || e->ack_missing > 1 ||
        e->passive_flag_all_recessive > 1 ||
        e->prearbitration_stuff_exception > 1)
        return CN_INVALID;
    if ((s->state == CN_OFF) != (s->tec > 255) ||
        (s->state != CN_OFF &&
         ((s->tec > 127 || s->rec > 127) != (s->state == CN_PASSIVE))))
        return CN_INVALID;
    if ((e->kind == CN_RECEIVER_ERROR || e->kind == CN_FIRST_DOMINANT ||
         e->kind == CN_RECEIVE_SUCCESS) &&
        e->transmitter)
        return CN_INVALID;
    if ((e->kind == CN_ARBITRATION_LOSS || e->kind == CN_TRANSMITTER_FLAG ||
         e->kind == CN_TRANSMIT_SUCCESS) &&
        !e->transmitter)
        return CN_INVALID;
    if (e->kind == CN_RECOVERY_BIT && e->value > 1)
        return CN_INVALID;
    if (e->kind == CN_DOMINANT_RUN && e->value > 1024)
        return CN_INVALID;
    if (e->kind != CN_TRANSMITTER_FLAG &&
        (e->ack_missing || e->passive_flag_all_recessive ||
         e->prearbitration_stuff_exception))
        return CN_INVALID;
    if (e->kind != CN_DOMINANT_RUN && e->kind != CN_RECOVERY_BIT && e->value)
        return CN_INVALID;
    if (e->kind != CN_RECEIVER_ERROR && e->kind != CN_BIT_DURING_FLAG &&
        e->kind != CN_DOMINANT_RUN && e->active_or_overload)
        return CN_INVALID;
    if (e->kind == CN_BIT_DURING_FLAG && !e->active_or_overload)
        return CN_INVALID;
    if ((e->passive_flag_all_recessive && !e->ack_missing) ||
        (e->prearbitration_stuff_exception &&
         (e->ack_missing || e->passive_flag_all_recessive)))
        return CN_INVALID;
    if (s->state != CN_OFF && (s->restart || s->idles || s->recessive))
        return CN_INVALID;
    struct cn_fault f = *s;
    struct cn_effect effect = {s->tec, s->rec, s->state, 0, 0, 0, 0};
    if (!s->enabled)
    {
        if (s->state != CN_ACTIVE || s->tec || s->rec)
            return CN_INVALID;
        goto publish;
    }
    if (s->state == CN_OFF)
    {
        if (e->kind == CN_RESTART)
        {
            f.restart = 1;
            f.idles = f.recessive = 0;
        }
        else if (e->kind == CN_RECOVERY_BIT && s->restart)
        {
            if (e->value == 0)
                f.recessive = 0;
            else if (++f.recessive == 11)
            {
                f.recessive = 0;
                ++f.idles;
            }
            if (f.idles == 128)
            {
                f.tec = f.rec = 0;
                f.state = CN_ACTIVE;
                f.restart = 0;
                f.idles = 0;
            }
        }
        else
            return CN_BUSY;
        goto publish;
    }
    if (e->kind == CN_RESTART || e->kind == CN_RECOVERY_BIT)
        return CN_INVALID;
    switch (e->kind)
    {
        case CN_ARBITRATION_LOSS:
            if (CN_FAULT_MUTANT == 1)
                f.tec = add(f.tec, 8, 256);
            break;
        case CN_RECEIVER_ERROR:
            if (!e->active_or_overload)
            {
                f.rec = add(f.rec, 1, 255);
                effect.rules = 1;
            }
            break;
        case CN_FIRST_DOMINANT:
            f.rec = add(f.rec, 8, 255);
            effect.rules = 2;
            break;
        case CN_TRANSMITTER_FLAG:
            if (CN_FAULT_MUTANT == 2 ||
                !((s->state == CN_PASSIVE && e->ack_missing &&
                   e->passive_flag_all_recessive) ||
                  e->prearbitration_stuff_exception))
            {
                f.tec = add(f.tec, 8, 256);
                effect.rules = 4;
            }
            break;
        case CN_BIT_DURING_FLAG:
            if (e->transmitter)
            {
                f.tec = add(f.tec, 8, 256);
                effect.rules = 8;
            }
            else
            {
                f.rec = add(f.rec, 8, 255);
                effect.rules = 16;
            }
            break;
        case CN_DOMINANT_RUN:
        {
            unsigned threshold = e->active_or_overload ? 14 : 8;
            unsigned increments =
                e->value < threshold ? 0 : 1 + (e->value - threshold) / 8;
            if (increments)
            {
                effect.rules = 32;
                if (e->transmitter)
                    f.tec = add(f.tec, 8 * increments, 256);
                else
                    f.rec = add(f.rec, 8 * increments, 255);
            }
            break;
        }
        case CN_TRANSMIT_SUCCESS:
            if (f.tec)
                --f.tec;
            effect.rules = 64;
            break;
        case CN_RECEIVE_SUCCESS:
            if (f.rec > 127)
                f.rec = CN_FAULT_MUTANT == 3 ? 0u : 127u;
            else if (f.rec)
                --f.rec;
            effect.rules = 128;
            break;
        default:
            return CN_INVALID;
    }
    f.state = f.tec > 255                    ? CN_OFF
              : (f.tec > 127 || f.rec > 127) ? CN_PASSIVE
                                             : CN_ACTIVE;
publish:
    effect.after_tec = f.tec;
    effect.after_rec = f.rec;
    effect.after_state = f.state;
    *s = f;
    *out = effect;
    return CN_OK;
}
