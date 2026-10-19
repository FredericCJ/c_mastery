#ifndef CMASTERY_ARCH_LINUX_H
#define CMASTERY_ARCH_LINUX_H
#include "arch.h"
/* Linux hosted adapter; file descriptor ownership stays with its caller. */
ArResult ar_linux_read(int, unsigned char *, size_t);
int ar_outbox(int, uint32_t, int32_t, unsigned);
#endif
