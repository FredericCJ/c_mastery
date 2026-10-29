#ifndef CORPUS_EVENTS_H
#define CORPUS_EVENTS_H
#include <stddef.h>
#include <stdint.h>
#ifndef EV_TEXT
#define EV_TEXT 16
#endif
#define EV_DEPTH 8
#define EV_NAME 12
#define EV_DATA ((EV_TEXT) > EV_NAME ? (EV_TEXT) : EV_NAME)
_Static_assert(EV_TEXT >= 1 && EV_TEXT <= 64, "bounded text chunk");

enum ev_status
{
    EV_OK,
    EV_PAUSED,
    EV_DONE,
    EV_INVALID,
    EV_SYNTAX,
    EV_LIMIT,
    EV_CALLBACK,
    EV_CANCELLED,
    EV_BUSY
};

enum ev_kind
{
    EV_START = 1,
    EV_CHARACTERS,
    EV_END,
    EV_DOCUMENT
};

struct ev_event
{
    enum ev_kind kind;
    uint32_t sequence;
    size_t depth, length;
    unsigned char data[EV_DATA];
};

enum ev_action
{
    EV_ACCEPT,
    EV_WAIT,
    EV_REJECT
};
typedef enum ev_action (*ev_handler_fn)(void *, const struct ev_event *);

/* Live noncopied, externally serialized parser; callback borrow ends on return.
   CONTRACTS.md defines exact prefix, retry and terminal behavior. */
struct ev_parser
{
    unsigned char names[EV_DEPTH][EV_NAME], name[EV_NAME], text[EV_TEXT],
        entity[5];
    size_t lengths[EV_DEPTH], depth, limit, name_used, text_used, entity_used;
    unsigned state, root_seen, root_closed, pending, empty_end, busy;
    uint32_t sequence;
    enum ev_status terminal;
    struct ev_event event;
};
enum ev_status ev_init(struct ev_parser *, size_t depth_limit);
enum ev_status ev_feed(struct ev_parser *, const unsigned char *, size_t,
                       ev_handler_fn, void *, size_t *consumed);
enum ev_status ev_finish(struct ev_parser *, ev_handler_fn, void *);
enum ev_status ev_cancel(struct ev_parser *);

/* Two copied events: WAIT commits nothing; pop preserves output when empty. */
struct ev_queue
{
    struct ev_event events[2];
    size_t head, count;
};

void evq_init(struct ev_queue *);
enum ev_action evq_offer(void *, const struct ev_event *);
enum ev_status evq_pop(struct ev_queue *, struct ev_event *);

/* Separate semantic consumer: <sum> contains <g>/<n>; n is a nonempty decimal
   uint32 leaf; ASCII whitespace elsewhere. Failed event preserves all state. */
struct ev_sum
{
    unsigned roles[EV_DEPTH];
    size_t depth;
    uint32_t sequence, value;
    uint64_t sum;
    unsigned digits, seen, closed, done;
};

void evs_init(struct ev_sum *);
enum ev_status evs_event(struct ev_sum *, const struct ev_event *);
#endif
