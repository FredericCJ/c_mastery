#ifndef LP_MIGRATION_H
#define LP_MIGRATION_H
#include <stddef.h>
typedef struct {int key;unsigned serial;} LpRecord;
typedef struct {unsigned reverse;} LpOrder;
enum {LP_OK,LP_INVALID,LP_RANGE};
int lp_sum(int,int,int *);
int lp_compare(const LpRecord *,const LpRecord *);
int lp_adapter(const void *,const void *);
int lp_context_adapter(const void *,const void *,void *);
int lp_sort(LpRecord *,size_t,const LpOrder *);
#endif
