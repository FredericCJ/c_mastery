#include "session_v2.h"
#include "generated_session.h"
#ifndef AC_REQUIRED_ALIGNMENT
#define AC_REQUIRED_ALIGNMENT 8
#endif
struct actual_storage { _Alignas(AC_REQUIRED_ALIGNMENT) struct ac_typed_v2 value; };
_Static_assert(AC_STORAGE_ABI==0x0200,"storage contract stale version");
_Static_assert(AC_STORAGE_SIZE>=sizeof(struct actual_storage),"storage contract size");
_Static_assert(AC_STORAGE_ALIGNMENT>=_Alignof(struct actual_storage),"storage contract alignment");
int main(void) {return 0;}
