#ifndef LC_POLICY_H
#define LC_POLICY_H
#include <stddef.h>
#include <stdint.h>
struct lc_entity {int64_t lag;uint64_t virtual_deadline;};
/* Mathematical snapshot, no kernel slice-protection/group/placement state. */
int lc_select(const struct lc_entity *entities,size_t count,size_t *selected);
int lc_parameters(uint64_t runtime,uint64_t deadline,uint64_t period);
#endif
