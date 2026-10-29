#include "dma.h"
#ifndef PD_NOTICE_MUTANT
#define PD_NOTICE_MUTANT 0
#endif
int pd_notice(struct pd_notice *s, unsigned op, uint32_t epoch, uint32_t sequence,
              const struct pd_notice *message, struct pd_notice *out) {
  if (!s || op > PD_NOTICE_QUERY || !epoch || !sequence)
    return PD_INVALID;
  if (PD_NOTICE_MUTANT != 1 && epoch != s->epoch)
    return PD_STALE;
  if (op == PD_NOTICE_PUBLISH) {
    if (!message || message->epoch != epoch || message->sequence != sequence ||
        sequence <= s->sequence || message->state != 1 ||
        (message->status != PD_OK && message->status != PD_CANCELLED &&
         message->status != PD_IO) ||
        message->complete > message->possible ||
        message->possible > message->extent || !message->extent ||
        message->extent > 4096 ||
        (message->destination | message->extent) & 31u ||
        message->destination < 0x30040040u ||
        message->destination >= 0x30048000u ||
        message->extent > 0x30048000u - message->destination)
      return PD_INVALID;
    if (PD_NOTICE_MUTANT != 2 && s->state != 0 && s->state != 3)
      return PD_BUSY;
    *s = *message;
    return PD_OK;
  }
  if (sequence != s->sequence)
    return PD_STALE;
  if (op == PD_NOTICE_QUERY)
    return s->state == 3 ? PD_OK : PD_BUSY;
  if (op == PD_NOTICE_TAKE) {
    if (!out)
      return PD_INVALID;
    if (s->state != 1)
      return PD_BUSY;
    *out = *s;
    s->state = PD_NOTICE_MUTANT == 3 ? 3 : 2;
    return PD_OK;
  }
  if (s->state != 2)
    return PD_BUSY;
  s->state = 3;
  return PD_OK;
}
