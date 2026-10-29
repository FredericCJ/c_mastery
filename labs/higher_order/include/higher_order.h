#ifndef CMASTERY_HIGHER_ORDER_H
#define CMASTERY_HIGHER_ORDER_H
#include <stddef.h>
#include <stdint.h>
enum ho_status {HO_OK,HO_INVALID,HO_FULL,HO_RANGE,HO_INPUT,HO_FAILED};
enum ho_kind {HO_PURE,HO_EMIT,HO_CHOOSE};
struct ho_node {enum ho_kind kind;int32_t value;unsigned left,right;};
struct ho_hoa {struct ho_node nodes[65];unsigned used;size_t attempts,fail_at;};
typedef int (*ho_bind_fn)(struct ho_hoa *,int32_t,void *,unsigned *);
int ho_pure(struct ho_hoa *,int32_t,unsigned *);
int ho_emit(struct ho_hoa *,int32_t,unsigned,unsigned *);
int ho_choose(struct ho_hoa *,unsigned,unsigned,unsigned *);
int ho_bind(struct ho_hoa *,unsigned,ho_bind_fn,void *,unsigned *);
struct ho_result {int32_t value,trace[32];size_t consumed,count;};
int ho_run(const struct ho_hoa *,unsigned,const unsigned char *,size_t,struct ho_result *);
struct ho_summary {uint64_t paths,emissions;int64_t sum;};
int ho_summarize(const struct ho_hoa *,unsigned,struct ho_summary *);
/* The two carriers are unrelated C types, not a runtime union/tagged value. */
typedef int32_t he_int;
typedef int (*he_body_fn)(void *,he_int,he_int *);
struct he_fun {he_body_fn body;void *context;};
struct hp_int {char data[128];size_t length;};
struct hp_fun {char data[128];size_t length;};
typedef int (*hp_body_fn)(void *,struct hp_int,struct hp_int *);
struct he_ops {int (*lit)(int32_t,he_int *);int (*add)(he_int,he_int,he_int *);int (*lam)(he_body_fn,void *,struct he_fun *);int (*app)(struct he_fun,he_int,he_int *);};
struct hp_ops {int (*lit)(int32_t,struct hp_int *);int (*add)(struct hp_int,struct hp_int,struct hp_int *);int (*lam)(hp_body_fn,void *,struct hp_fun *);int (*app)(struct hp_fun,struct hp_int,struct hp_int *);};
struct he_ops ho_eval_ops(void);
struct hp_ops ho_print_ops(void);
#endif
