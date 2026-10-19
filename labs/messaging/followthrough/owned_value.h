#ifndef CMASTERY_MESSAGING_OWNED_VALUE_H
#define CMASTERY_MESSAGING_OWNED_VALUE_H
/* X-T73-002 producer; X-T73-003 and later adapters consume this contract. */
typedef struct { void *data; void (*drop)(void *); } MfoValue;
void mfo_drop(MfoValue *);
#endif
