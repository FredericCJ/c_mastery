#ifndef CMASTERY_ADU_H
#define CMASTERY_ADU_H
#include "owning_sum.h"
typedef enum {
  ADU_OK,
  ADU_INVALID,
  ADU_UTF8,
  ADU_RANGE,
  ADU_STATE,
  ADU_NOMEM
} AduStatus;
typedef enum { ADU_OPEN, ADU_FINISHED, ADU_CANCELLED } AduPhase;
typedef struct {
  unsigned char pending[3];
  unsigned used;
  size_t bytes, scalars;
  AduPhase phase;
} AduStream;
typedef struct {
  size_t bytes, scalars;
} AduSummary;
typedef struct {
  AdoValue value;
  size_t scalars;
} AduOwner;
/* Live controls must be initialized and caller-exclusive. No chunk borrow
   survives feed. Refused feed leaves every observable stream field unchanged.
 */
void adu_init(AduStream *);
AduStatus adu_feed(AduStream *, const unsigned char *, size_t);
AduStatus adu_finish(AduStream *, AduSummary *);
AduStatus adu_cancel(AduStream *);
/* Initialize only unowned owner storage; don't copy owners by assignment. */
void adu_owner_init(AduOwner *);
AduStatus adu_owner_set(AduOwner *, const unsigned char *, size_t, AdoOps);
AduStatus adu_owner_view(const AduOwner *, ADText *);
void adu_owner_drop(AduOwner *);
#endif
