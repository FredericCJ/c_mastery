#ifndef B06_INDEPENDENT_AUDIT_H
#define B06_INDEPENDENT_AUDIT_H
#include <stddef.h>
#include <stdint.h>
typedef struct {unsigned char present,value[4];} AuditState;
int audit_packet(const unsigned char*,size_t,AuditState*);
#endif
