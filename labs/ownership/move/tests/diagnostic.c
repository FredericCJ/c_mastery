#include "move.h"

struct ow_owner permitted_struct_copy(struct ow_owner source)
{
    return source;
}

void ignored_result(struct ow_owner *old, struct omm_value *value)
{
#ifdef OMM_DIAGNOSTIC
    (void)value;
    ow_consume(old);
#else
    (void)old;
    omm_consume(value);
#endif
}
