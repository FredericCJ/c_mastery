#ifndef AI_INSTRUMENT_H
#define AI_INSTRUMENT_H
struct ai_counts { unsigned decisions,transfers,slot_probes,links; };
extern struct ai_counts ai_ring_counts,ai_link_counts;
#define AI_CHECK(c,e) ((c).decisions++,!!(e))
#endif
