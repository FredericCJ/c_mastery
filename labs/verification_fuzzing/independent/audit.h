#ifndef B06_INDEPENDENT_AUDIT_H
#define B06_INDEPENDENT_AUDIT_H
#include <stddef.h>
#include <stdint.h>
struct audit_state {unsigned char present,value[4];};
int audit_packet(const unsigned char*,size_t,struct audit_state*);
#endif
