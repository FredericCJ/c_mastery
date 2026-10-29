#ifndef ADT_EVOLUTION_TARGET_H
#define ADT_EVOLUTION_TARGET_H
#include "evolution_record.h"
/* Original intentionally flawed local target, synthetic counters only.
 * Caller supplies a successfully admitted live record and disjoint live key,
 * tag and service. This intentionally omits the higher-level policy, not C
 * storage preconditions; it is never an arbitrary-pointer input boundary. */
int ade_vulnerable(const struct ade_record *,const unsigned char key[32],const unsigned char tag[32],struct ade_service *);
#endif
