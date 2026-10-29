#ifndef LP_MIGRATION_H
#define LP_MIGRATION_H
#include <stddef.h>
struct lp_record {int key;unsigned serial;};
struct lp_order {unsigned reverse;};
enum {LP_OK,LP_INVALID,LP_RANGE};
int lp_sum(int,int,int *);
int lp_compare(const struct lp_record *,const struct lp_record *);
int lp_adapter(const void *,const void *);
int lp_context_adapter(const void *,const void *,void *);
int lp_sort(struct lp_record *,size_t,const struct lp_order *);
#endif
