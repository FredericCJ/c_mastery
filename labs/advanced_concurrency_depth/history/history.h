#ifndef AH_HISTORY_H
#define AH_HISTORY_H
#include <stddef.h>
#include <stdint.h>
#define AH_PENDING UINT32_MAX
enum { AH_FIFO, AH_SET };
enum { AH_INSERT, AH_REMOVE };
enum { AH_VALUE, AH_FULL, AH_EMPTY, AH_EXISTS, AH_ABSENT };
enum { AH_VALID, AH_NONLINEAR, AH_INCOMPLETE, AH_INVALID, AH_UNIMPLEMENTED };
enum { AH_COMPLETE_ONLY, AH_EXTEND_PENDING };
typedef struct {
  uint32_t invoke, respond;
  unsigned participant, operation, value, result;
} AHOperation;
typedef struct {
  unsigned length, completed_pending, order[7], result[7], value[7];
} AHWitness;
/* One object, <=7 operations,1..4 capacity, initially empty. Unique event
 * ordinals except AH_PENDING; each participant has nonoverlapping calls. */
int ah_history(const AHOperation *, size_t, unsigned capacity, unsigned spec,
               unsigned pending_policy, AHWitness *);
/* Count an attempted bounded operation, not a proof of service or success. */
int ah_retry(unsigned attempts, unsigned budget, unsigned changed);
#endif
