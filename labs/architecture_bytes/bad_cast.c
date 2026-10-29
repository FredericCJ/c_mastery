/* Compile and reason only: this function MUST NOT be called. Alignment does
 * not change the declared unsigned-char object into an AWRecord. */
#include "bytes.h"

int32_t ab_bad_cast(void)
{
    _Alignas(
        struct aw_record) unsigned char storage[sizeof(struct aw_record)] = {0};
    return ((struct aw_record *)(void *)storage)->id;
}
