#include "abi.h"
static int apply(int n) { return n + 1; }
const struct spike_api dispatch_spike = {1, sizeof(struct spike_api), apply};
