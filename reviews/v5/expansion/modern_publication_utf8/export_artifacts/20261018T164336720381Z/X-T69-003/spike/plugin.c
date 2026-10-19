#include "abi.h"
static int apply(int n) { return n + 1; }
const SpikeAPI dispatch_spike = {1, sizeof(SpikeAPI), apply};
