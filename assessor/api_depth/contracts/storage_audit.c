#include "session_v2.h"
#include "generated_session.h"
#ifndef AC_REQUIRED_ALIGNMENT
#define AC_REQUIRED_ALIGNMENT 8
#endif
typedef struct { _Alignas(AC_REQUIRED_ALIGNMENT) AcTypedV2 value; } ActualStorage;
_Static_assert(AC_STORAGE_ABI==0x0200,"storage contract stale version");
_Static_assert(AC_STORAGE_SIZE>=sizeof(ActualStorage),"storage contract size");
_Static_assert(AC_STORAGE_ALIGNMENT>=_Alignof(ActualStorage),"storage contract alignment");
int main(void) {return 0;}
