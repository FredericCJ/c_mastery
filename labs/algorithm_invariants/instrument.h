#ifndef AI_INSTRUMENT_H
#define AI_INSTRUMENT_H
typedef struct { unsigned decisions,transfers,slot_probes,links; } AICounts;
extern AICounts ai_ring_counts,ai_link_counts;
#define AI_CHECK(c,e) ((c).decisions++,!!(e))
#endif
