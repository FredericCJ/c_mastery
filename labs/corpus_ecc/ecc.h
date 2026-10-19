#ifndef CORPUS_ECC_H
#define CORPUS_ECC_H
#include <stdint.h>
typedef enum { EC_OK, EC_CORRECTED, EC_DETECTED, EC_INVALID, EC_AMBIGUOUS } ECStatus;
typedef struct { uint16_t data, word; unsigned position; } ECResult;
/* n=8 or16. Positions1..n-1 use powers of two for checks; position n
 * is overall even parity. Data bits ascend through remaining positions.
 * All outputs unchanged on refusal. Controls/outputs live and disjoint. */
ECStatus ec_encode(unsigned n, uint16_t data, uint16_t *out);
ECStatus ec_decode(unsigned n, uint16_t received, ECResult *out);
/* Erased bits ignored. Admit 2*errors+popcount(erased)<4, errors0or1.
 * Nearest search is intentionally bounded/enumerative, not the fast decoder. */
ECStatus ec_erasures(unsigned n, uint16_t received, uint16_t erased,
                     unsigned errors, ECResult *out);
/* Four immutable words, column-major bit interleave. Each byte is bit0or1.
 * Output needs4*n octets. Input/output extents must be disjoint. */
ECStatus ec_interleave(unsigned n, const uint16_t words[4], uint8_t bits[64]);
ECStatus ec_deinterleave(unsigned n, const uint8_t bits[64], uint16_t words[4]);
ECStatus ec_interleave_phase(unsigned n, const uint16_t words[4], unsigned phase, uint8_t bits[64]);
ECStatus ec_deinterleave_phase(unsigned n, const uint8_t bits[64], unsigned phase, uint16_t words[4]);
#endif
