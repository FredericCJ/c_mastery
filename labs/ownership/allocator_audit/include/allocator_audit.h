#ifndef CMASTERY_OWNERSHIP_ALLOCATOR_AUDIT_H
#define CMASTERY_OWNERSHIP_ALLOCATOR_AUDIT_H
#include "ownership.h"
#include "errors.h"

/* This extends the adapter, never the imported V4Allocator ABI. Resize preserves
 * the old allocation on NULL; success preserves min(old,new) elements and ends
 * the old allocation/borrow. Callbacks do not reenter the same audit. */
struct oaa_upstream
{
    struct v4_allocator base;
    int32_t *(*resize)(void *, int32_t *, size_t, size_t);
};

struct oaa_failure
{
    enum ow_status status;
    size_t attempt, live, live_cells;
};

typedef void (*oaa_reporter_fn)(void *, struct oaa_failure);

struct oaa_block
{
    int32_t *data;
    size_t count;
};

struct oaa_audit
{
    struct oaa_upstream upstream;
    struct oaa_block blocks[8];
    oaa_reporter_fn report;
    void *report_context;
    size_t attempts, fail_at, successes, resize_successes, releases,
        invalid_releases;
    size_t live, live_cells, peak_cells, upstream_calls, nested_attempts,
        reports;
    unsigned exhausted, busy, reporting;
};
/* Fresh nonowning control, stable externally serialized lifetime. All genuine
 * live backing supports int32_t; every extent is 1..16. Reporter runs after a
 * consistent failure state, may allocate recursively, and never reports itself.
 * It cannot mutate/release the failed resize's old owner or any other object
 * borrowed by its invoking call; callbacks preserve those caller lifetimes. */
enum ow_status oaa_init(struct oaa_audit *, struct oaa_upstream);
enum ow_status oaa_set_report(struct oaa_audit *, oaa_reporter_fn, void *);
struct v4_allocator oaa_allocator(struct oaa_audit *);
enum ow_status oaa_resize(struct oaa_audit *, int32_t *old, size_t,
                          int32_t **out);
struct oaa_upstream oaa_ow_upstream(struct ow_allocator *);
typedef void (*oaa_format_fn)(void *, er_result);
/* Actual T65 parser, then an optional ordinary callback on its error path. */
er_result oaa_parse_call(const unsigned char *, size_t, uint32_t *,
                         oaa_format_fn, void *);

struct oaa_formatting
{
    struct v4_allocator allocator;
    unsigned depth;
    char text[64];
    size_t length, calls, failures;
    enum ow_status last;
};
/* Bounded first-principles diagnostic consumer: depth1..3, error offset0..999.
 * Every recursive frame allocates16typed cells, then releases after its child.
 * Text publication is transactional; accounting observes failed attempts. */
enum ow_status oaa_format_result(struct oaa_formatting *, er_result);
void oaa_allocating_formatter(void *, er_result);
#endif
