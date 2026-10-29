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
typedef enum { EV_OK, EV_PAUSED, EV_DONE, EV_INVALID, EV_SYNTAX,
               EV_LIMIT, EV_CALLBACK, EV_CANCELLED, EV_BUSY } EvStatus;
typedef enum { EV_START=1, EV_CHARACTERS, EV_END, EV_DOCUMENT } EvKind;
typedef struct { EvKind kind; uint32_t sequence; size_t depth,length;
                 unsigned char data[EV_DATA]; } EvEvent;
typedef enum { EV_ACCEPT, EV_WAIT, EV_REJECT } EvAction;
typedef EvAction (*EvHandler)(void *, const EvEvent *);
/* Live noncopied, externally serialized parser; callback borrow ends on return.
   CONTRACTS.md defines exact prefix, retry and terminal behavior. */
typedef struct {
    unsigned char names[EV_DEPTH][EV_NAME], name[EV_NAME], text[EV_TEXT], entity[5];
    size_t lengths[EV_DEPTH], depth,limit,name_used,text_used,entity_used;
    unsigned state,root_seen,root_closed,pending,empty_end,busy;
    uint32_t sequence;
    EvStatus terminal;
    EvEvent event;
} EvParser;
EvStatus ev_init(EvParser *,size_t depth_limit);
EvStatus ev_feed(EvParser *,const unsigned char *,size_t,EvHandler,void *,size_t *consumed);
EvStatus ev_finish(EvParser *,EvHandler,void *);
EvStatus ev_cancel(EvParser *);
/* Two copied events: WAIT commits nothing; pop preserves output when empty. */
typedef struct { EvEvent events[2]; size_t head,count; } EvQueue;
void evq_init(EvQueue *);
EvAction evq_offer(void *,const EvEvent *);
EvStatus evq_pop(EvQueue *,EvEvent *);
/* Separate semantic consumer: <sum> contains <g>/<n>; n is a nonempty decimal
   uint32 leaf; ASCII whitespace elsewhere. Failed event preserves all state. */
typedef struct { unsigned roles[EV_DEPTH];size_t depth;uint32_t sequence,value;
    uint64_t sum;unsigned digits,seen,closed,done; } EvSum;
void evs_init(EvSum *);
EvStatus evs_event(EvSum *,const EvEvent *);
#endif
