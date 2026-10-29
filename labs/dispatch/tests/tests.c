#include "dispatch.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                              \
    do                                                        \
    {                                                         \
        if (!(x))                                             \
        {                                                     \
            fprintf(stderr, "DS_CHECK:%s:%s\n", argv[1], #x); \
            return 1;                                         \
        }                                                     \
    } while (0)

static int read_int(const void *p, int *out)
{
    *out = *(const int *)p;
    return DS_OK;
}

static int read_fail(const void *p, int *out)
{
    (void)p;
    *out = 123;
    return DS_FAILED;
}

static int add(int a, int b, int *out)
{
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
        return DS_RANGE;
    *out = a + b;
    return DS_OK;
}

static int subtract(int a, int b, int *out)
{
    if (a < -100 || a > 100 || b < -100 || b > 100)
        return DS_RANGE;
    *out = a - b;
    return DS_OK;
}

static int reject_negative(int a, int b, int *out)
{
    if (b < 0)
        return DS_FAILED;
    return add(a, b, out);
}

static unsigned char *allocate(void *c, size_t n)
{
    (void)c;
    return malloc(n);
}

static void release(void *c, unsigned char *p)
{
    (void)c;
    free(p);
}

static int thunk(const unsigned char *p, size_t n, int a, int *out)
{
    if (n != 1 || a < -100 || a > 100)
        return DS_RANGE;
    *out = p[0] + a;
    return DS_OK;
}

static void destroy(void *p)
{
    (*(unsigned *)p)++;
}

struct ds_test_service
{
    unsigned calls, fail;
    int value;
};

static int service_read(void *p, int *out)
{
    struct ds_test_service *s = p;
    s->calls++;
    if (s->calls == s->fail)
        return DS_FAILED;
    *out = s->value;
    return DS_OK;
}

int main(int argc, char **argv)
{
    if (argc != 5)
        return 2;
    if (!strcmp(argv[1], "traits"))
    {
        int value = 7, out = 99;
        struct ds_ops ops = {read_int}, bad = {read_fail};
        CHECK(ds_measure((struct ds_view){&value, &ops}, &out) == DS_OK &&
              out == 7);
        out = 99;
        CHECK(ds_measure((struct ds_view){&value, &bad}, &out) == DS_FAILED &&
              out == 99);
        CHECK(ds_measure((struct ds_view){&value, NULL}, &out) == DS_INVALID &&
              out == 99);
    }
    else if (!strcmp(argv[1], "fat"))
    {
        struct ds_ops ops = {read_int}, bad = {read_fail};
        struct ds_box box = {&ops, 17};
        int out = 99;
        CHECK(ds_embedded(&box, &out) == DS_OK && out == 17);
        box.ops = &bad;
        out = 99;
        CHECK(ds_embedded(&box, &out) == DS_FAILED && out == 99);
    }
    else if (!strcmp(argv[1], "dictionary"))
    {
        int p[] = {3, 2}, bad[] = {3, -1}, out = 99;
        CHECK(ds_fold(p, 2, 10, subtract, &out) == DS_OK && out == 5);
        CHECK(ds_sum(p, 2, 10, &out) == DS_OK && out == 15);
        CHECK(ds_fold(NULL, 0, 7, add, &out) == DS_OK && out == 7);
        out = 99;
        CHECK(ds_fold(bad, 2, 10, reject_negative, &out) == DS_FAILED &&
              out == 99);
    }
    else if (!strcmp(argv[1], "closures"))
    {
        struct ds_closure c;
        int out = 99;
        CHECK(ds_closure_init(&c, 3, DS_SHARED) == DS_OK);
        CHECK(ds_closure_call(&c, 2, &out) == DS_OK && out == 5 &&
              c.capture == 3);
        CHECK(ds_closure_init(&c, 3, DS_MUTABLE) == DS_OK);
        CHECK(ds_closure_call(&c, 2, &out) == DS_OK && out == 5 &&
              c.capture == 5);
        CHECK(ds_closure_call(&c, 2, &out) == DS_OK && out == 7);
        CHECK(ds_closure_init(&c, 1, DS_ONCE) == DS_OK);
        CHECK(ds_closure_call(&c, 2, &out) == DS_OK && out == 3);
        CHECK(ds_closure_call(&c, 2, &out) == DS_EMPTY && out == 3);
        CHECK(ds_closure_init(&c, INT_MAX, DS_ONCE) == DS_OK);
        CHECK(ds_closure_call(&c, 1, &out) == DS_RANGE && !c.active &&
              out == 3);
    }
    else if (!strcmp(argv[1], "erased"))
    {
        struct ds_erased a, b;
        unsigned destroyed = 0;
        unsigned char bytes[] = {7};
        int out = 99;
        struct d_allocator allocator = {NULL, allocate, release};
        ds_erased_init(&a, allocator);
        ds_erased_init(&b, allocator);
        CHECK(ds_erased_assign(&a, bytes, 1, thunk, destroy, &destroyed) ==
              DS_OK);
        CHECK(ds_erased_call(&a, 3, &out) == DS_OK && out == 10);
        CHECK(ds_erased_move(&b, &a) == DS_OK && !a.active);
        CHECK(ds_erased_call(&a, 3, &out) == DS_EMPTY && out == 10);
        ds_erased_drop(&a);
        CHECK(destroyed == 0);
        CHECK(ds_erased_call(&b, 3, &out) == DS_OK && out == 10);
        ds_erased_drop(&b);
        ds_erased_drop(&b);
        CHECK(destroyed == 1);
    }
    else if (!strcmp(argv[1], "multiple"))
    {
        struct ds_methods m;
        struct ds_cache c = {0, 0, 0, NULL};
        int out = 99;
        ds_methods_init(&m);
        CHECK(ds_methods_set(&m, 0, 1, subtract) == DS_OK);
        CHECK(ds_dispatch(&m, &c, 0, 1, 10, 2, &out) == DS_OK && out == 8);
        CHECK(ds_dispatch(&m, &c, 1, 0, 10, 2, &out) == DS_UNSUPPORTED &&
              out == 8);
        CHECK(ds_dispatch(&m, &c, 0, 1, 10, 2, &out) == DS_OK && out == 8);
        CHECK(ds_methods_set(&m, 0, 1, add) == DS_OK);
        CHECK(ds_dispatch(&m, &c, 0, 1, 10, 2, &out) == DS_OK && out == 12);
        CHECK(ds_methods_set(&m, 0, 1, NULL) == DS_FULL);
    }
    else if (!strcmp(argv[1], "intrusive"))
    {
        struct ds_list l;
        struct ds_item a, b, c;
        int out = 99;
        ds_list_init(&l);
        ds_item_init(&a, 1);
        ds_item_init(&b, 2);
        ds_item_init(&c, 3);
        CHECK(ds_list_push(&l, &a) == DS_OK);
        CHECK(ds_list_push(&l, &b) == DS_OK);
        CHECK(ds_list_push(&l, &c) == DS_OK);
        CHECK(ds_list_push(&l, &a) == DS_INVALID);
        CHECK(ds_list_sum(&l, &out) == DS_OK && out == 6);
        CHECK(ds_list_remove(&l, &b) == DS_OK && l.count == 2);
        CHECK(c.link.previous == &a.link && a.link.next == &c.link);
        CHECK(ds_list_sum(&l, &out) == DS_OK && out == 4);
        CHECK(ds_list_remove(&l, &b) == DS_INVALID);
        CHECK(ds_list_remove(&l, &a) == DS_OK);
        CHECK(ds_list_remove(&l, &c) == DS_OK);
        CHECK(l.head.next == &l.head && l.head.previous == &l.head);
    }
    else if (!strcmp(argv[1], "modules"))
    {
        struct ds_test_service s = {0, 0, 7};
        struct ds_module *a = NULL, *b = NULL, *out_module;
        int out = 99;
        CHECK(ds_module_open((struct ds_service){service_read, &s}, &a) ==
              DS_OK);
        CHECK(ds_module_open((struct ds_service){service_read, &s}, &b) ==
              DS_OK);
        out_module = a;
        CHECK(ds_module_open((struct ds_service){service_read, &s},
                             &out_module) == DS_FULL &&
              out_module == a);
        CHECK(ds_module_twice(a, &out) == DS_OK && out == 14 && s.calls == 2);
        s.calls = 0;
        s.fail = 2;
        out = 99;
        CHECK(ds_module_twice(a, &out) == DS_FAILED && out == 99 &&
              s.calls == 2);
        ds_module_close(a);
        ds_module_close(b);
    }
    else if (!strcmp(argv[1], "capabilities"))
    {
        struct ds_authority root;
        struct ds_capability full, read, other;
        ds_authority_init(&root, 3);
        CHECK(ds_cap_issue(&root, 3, &full) == DS_OK);
        CHECK(ds_cap_attenuate(full, 1, &read) == DS_OK);
        CHECK(ds_cap_add(&root, read, 5) == DS_DENIED && root.counter == 0);
        other = read;
        CHECK(ds_cap_attenuate(read, 3, &other) == DS_DENIED &&
              other.rights == 1);
        CHECK(ds_cap_add(&root, full, 5) == DS_OK && root.counter == 5);
        CHECK(ds_cap_revoke(&root) == DS_OK);
        CHECK(ds_cap_add(&root, full, 5) == DS_STALE && root.counter == 5);
    }
    else if (!strcmp(argv[1], "plugin"))
    {
        struct ds_loaded m;
        const struct ds_plugin *api = NULL;
        int out = 99;
        ds_loaded_init(&m);
        CHECK(ds_loaded_open(&m, argv[4]) == DS_INVALID && !m.handle);
        CHECK(ds_loaded_open(&m, argv[2]) == DS_OK);
        CHECK(ds_loaded_acquire(&m, &api) == DS_OK && api);
        CHECK(ds_loaded_close(&m) == DS_BUSY && m.handle);
        CHECK(api->apply(17, 3, &out) == DS_OK && out == 20);
        api = NULL;
        CHECK(ds_loaded_release(&m) == DS_OK);
        CHECK(ds_loaded_close(&m) == DS_OK && !m.handle && !m.api);
    }
    else if (!strcmp(argv[1], "reload"))
    {
        struct ds_host h;
        struct ds_loaded candidate;
        ds_host_init(&h);
        ds_loaded_init(&candidate);
        h.state = 7;
        CHECK(ds_loaded_open(&candidate, argv[2]) == DS_OK);
        CHECK(ds_host_reload(&h, &candidate) == DS_OK && h.state == 7 &&
              h.generation == 1 && !candidate.handle);
        CHECK(ds_loaded_open(&candidate, argv[3]) == DS_OK);
        CHECK(ds_host_reload(&h, &candidate) == DS_OK && h.state == 14 &&
              h.generation == 2 && h.retired.handle && !candidate.handle);
        CHECK(ds_host_collect(&h) == DS_OK && !h.retired.handle);
        CHECK(ds_loaded_open(&candidate, argv[3]) == DS_OK);
        h.state = 600;
        CHECK(ds_host_reload(&h, &candidate) == DS_RANGE && h.state == 600 &&
              h.generation == 2 && candidate.handle && !h.retired.handle);
        CHECK(ds_loaded_close(&candidate) == DS_OK);
        CHECK(ds_loaded_close(&h.current) == DS_OK);
    }
    else
        return 2;
    return 0;
}
