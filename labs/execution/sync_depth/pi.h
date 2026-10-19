#ifndef CMASTERY_SYNC_DEPTH_PI_H
#define CMASTERY_SYNC_DEPTH_PI_H
#include "cmx.h"
/* All objects and task slots are live, stable and externally serialized.
 * This extension validates a registered wait graph, not arbitrary pointers. */
typedef struct {
  unsigned vertices, edges, closure_probes, donated_pairs;
} CDPIStats;
enum cmx_result cdpi_reprioritize(cmx_kernel *, unsigned task,
                                 unsigned new_base, CDPIStats *);
#endif
