#include "dispatch.h"
#ifndef PLUGIN_SCHEMA
#define PLUGIN_SCHEMA 1
#endif
#ifndef PLUGIN_ABI
#define PLUGIN_ABI 1
#endif
static int apply(int state, int amount, int *out) {
  if (amount < 0 || amount > 100 || state < 0 || state > 1000)
    return DS_RANGE;
  *out = state + amount;
  return DS_OK;
}
static int migrate(unsigned previous, int state, int *out) {
  if (previous > 2 || state < 0 || state > 500)
    return DS_RANGE;
  *out = PLUGIN_SCHEMA == 2 ? state * 2 : state;
  return DS_OK;
}
const DSPlugin dispatch_plugin = {PLUGIN_ABI, sizeof(DSPlugin), PLUGIN_SCHEMA,
                                  apply, migrate};
