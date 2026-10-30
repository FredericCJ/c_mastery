/*
 * Teaching plugin built as a shared object. The default build exports a
 * valid descriptor `oc_plugin` and the function `oc_function`. The macros
 * OC_PLUGIN_BAD_ABI, OC_PLUGIN_BAD_SIZE and OC_PLUGIN_MISSING build the three
 * malformed variants used to exercise the loader's admission checks.
 */
#include "loader.h"

#define OC_PLUGIN_OFFSET 3

int oc_function(int value)
{
    return value + OC_PLUGIN_OFFSET;
}
#ifdef OC_PLUGIN_BAD_ABI
#define ABI 2
#else
#define ABI 1
#endif
#ifdef OC_PLUGIN_BAD_SIZE
#define SIZE 0
#else
#define SIZE sizeof(struct oc_plugin)
#endif
#ifndef OC_PLUGIN_MISSING
const struct oc_plugin oc_plugin = {ABI, SIZE, oc_function};
#endif
