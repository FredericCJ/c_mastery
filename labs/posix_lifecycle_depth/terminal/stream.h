#ifndef PT_STREAM_H
#define PT_STREAM_H
#include "checkpoint_depth.h"
#include <stdbool.h>
/* Explicit dependency: the caller supplies one completed checkpoint parser
 * object, built under its own profile-C contract. This layer owns no transport. */
typedef struct { CPParser parser; CPQueue queue; uint64_t epoch; size_t rejects; bool connected; } PTSession;
void pt_session_init(PTSession *session);
CPStatus pt_session_open(PTSession *session);
void pt_session_close(PTSession *session);
CPStatus pt_session_feed(PTSession *session,const unsigned char *bytes,size_t length,size_t *used);
CPStatus pt_session_take(PTSession *session,CPMessage *message);
#endif
