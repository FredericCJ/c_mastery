#ifndef CORPUS_MODEL_H
#define CORPUS_MODEL_H
#include <stddef.h>
#include <stdint.h>

enum
{
    CM_OK = 0,
    CM_INVALID = 1,
    CM_SYNTAX = 2,
    CM_RANGE = 3,
    CM_MISSING = 4,
    CM_CYCLE = 5,
    CM_CONFLICT = 6,
    CM_BUSY = 7,
    CM_PENDING = 8,
    CM_IO = 9,
    CM_CLOSED = 10
};

enum
{
    CM_ABSENT = 0,
    CM_INPUT = 1,
    CM_LINEAR = 2,
    CM_NODES = 16
};

struct cm_node
{
    unsigned kind, a, b;
    int32_t w0, w1, bias, value;
};

struct cm_model
{
    struct cm_node nodes[CM_NODES];
    uint32_t revision;
};

struct cm_edit
{
    unsigned id, kind, a, b;
    int32_t w0, w1, bias;
};

/* Live initialized objects, correctly typed and disjoint spans/control/output
 * storage, and one serialized owner; see CONTRACTS.md. */
void cm_init(struct cm_model *model);
int cm_valid(const struct cm_model *model);
int cm_apply(struct cm_model *model, const struct cm_edit *edits, size_t count);
int cm_load(struct cm_model *model, const unsigned char *text, size_t length,
            unsigned csv);
int cm_get(const struct cm_model *model, unsigned id, int32_t *value);
int cm_eval(const struct cm_model *model, const unsigned char *text,
            size_t length, int32_t *value);

struct cm_session
{
    struct cm_model model;
    unsigned char line[256];
    size_t used;
    unsigned discarding, closed;
    char reply[96];
    size_t reply_length, sent;
};

typedef size_t (*cm_write_fn)(void *context, const char *bytes, size_t length);
void cm_session_init(struct cm_session *session);
/* step executes one complete line only if no reply is pending. Command
 * failures are queued ERR replies, not API failures. */
int cm_step(struct cm_session *session, const unsigned char *line,
            size_t length);
int cm_feed(struct cm_session *session, const unsigned char *bytes,
            size_t length, size_t *consumed);
int cm_finish(struct cm_session *session);
int cm_drain(struct cm_session *session, cm_write_fn writer, void *context);
#endif
