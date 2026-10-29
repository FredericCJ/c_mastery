#include "scope.h"
#include "fractional.h"
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(CHAR_BIT == 8, "wire contract uses octets");
#define CHECK(c)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(c))                                                             \
        {                                                                     \
            fprintf(stderr, "RZ_CHECK: %s:%d: %s\n", __FILE__, __LINE__, #c); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

struct rz_allocation
{
    size_t allocations, releases, attempts, fail;
};

static void *allocate(void *p, size_t n)
{
    struct rz_allocation *a = p;
    if (a->attempts++ == a->fail)
    {
        return NULL;
    }
    void *result = malloc(n);
    if (result)
    {
        ++a->allocations;
    }
    return result;
}

static void deallocate(void *p, void *value)
{
    ++((struct rz_allocation *)p)->releases;
    free(value);
}

static struct rzf_allocator allocator(struct rz_allocation *a)
{
    return (struct rzf_allocator){a, allocate, deallocate};
}

struct rz_pause
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    unsigned started;
    bool run;
};

struct rz_worker
{
    struct rz_pause *pause;
    struct rzf_borrow borrow;
    int result;
};

static void *borrower(void *p)
{
    struct rz_worker *w = p;
    CHECK(!pthread_mutex_lock(&w->pause->mutex));
    ++w->pause->started;
    CHECK(!pthread_cond_broadcast(&w->pause->condition));
    while (!w->pause->run)
    {
        CHECK(!pthread_cond_wait(&w->pause->condition, &w->pause->mutex));
    }
    CHECK(!pthread_mutex_unlock(&w->pause->mutex));
    w->result = rz_check(&w->borrow.scope->core, 9);
    return NULL;
}

static void scope_test(void)
{
    struct rz_allocation a = {0, 0, 0, SIZE_MAX};
    struct rzf_scope *parent = NULL, *child = NULL;
    CHECK(rzf_scope_create(NULL, 10, 10, allocator(&a), &parent) == RZ_OK);
    CHECK(rzf_scope_create(parent, 13, 100, allocator(&a), &child) == RZ_OK);
    CHECK(child->core.deadline == 20 && atomic_load(&parent->references) == 2);
    struct rzf_budget budget = {99, 99};
    CHECK(rzf_budget_export(child, 14, 2, &budget) == RZ_OK);
    CHECK(budget.remaining == 6 && budget.transit_bound == 2);
    unsigned char wire[16];
    CHECK(rzf_budget_encode(&budget, wire) == RZ_OK);
    CHECK(wire[7] == 6 && wire[15] == 2 && wire[0] == 0);
    struct rzf_budget decoded = {91, 90};
    CHECK(rzf_budget_decode(wire, &decoded) == RZ_OK);
    struct rzf_scope *remote = NULL;
    CHECK(rzf_budget_import(&decoded, NULL, 1003, 1, allocator(&a), &remote) ==
          RZ_OK);
    CHECK(remote->core.deadline == 1006);
    /* Same-rate clocks: actual transit=2, processing=1 => six sender ticks total. */
    CHECK((remote->core.deadline - 1003) + 2 + 1 == budget.remaining);
    rzf_scope_release(&remote);
    CHECK(rzf_budget_import(&decoded, NULL, 0, 4, allocator(&a), &remote) ==
              RZ_EXPIRED &&
          !remote);
    CHECK(rzf_budget_import(&decoded, NULL, UINT64_MAX, 0, allocator(&a),
                            &remote) == RZ_RANGE &&
          !remote);
    memset(wire, 0, sizeof wire);
    CHECK(rzf_budget_decode(wire, &decoded) == RZ_INVALID &&
          decoded.remaining == 6);
    rzf_scope_release(&parent);
    CHECK(a.releases ==
          1); /* Only remote is released; child owns the parent. */
    CHECK(rz_check(&child->core, 19) == RZ_OK);
    rz_cancel(&child->parent->core);
    CHECK(rz_check(&child->core, 20) == RZ_CANCELLED);
    CHECK(rzf_budget_export(child, 19, 0, &budget) == RZ_CANCELLED &&
          budget.remaining == 6);
    rzf_scope_release(&child);
    CHECK(a.releases == a.allocations);

    for (size_t fail = 0; fail < 5; ++fail)
    {
        struct rz_allocation cuts = {0, 0, 0, fail};
        struct rzf_scope *children[3] = {NULL, NULL, NULL};
        struct rzf_scope *root = NULL;
        int status = rzf_scope_create(NULL, 0, 30, allocator(&cuts), &root);
        if (status == RZ_OK)
        {
            for (unsigned i = 0; i < 3; ++i)
            {
                if (rzf_scope_create(root, i, 20, allocator(&cuts),
                                     &children[i]) != RZ_OK)
                {
                    break;
                }
            }
        }
        rzf_scope_release(&root);
        for (unsigned i = 0; i < 3; ++i)
        {
            rzf_scope_release(&children[i]);
        }
        CHECK(cuts.allocations == cuts.releases);
    }
    struct rzf_scope *chain[8] = {NULL};
    for (unsigned i = 0; i < 8; ++i)
    {
        CHECK(rzf_scope_create(i ? chain[i - 1] : NULL, 0, 1, allocator(&a),
                               &chain[i]) == RZ_OK);
    }
    CHECK(rzf_scope_create(chain[7], 0, 1, allocator(&a), &child) ==
              RZ_INVALID &&
          !child);
    atomic_store(&chain[7]->references, SIZE_MAX);
    CHECK(rzf_scope_retain(chain[7], &child) == RZ_FULL && !child);
    atomic_store(&chain[7]->references, 1);
    for (unsigned i = 0; i < 8; ++i)
    {
        rzf_scope_release(&chain[i]);
    }
    CHECK(a.allocations == a.releases);

    struct rz_pause pause = {PTHREAD_MUTEX_INITIALIZER,
                             PTHREAD_COND_INITIALIZER, 0, false};
    CHECK(rzf_scope_create(NULL, 0, 20, allocator(&a), &parent) == RZ_OK);
    struct rz_worker workers[2] = {{&pause, {NULL}, 0}, {&pause, {NULL}, 0}};
    pthread_t threads[2];
    for (unsigned i = 0; i < 2; ++i)
    {
        CHECK(rzf_borrow_begin(&workers[i].borrow, parent) == RZ_OK);
    }
    CHECK(atomic_load(&parent->references) ==
          3); /* Reject omitted retain before freeing anything. */
    for (unsigned i = 0; i < 2; ++i)
    {
        CHECK(!pthread_create(&threads[i], NULL, borrower, &workers[i]));
    }
    CHECK(!pthread_mutex_lock(&pause.mutex));
    while (pause.started != 2)
    {
        CHECK(!pthread_cond_wait(&pause.condition, &pause.mutex));
    }
    rz_cancel(&parent->core);
    size_t before = a.releases;
    rzf_scope_release(&parent);
    CHECK(a.releases == before);
    pause.run = true;
    CHECK(!pthread_cond_broadcast(&pause.condition));
    CHECK(!pthread_mutex_unlock(&pause.mutex));
    for (unsigned i = 0; i < 2; ++i)
    {
        CHECK(!pthread_join(threads[i], NULL));
        CHECK(workers[i].result == RZ_CANCELLED);
        CHECK(rzf_borrow_ack(&workers[i].borrow) == RZ_OK);
        CHECK(a.releases == before + (i == 1));
        CHECK(rzf_borrow_ack(&workers[i].borrow) == RZ_STALE);
    }
    CHECK(!pthread_cond_destroy(&pause.condition));
    CHECK(!pthread_mutex_destroy(&pause.mutex));
    CHECK(a.allocations == a.releases);
}

static bool same_bucket(struct rzf_bucket a, struct rzf_bucket b)
{
    return a.capacity == b.capacity && a.numerator == b.numerator &&
           a.denominator == b.denominator && a.tokens == b.tokens &&
           a.remainder == b.remainder && a.last == b.last;
}

static void fractional_test(void)
{
    struct rzf_bucket a, p;
    CHECK(rzf_bucket_init(&a, 4, 1, 3, 0) == RZ_OK);
    CHECK(rzf_bucket_consume(&a, 0, 4) == RZ_OK);
    CHECK(rzf_bucket_consume(&a, 1, 1) == RZ_DENIED && a.remainder == 1);
    CHECK(rzf_bucket_consume(&a, 2, 1) == RZ_DENIED && a.remainder == 2);
    CHECK(rzf_bucket_consume(&a, 3, 1) == RZ_OK && a.remainder == 0);
    struct rzf_bucket copy = a;
    CHECK(rzf_bucket_consume(&a, 2, 1) == RZ_RANGE && same_bucket(a, copy));
    CHECK(rzf_bucket_consume(&a, 4, 5) == RZ_RANGE && same_bucket(a, copy));
    CHECK(rzf_bucket_init(&a, 4, 1, 3, 0) == RZ_OK);
    CHECK(rzf_bucket_init(&p, 2, 1, 4, 0) == RZ_OK);
    CHECK(rzf_bucket_consume(&p, 0, 2) == RZ_OK);
    CHECK(rzf_bucket_pair(&a, &p, 1, 1) == RZ_DENIED);
    CHECK(a.tokens == 4 && p.tokens == 0 && p.remainder == 1 && a.last == 1 &&
          p.last == 1);
    CHECK(rzf_bucket_pair(&a, &p, 4, 1) == RZ_OK && a.tokens == 3 &&
          p.tokens == 0);
    struct rzf_bucket before_a = a, before_p = p;
    CHECK(rzf_bucket_pair(&a, &p, 3, 1) == RZ_RANGE &&
          same_bucket(a, before_a) && same_bucket(p, before_p));
    CHECK(rzf_bucket_pair(&a, &a, 5, 1) == RZ_INVALID &&
          same_bucket(a, before_a));
    CHECK(rzf_bucket_init(&a, UINT32_MAX, UINT16_MAX, UINT16_MAX, 0) == RZ_OK);
    CHECK(rzf_bucket_consume(&a, 0, UINT32_MAX) == RZ_OK);
    CHECK(rzf_bucket_consume(&a, UINT64_MAX, UINT32_MAX) == RZ_OK &&
          a.tokens == 0);
    CHECK(rzf_bucket_init(&a, 2, 1, 3, 1000) == RZ_OK);
    CHECK(rzf_bucket_consume(&a, 1000, 2) == RZ_OK);
    struct rzf_credit credit;
    CHECK(rzf_credit_export(&a, 1002, &credit) == RZ_OK && credit.tokens == 0 &&
          credit.remainder == 2);
    CHECK(a.last == 1000 &&
          a.remainder ==
              0); /* Snapshot does not mint a second admission authority. */
    CHECK(rzf_credit_import(&credit, 40, 2, RZF_FREEZE_DOWNTIME, &p) == RZ_OK);
    CHECK(p.last == 40 && p.tokens == 0 && p.remainder == 2);
    CHECK(rzf_bucket_consume(&p, 41, 1) == RZ_OK && p.remainder == 0);
    CHECK(rzf_credit_import(&credit, 40, 2, RZF_REFILL_DOWNTIME, &p) == RZ_OK);
    CHECK(p.last == 40 && p.tokens == 1 && p.remainder == 1);
    CHECK(rzf_credit_import(&credit, UINT64_MAX, UINT64_MAX,
                            RZF_REFILL_DOWNTIME, &p) == RZ_OK &&
          p.tokens == 2);
    credit.remainder = credit.denominator;
    before_p = p;
    CHECK(rzf_credit_import(&credit, 0, 0, RZF_FREEZE_DOWNTIME, &p) ==
              RZ_INVALID &&
          same_bucket(p, before_p));
    for (uint64_t capacity = 1; capacity <= 8; ++capacity)
    {
        for (uint64_t rate = 1; rate <= 8; ++rate)
        {
            for (uint64_t denominator = 1; denominator <= 8; ++denominator)
            {
                CHECK(rzf_bucket_init(&a, capacity, rate, denominator, 0) ==
                      RZ_OK);
                CHECK(rzf_bucket_consume(&a, 0, capacity) == RZ_OK);
                for (uint64_t time = 1; time <= 20; ++time)
                {
                    CHECK(rzf_bucket_consume(&a, time, 0) == RZ_OK);
                    CHECK(rzf_bucket_init(&p, capacity, rate, denominator, 0) ==
                          RZ_OK);
                    CHECK(rzf_bucket_consume(&p, 0, capacity) == RZ_OK);
                    CHECK(rzf_bucket_consume(&p, time, 0) == RZ_OK);
                    CHECK(same_bucket(a, p));
                }
            }
        }
    }
}

static void deadline_oracle(void)
{
    uint64_t parent_now, parent_budget, child_now, child_budget;
    while (scanf("%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64, &parent_now,
                 &parent_budget, &child_now, &child_budget) == 4)
    {
        struct rz_allocation a = {0, 0, 0, SIZE_MAX};
        struct rzf_scope *parent = NULL, *child = NULL;
        int status = rzf_scope_create(NULL, parent_now, parent_budget,
                                      allocator(&a), &parent);
        if (status == RZ_OK)
        {
            status = rzf_scope_create(parent, child_now, child_budget,
                                      allocator(&a), &child);
        }
        printf("%d %" PRIu64 "\n", status, child ? child->core.deadline : 0);
        rzf_scope_release(&parent);
        rzf_scope_release(&child);
        CHECK(a.allocations == a.releases);
    }
}

static void budget_oracle(void)
{
    uint64_t remaining, transit, processing, now, parent_budget;
    while (scanf("%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
                 &remaining, &transit, &processing, &now, &parent_budget) == 5)
    {
        struct rz_allocation a = {0, 0, 0, SIZE_MAX};
        struct rzf_scope *parent = NULL, *scope = NULL;
        int status = RZ_OK;
        if (parent_budget != UINT64_MAX)
        {
            status = rzf_scope_create(NULL, now, parent_budget, allocator(&a),
                                      &parent);
        }
        struct rzf_budget budget = {remaining, transit};
        if (status == RZ_OK)
        {
            status = rzf_budget_import(&budget, parent, now, processing,
                                       allocator(&a), &scope);
        }
        printf("%d %" PRIu64 "\n", status, scope ? scope->core.deadline : 0);
        rzf_scope_release(&scope);
        rzf_scope_release(&parent);
        CHECK(a.allocations == a.releases);
    }
}

static void fractional_oracle(void)
{
    uint64_t capacity, p, q, last, now, cost;
    unsigned n;
    while (scanf("%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %u",
                 &capacity, &p, &q, &last, &n) == 5)
    {
        struct rzf_bucket b = {0};
        int status = rzf_bucket_init(&b, capacity, p, q, last);
        printf("%d", status);
        for (unsigned i = 0; i < n; ++i)
        {
            CHECK(scanf("%" SCNu64 " %" SCNu64, &now, &cost) == 2);
            status = rzf_bucket_consume(&b, now, cost);
            printf(" %d,%" PRIu64 ",%" PRIu64 ",%" PRIu64, status, b.tokens,
                   b.remainder, b.last);
        }
        puts("");
    }
}

static void scope_oracle(void)
{
    char word[16];
    while (scanf("%15s", word) == 1)
    {
        struct rz_allocation a = {0, 0, 0, SIZE_MAX};
        struct rzf_scope *parent = NULL, *child = NULL;
        CHECK(rzf_scope_create(NULL, 0, 10, allocator(&a), &parent) == RZ_OK);
        CHECK(rzf_scope_create(parent, 0, 20, allocator(&a), &child) == RZ_OK);
        struct rzf_borrow borrows[2] = {{NULL}, {NULL}};
        CHECK(rzf_borrow_begin(&borrows[0], child) == RZ_OK);
        CHECK(rzf_borrow_begin(&borrows[1], child) == RZ_OK);
        CHECK(atomic_load(&child->references) == 3);
        rzf_scope_release(&child);
        uint64_t now = 0;
        for (size_t i = 0; word[i]; ++i)
        {
            int status = RZ_OK;
            if (word[i] == 'c')
            {
                rz_cancel(&parent->core);
            }
            else if (word[i] == 'e')
            {
                now = 10;
            }
            else if (word[i] == 'a' || word[i] == 'b')
            {
                status = rzf_borrow_ack(&borrows[word[i] - 'a']);
            }
            else
            {
                CHECK(false);
            }
            struct rzf_scope *live =
                borrows[0].scope ? borrows[0].scope : borrows[1].scope;
            printf("%d,%d,%zu;", status,
                   live ? rz_check(&live->core, now) : RZ_DONE, a.releases);
        }
        if (borrows[0].scope)
        {
            CHECK(rzf_borrow_ack(&borrows[0]) == RZ_OK);
        }
        if (borrows[1].scope)
        {
            CHECK(rzf_borrow_ack(&borrows[1]) == RZ_OK);
        }
        rzf_scope_release(&parent);
        CHECK(a.allocations == a.releases);
        printf("D%zu\n", a.releases);
    }
}

static void migration_oracle(void)
{
    uint64_t capacity, p, q, spent, elapsed, downtime, origin;
    unsigned policy;
    while (scanf("%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64
                 " %" SCNu64 " %" SCNu64 " %u",
                 &capacity, &p, &q, &spent, &elapsed, &downtime, &origin,
                 &policy) == 8)
    {
        struct rzf_bucket source, destination = {0};
        struct rzf_credit credit;
        CHECK(rzf_bucket_init(&source, capacity, p, q, 0) == RZ_OK);
        CHECK(rzf_bucket_consume(&source, 0, spent) == RZ_OK);
        CHECK(rzf_credit_export(&source, elapsed, &credit) == RZ_OK);
        int status = rzf_credit_import(&credit, origin, downtime,
                                       (enum rzf_downtime)policy, &destination);
        printf("%d %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", status,
               destination.tokens, destination.remainder, destination.last);
    }
}

static void pair_oracle(void)
{
    uint64_t ac, ap, aq, pc, pp, pq, now, cost;
    unsigned count;
    while (scanf("%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64
                 " %" SCNu64 " %u",
                 &ac, &ap, &aq, &pc, &pp, &pq, &count) == 7)
    {
        struct rzf_bucket average, peak;
        CHECK(rzf_bucket_init(&average, ac, ap, aq, 0) == RZ_OK);
        CHECK(rzf_bucket_init(&peak, pc, pp, pq, 0) == RZ_OK);
        for (unsigned i = 0; i < count; ++i)
        {
            CHECK(scanf("%" SCNu64 " %" SCNu64, &now, &cost) == 2);
            int status = rzf_bucket_pair(&average, &peak, now, cost);
            printf("%d,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
                   ";",
                   status, average.tokens, average.remainder, peak.tokens,
                   peak.remainder, average.last);
            CHECK(average.last == peak.last);
        }
        puts("");
    }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "scope"))
    {
        scope_test();
    }
    else if (!strcmp(argv[1], "fractional"))
    {
        fractional_test();
    }
    else if (!strcmp(argv[1], "budget_oracle"))
    {
        budget_oracle();
    }
    else if (!strcmp(argv[1], "fractional_oracle"))
    {
        fractional_oracle();
    }
    else if (!strcmp(argv[1], "scope_oracle"))
    {
        scope_oracle();
    }
    else if (!strcmp(argv[1], "deadline_oracle"))
    {
        deadline_oracle();
    }
    else if (!strcmp(argv[1], "migration_oracle"))
    {
        migration_oracle();
    }
    else if (!strcmp(argv[1], "pair_oracle"))
    {
        pair_oracle();
    }
    else
    {
        return 2;
    }
    return 0;
}
