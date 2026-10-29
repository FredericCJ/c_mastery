#ifndef CMASTERY_ALGORITHM_TREE_H
#define CMASTERY_ALGORITHM_TREE_H
#include "algo.h"
#include "containers.h"

struct at_node
{
    int32_t key;
    int left, right, parent, height;
    unsigned live, red, generation;
};

struct at_certificate
{
    struct at_node nodes[64];
    int root;
    unsigned capacity, count;
};

struct at_work
{
    unsigned search_nodes, key_predicates, allocation_checks, rotations,
        height_updates, balances, repair_count;
    int repair_roots[64];
};

struct at_pool
{
    algo_avl tree;
    unsigned free_slots[64], free_count, generation[64], allocation_mode;
};

enum
{
    AT_DUPLICATE = 6
};
extern struct at_work *at_work;
extern struct at_pool *at_pool;
void at_observed_init(algo_avl *);
int at_observed_insert(algo_avl *, int32_t);
int at_observed_delete(algo_avl *, int32_t);
int at_observed_contains(const algo_avl *, int32_t);
void at_free_init(algo_avl *);
int at_free_insert(algo_avl *, int32_t);
int at_free_delete(algo_avl *, int32_t);
int at_free_contains(const algo_avl *, int32_t);
void at_init(struct at_pool *);
int at_insert(struct at_pool *, int32_t, unsigned free_list, struct at_work *);
int at_delete(struct at_pool *, int32_t, unsigned free_list, struct at_work *);
int at_contains(struct at_pool *, int32_t, unsigned free_list,
                struct at_work *);
int at_certificate(const struct at_pool *, struct at_certificate *);
int at_cd_certificate(const struct cd_tree *, struct at_certificate *);
int at_check(const struct at_certificate *, unsigned policy, int *height);
int at_test(unsigned);
int at_failure(const char *, unsigned);
void at_emit(const char *, const struct at_certificate *,
             const struct at_work *);
#endif
