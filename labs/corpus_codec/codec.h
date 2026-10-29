#ifndef CORPUS_CODEC_H
#define CORPUS_CODEC_H
#include <stddef.h>
#include <stdint.h>
enum { CC_OK, CC_INVALID, CC_SPACE };
enum { CC_SYMBOLS = 16, CC_SYMBOL_LIMIT = 512, CC_INPUT_LIMIT = 128,
       CC_TOKEN_LIMIT = 256, CC_FRAME_LIMIT = 978 };
struct cc_book { uint8_t length[16]; uint16_t code[16]; };
/* All spans must be live and pairwise disjoint, including output counters.
   Zero lengths admit NULL spans; nonzero lengths do not. No output changes on
   error. A book is reconstructed and validated before use; no padding bits are
   ignored. Contracts and original wire grammar are in CONTRACTS.md. */
int cc_lengths(const uint16_t frequency[16], uint8_t length[16]);
int cc_book(const uint8_t length[16], struct cc_book *book);
int cc_hencode(const uint8_t *symbols, size_t n, uint8_t *out, size_t cap, size_t *used);
int cc_hdecode(const uint8_t *frame, size_t n, uint8_t *out, size_t cap, size_t *used);
int cc_lzencode(const uint8_t *input, size_t n, uint8_t *out, size_t cap, size_t *used);
int cc_lzoptimal(const uint8_t *input, size_t n, uint8_t *out, size_t cap, size_t *used);
int cc_lzdecode(const uint8_t *tokens, size_t n, uint8_t *out, size_t cap, size_t *used);
int cc_compress(const uint8_t *input, size_t n, uint8_t *out, size_t cap, size_t *used);
int cc_compress_optimal(const uint8_t *input, size_t n, uint8_t *out, size_t cap, size_t *used);
int cc_expand(const uint8_t *frame, size_t n, uint8_t *out, size_t cap, size_t *used);
#endif
