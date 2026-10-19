#ifndef EMBEDDED_DEBUG_RELEASE_TARGET_H
#define EMBEDDED_DEBUG_RELEASE_TARGET_H
#include "release.h"
int dt_target(void);
void dt_irq0(void);
void dt_irq1(void);
void dt_output(const char *);
void dt_hex(uint32_t);
#endif
