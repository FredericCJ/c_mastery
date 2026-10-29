#ifndef B06_LC_H
#define B06_LC_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
struct lc_boot { unsigned active,trial,confirmed,attempted; };
struct lc_journal { uint32_t seq,value,check,commit; };
int lc_program(unsigned char *dst, const unsigned char *src, size_t n);
uint32_t lc_crc(const unsigned char *data, size_t n);
int lc_admit(uint32_t length, uint32_t capacity, uint32_t version, uint32_t floor, unsigned signature_ok);
int lc_select(struct lc_boot *state, unsigned *slot);
int lc_journal_select(const struct lc_journal *a, const struct lc_journal *b, unsigned *index);
int lc_raise_floor(uint32_t current, uint32_t candidate, unsigned confirmed, uint32_t *out);
int lc_peer_ready(uint32_t local_epoch, uint32_t ack_epoch, unsigned outstanding);
int lc_capture_valid(const uint32_t *words, size_t n);
#endif
