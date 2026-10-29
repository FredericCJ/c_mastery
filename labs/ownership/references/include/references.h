#ifndef CMASTERY_OWNERSHIP_REFERENCES_H
#define CMASTERY_OWNERSHIP_REFERENCES_H
#include "ownership.h"
struct orc_memory {void *context;void *(*take)(void *,size_t);void (*release)(void *,void *);};
struct orc_cycle_report {unsigned retained_values,retained_controls,destructions,reclamations,repair_edges,queued_releases;};
/* Two actual allocated control blocks and two typed payloads; output on success. */
enum ow_status orc_cycle(struct orc_memory,struct v4_allocator,int weak_back_edge,struct orc_cycle_report *);
#endif
