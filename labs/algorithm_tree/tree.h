#ifndef CMASTERY_ALGORITHM_TREE_H
#define CMASTERY_ALGORITHM_TREE_H
#include "algo.h"
#include "containers.h"
typedef struct {int32_t key;int left,right,parent,height;unsigned live,red,generation;} ATNode;
typedef struct {ATNode nodes[64];int root;unsigned capacity,count;} ATCertificate;
typedef struct {unsigned search_nodes,key_predicates,allocation_checks,rotations,height_updates,balances,repair_count;int repair_roots[64];} ATWork;
typedef struct {algo_avl tree;unsigned free_slots[64],free_count,generation[64],allocation_mode;} ATPool;
enum {AT_DUPLICATE=6};
extern ATWork *at_work;
extern ATPool *at_pool;
void at_observed_init(algo_avl*);
int at_observed_insert(algo_avl*,int32_t);
int at_observed_delete(algo_avl*,int32_t);
int at_observed_contains(const algo_avl*,int32_t);
void at_free_init(algo_avl*);
int at_free_insert(algo_avl*,int32_t);
int at_free_delete(algo_avl*,int32_t);
int at_free_contains(const algo_avl*,int32_t);
void at_init(ATPool*);
int at_insert(ATPool*,int32_t,unsigned free_list,ATWork*);
int at_delete(ATPool*,int32_t,unsigned free_list,ATWork*);
int at_contains(ATPool*,int32_t,unsigned free_list,ATWork*);
int at_certificate(const ATPool*,ATCertificate*);
int at_cd_certificate(const CDTree*,ATCertificate*);
int at_check(const ATCertificate*,unsigned policy,int*height);
int at_test(unsigned);
int at_failure(const char*,unsigned);
void at_emit(const char*,const ATCertificate*,const ATWork*);
#endif
