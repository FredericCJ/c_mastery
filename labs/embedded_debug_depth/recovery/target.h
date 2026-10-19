#ifndef DEBUG_RECOVERY_TARGET_H
#define DEBUG_RECOVERY_TARGET_H
#include "recovery.h"
/* Boot main returns only on rejection; actual accepted handoff never returns. */
int du_target(void);
#endif
