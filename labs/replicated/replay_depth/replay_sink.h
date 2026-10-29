#ifndef CMASTERY_REPLAY_SINK_H
#define CMASTERY_REPLAY_SINK_H
#include "durable.h"
#include "replay_depth.h"
#define RPR_EFFECT_SCHEMA UINT32_C(0x52505245)
/* Separate exclusive durable RplLog, matching timeline UUID and schema.
   This persists effect intent once per exact prefix identity; it cannot
   atomically commit an unrelated physical display/audio effect. */
int rpr_sink_apply(struct rpl_log *, struct rpr_effect,
                   unsigned *newly_applied);
#endif
