#ifndef CMASTERY_OWNERSHIP_REFERENCES_H
#define CMASTERY_OWNERSHIP_REFERENCES_H
#include "ownership.h"
typedef struct {void *context;void *(*take)(void *,size_t);void (*release)(void *,void *);} OrcMemory;
typedef struct {unsigned retained_values,retained_controls,destructions,reclamations,repair_edges,queued_releases;} OrcCycleReport;
/* Two actual allocated control blocks and two typed payloads; output on success. */
OwStatus orc_cycle(OrcMemory,V4Allocator,int weak_back_edge,OrcCycleReport *);
#endif
