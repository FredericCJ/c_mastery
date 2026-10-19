#ifndef CMASTERY_OWNERSHIP_ALLOCATOR_AUDIT_H
#define CMASTERY_OWNERSHIP_ALLOCATOR_AUDIT_H
#include "ownership.h"
#include "errors.h"
/* This extends the adapter, never the imported V4Allocator ABI. Resize preserves
 * the old allocation on NULL; success preserves min(old,new) elements and ends
 * the old allocation/borrow. Callbacks do not reenter the same audit. */
typedef struct {V4Allocator base;int32_t *(*resize)(void *,int32_t *,size_t,size_t);} OaaUpstream;
typedef struct {OwStatus status;size_t attempt,live,live_cells;} OaaFailure;
typedef void (*OaaReporter)(void *,OaaFailure);
typedef struct {int32_t *data;size_t count;} OaaBlock;
typedef struct {
 OaaUpstream upstream;OaaBlock blocks[8];OaaReporter report;void *report_context;
 size_t attempts,fail_at,successes,resize_successes,releases,invalid_releases;
 size_t live,live_cells,peak_cells,upstream_calls,nested_attempts,reports;
 unsigned exhausted,busy,reporting;
} OaaAudit;
/* Fresh nonowning control, stable externally serialized lifetime. All genuine
 * live backing supports int32_t; every extent is 1..16. Reporter runs after a
 * consistent failure state, may allocate recursively, and never reports itself.
 * It cannot mutate/release the failed resize's old owner or any other object
 * borrowed by its invoking call; callbacks preserve those caller lifetimes. */
OwStatus oaa_init(OaaAudit *,OaaUpstream);
OwStatus oaa_set_report(OaaAudit *,OaaReporter,void *);
V4Allocator oaa_allocator(OaaAudit *);
OwStatus oaa_resize(OaaAudit *,int32_t *old,size_t,int32_t **out);
OaaUpstream oaa_ow_upstream(OwAllocator *);
typedef void (*OaaFormat)(void *,er_result);
/* Actual T65 parser, then an optional ordinary callback on its error path. */
er_result oaa_parse_call(const unsigned char *,size_t,uint32_t *,OaaFormat,void *);
typedef struct {V4Allocator allocator;unsigned depth;char text[64];size_t length,calls,failures;OwStatus last;} OaaFormatting;
/* Bounded first-principles diagnostic consumer: depth1..3, error offset0..999.
 * Every recursive frame allocates16typed cells, then releases after its child.
 * Text publication is transactional; accounting observes failed attempts. */
OwStatus oaa_format_result(OaaFormatting *,er_result);
void oaa_allocating_formatter(void *,er_result);
#endif
