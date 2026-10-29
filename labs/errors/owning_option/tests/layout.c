#include "option_types.h"
const unsigned eop_layout[] = {sizeof(void *),
                               sizeof(size_t),
                               sizeof(uint32_t),
                               sizeof(struct eop_scalar),
                               _Alignof(struct eop_scalar),
                               sizeof(struct eop_owner),
                               _Alignof(struct eop_owner)};
