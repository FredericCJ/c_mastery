#include "registry.h"
#include "test.h"

struct ac_plugin
{
    unsigned calls, drops;
};

static void release(void *p)
{
    ++((struct ac_plugin *)p)->drops;
}

static void call(void *p, struct as_registry *r, struct api_handle handle)
{
    struct ac_plugin *x = p;
    ++x->calls;
    CHECK(as_remove(r, handle) == API_PENDING);
}

int main(void)
{
    struct as_registry r;
    as_registry_init(&r, 1, 0, 0);
    struct ac_plugin p = {0};
    struct api_handle h;
    CHECK(as_register(&r, call, &p, release, &h) == API_OK && p.calls == 0);
    CHECK(as_dispatch(&r) == API_OK && p.calls == 1 && p.drops == 1);
    CHECK(as_dispatch(&r) == API_OK && p.calls == 1);
    CHECK(as_destroy(&r) == API_OK);
    puts(
        "PASS actual registry registration transfers context,execution callback "
        "removes,deferred release");
}
