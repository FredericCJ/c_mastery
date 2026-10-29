#ifndef CMASTERY_SYNC_DEPTH_PI_H
#define CMASTERY_SYNC_DEPTH_PI_H
#include "cmx.h"

/* All objects and task slots are live, stable and externally serialized.
 * This extension validates a registered wait graph, not arbitrary pointers. */
struct cdpi_stats
{
    unsigned vertices, edges, closure_probes, donated_pairs;
};
enum cmx_result cdpi_reprioritize(cmx_kernel *, unsigned task,
                                  unsigned new_base, struct cdpi_stats *);
#endif
