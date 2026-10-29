#ifndef PT_STREAM_H
#define PT_STREAM_H
#include "checkpoint_depth.h"
#include <stdbool.h>

/* Explicit dependency: the caller supplies one completed checkpoint parser
 * object, built under its own profile-C contract. This layer owns no transport. */
struct pt_session
{
    struct cp_parser parser;
    struct cp_queue queue;
    uint64_t epoch;
    size_t rejects;
    bool connected;
};

void pt_session_init(struct pt_session *session);
enum cp_status pt_session_open(struct pt_session *session);
void pt_session_close(struct pt_session *session);
enum cp_status pt_session_feed(struct pt_session *session,
                               const unsigned char *bytes, size_t length,
                               size_t *used);
enum cp_status pt_session_take(struct pt_session *session,
                               struct cp_message *message);
#endif
