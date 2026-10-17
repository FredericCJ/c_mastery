#ifndef B06_LI_H
#define B06_LI_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
typedef struct {size_t index,offset;} LIPosition;
typedef struct {uint32_t generation;unsigned state;int result;} LIRequest;
int li_advance(const size_t *lengths, size_t count, LIPosition *position, size_t bytes);
int li_ready_after(int result, unsigned budget_exhausted);
int li_slot(uint32_t head, uint32_t tail, uint32_t capacity, uint32_t *out);
int li_event(LIRequest *request, uint32_t generation, unsigned event, int result);
int li_releasable(unsigned cancel_ack, unsigned target_terminal);
int li_expired(uint32_t start, uint32_t now, uint32_t duration, unsigned *out);
int li_durable(unsigned written, unsigned file_synced, unsigned renamed, unsigned dir_synced);
int li_buffer_release(unsigned submitted, unsigned terminal, unsigned borrowers);
#endif
