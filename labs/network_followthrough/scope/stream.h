#ifndef CMASTERY_NETWORK_STREAM_H
#define CMASTERY_NETWORK_STREAM_H
#include "com_http.h"
#include "scope.h"
/* At most budget input bytes. HTTP_DONE/ERROR preserves unconsumed leftovers;
 * caller owns explicit parser reinitialization and connection-close policy. */
int nw_http_pump(NwRx *, HttpParser *, size_t budget, size_t *consumed);
#endif
