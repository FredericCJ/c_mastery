#include "core.h"
int li_advance(const size_t *lengths, size_t count, LIPosition *position, size_t bytes) { return 0; }
int li_ready_after(int result, unsigned budget_exhausted) { return 0; }
int li_slot(uint32_t head, uint32_t tail, uint32_t capacity, uint32_t *out) { return 0; }
int li_event(LIRequest *request, uint32_t generation, unsigned event, int result) { return 0; }
int li_releasable(unsigned cancel_ack, unsigned target_terminal) { return 0; }
int li_expired(uint32_t start, uint32_t now, uint32_t duration, unsigned *out) { return 0; }
int li_durable(unsigned written, unsigned file_synced, unsigned renamed, unsigned dir_synced) { return 0; }
int li_buffer_release(unsigned submitted, unsigned terminal, unsigned borrowers) { return 0; }
