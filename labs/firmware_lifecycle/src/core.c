#include "core.h"
int lc_program(unsigned char *dst, const unsigned char *src, size_t n) { return 0; }
uint32_t lc_crc(const unsigned char *data, size_t n) { return 0; }
int lc_admit(uint32_t length, uint32_t capacity, uint32_t version, uint32_t floor, unsigned signature_ok) { return 0; }
int lc_select(LCBoot *state, unsigned *slot) { return 0; }
int lc_journal_select(const LCJournal *a, const LCJournal *b, unsigned *index) { return 0; }
int lc_raise_floor(uint32_t current, uint32_t candidate, unsigned confirmed, uint32_t *out) { return 0; }
int lc_peer_ready(uint32_t local_epoch, uint32_t ack_epoch, unsigned outstanding) { return 0; }
int lc_capture_valid(const uint32_t *words, size_t n) { return 0; }
