#ifndef CMASTERY_MPV_MACROS_H
#define CMASTERY_MPV_MACROS_H
#include "receivers.h"
/* Compiling unsolved forwarding: derive the complete typed selections. */
#define MPV_READ(p, out) mpv_read_cv((p), (out))
#define MPV_WRITE(p, allowed, value) mpv_write_v((p), (allowed), (value))
#define MPV_OBSERVE(p, between, context, out) \
    mpv_observe_cv((p), (between), (context), (out))
#endif
