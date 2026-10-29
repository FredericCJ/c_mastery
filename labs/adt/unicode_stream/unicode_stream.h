#ifndef CMASTERY_ADU_H
#define CMASTERY_ADU_H
#include "owning_sum.h"

enum adu_status
{
    ADU_OK,
    ADU_INVALID,
    ADU_UTF8,
    ADU_RANGE,
    ADU_STATE,
    ADU_NOMEM
};

enum adu_phase
{
    ADU_OPEN,
    ADU_FINISHED,
    ADU_CANCELLED
};

struct adu_stream
{
    unsigned char pending[3];
    unsigned used;
    size_t bytes, scalars;
    enum adu_phase phase;
};

struct adu_summary
{
    size_t bytes, scalars;
};

struct adu_owner
{
    struct ado_value value;
    size_t scalars;
};

/* Live controls must be initialized and caller-exclusive. No chunk borrow
   survives feed. Refused feed leaves every observable stream field unchanged.
 */
void adu_init(struct adu_stream *);
enum adu_status adu_feed(struct adu_stream *, const unsigned char *, size_t);
enum adu_status adu_finish(struct adu_stream *, struct adu_summary *);
enum adu_status adu_cancel(struct adu_stream *);
/* Initialize only unowned owner storage; don't copy owners by assignment. */
void adu_owner_init(struct adu_owner *);
enum adu_status adu_owner_set(struct adu_owner *, const unsigned char *, size_t,
                              struct ado_ops);
enum adu_status adu_owner_view(const struct adu_owner *, struct ad_text *);
void adu_owner_drop(struct adu_owner *);
#endif
