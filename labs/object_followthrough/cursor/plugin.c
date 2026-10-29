#include "loader.h"

int oc_function(int value)
{
    return value + 3;
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
