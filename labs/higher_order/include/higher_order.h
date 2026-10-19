#ifndef CMASTERY_HIGHER_ORDER_H
#define CMASTERY_HIGHER_ORDER_H
#include <stddef.h>
#include <stdint.h>
enum HOStatus {HO_OK,HO_INVALID,HO_FULL,HO_RANGE,HO_INPUT,HO_FAILED};
enum HOKind {HO_PURE,HO_EMIT,HO_CHOOSE};
typedef struct {enum HOKind kind;int32_t value;unsigned left,right;} HONode;
typedef struct {HONode nodes[65];unsigned used;size_t attempts,fail_at;} HOA;
typedef int (*HOBind)(HOA *,int32_t,void *,unsigned *);
int ho_pure(HOA *,int32_t,unsigned *);
int ho_emit(HOA *,int32_t,unsigned,unsigned *);
int ho_choose(HOA *,unsigned,unsigned,unsigned *);
int ho_bind(HOA *,unsigned,HOBind,void *,unsigned *);
typedef struct {int32_t value,trace[32];size_t consumed,count;} HOResult;
int ho_run(const HOA *,unsigned,const unsigned char *,size_t,HOResult *);
typedef struct {uint64_t paths,emissions;int64_t sum;} HOSummary;
int ho_summarize(const HOA *,unsigned,HOSummary *);
/* The two carriers are unrelated C types, not a runtime union/tagged value. */
typedef int32_t HEInt;
typedef int (*HEBody)(void *,HEInt,HEInt *);
typedef struct {HEBody body;void *context;} HEFun;
typedef struct {char data[128];size_t length;} HPInt;
typedef struct {char data[128];size_t length;} HPFun;
typedef int (*HPBody)(void *,HPInt,HPInt *);
typedef struct {int (*lit)(int32_t,HEInt *);int (*add)(HEInt,HEInt,HEInt *);int (*lam)(HEBody,void *,HEFun *);int (*app)(HEFun,HEInt,HEInt *);} HEOps;
typedef struct {int (*lit)(int32_t,HPInt *);int (*add)(HPInt,HPInt,HPInt *);int (*lam)(HPBody,void *,HPFun *);int (*app)(HPFun,HPInt,HPInt *);} HPOps;
HEOps ho_eval_ops(void);
HPOps ho_print_ops(void);
#endif
