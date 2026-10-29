#include "dispatch_plugin.h"
#include <stdint.h>
#ifndef DS_PLUGIN_CASE
#define DS_PLUGIN_CASE 0
#endif
void ds_constructor_note(void);

__attribute__((constructor)) static void note_load(void)
{
    ds_constructor_note();
}

static int apply(int state, int amount, int *out)
{
    if (state < 0 || state > 1000 || amount < 0 || amount > 100)
        return DS_RANGE;
    *out = state + amount;
    return DS_OK;
}

static int migrate(unsigned previous, int state, int *out)
{
    if (previous > 2 || state < 0 || state > 500)
        return DS_RANGE;
    *out = state;
    return DS_OK;
}
#if DS_PLUGIN_CASE == 7
const struct ds_plugin deliberately_wrong_symbol = {1, sizeof(struct ds_plugin),
                                                    1, apply, migrate};
#elif DS_PLUGIN_CASE == 8
const struct ds_plugin dispatch_plugin = {1, sizeof(struct ds_plugin) - 1, 1,
                                          apply, migrate};
#elif DS_PLUGIN_CASE == 9
const struct ds_plugin dispatch_plugin = {1, sizeof(struct ds_plugin) + 1, 1,
                                          apply, migrate};
#else
const struct ds_plugin dispatch_plugin = {1, sizeof(struct ds_plugin), 1, apply,
                                          migrate};
#endif
#if DS_PLUGIN_CASE == 0
const struct ds_plugin_base dispatch_plugin_extended = {
    2, 0, sizeof(struct ds_plugin_base), 0, 0, apply};
#elif DS_PLUGIN_CASE == 1 || DS_PLUGIN_CASE == 2 || DS_PLUGIN_CASE == 5 || \
    DS_PLUGIN_CASE == 6
static int describe(unsigned char *out, size_t cap, size_t *n)
{
    if (cap < 2)
        return DS_FULL;
    out[0] = 'v';
    out[1] = '2';
    *n = 2;
    return DS_OK;
}
#if DS_PLUGIN_CASE == 5
struct ds_future
{
    struct ds_plugin_optional prefix;
    uint64_t tail;
};
const struct ds_future dispatch_plugin_extended = {
    {{2, 9, sizeof(struct ds_future), 0, 3, apply}, describe},
    123};
#else
const struct ds_plugin_optional dispatch_plugin_extended = {
    {2, 1,
     DS_PLUGIN_CASE == 6 ? sizeof(struct ds_plugin_base)
                         : sizeof(struct ds_plugin_optional),
     DS_PLUGIN_CASE == 2 ? 1 : 0, 1, apply},
    describe};
#endif
#elif DS_PLUGIN_CASE == 3
const struct ds_plugin_base dispatch_plugin_extended = {
    2, 1, sizeof(struct ds_plugin_base), 2, 2, apply};
#elif DS_PLUGIN_CASE == 4
const struct ds_plugin_base dispatch_plugin_extended = {
    2, 0, sizeof(struct ds_plugin_base) - 1, 0, 0, apply};
#endif
