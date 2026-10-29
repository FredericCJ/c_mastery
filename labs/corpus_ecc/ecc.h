#ifndef CORPUS_ECC_H
#define CORPUS_ECC_H
#include <stdint.h>

enum ec_status
{
    EC_OK,
    EC_CORRECTED,
    EC_DETECTED,
    EC_INVALID,
    EC_AMBIGUOUS
};

struct ec_result
{
    uint16_t data, word;
    unsigned position;
};
/* n=8 or16. Positions1..n-1 use powers of two for checks; position n
 * is overall even parity. Data bits ascend through remaining positions.
 * All outputs unchanged on refusal. Controls/outputs live and disjoint. */
enum ec_status ec_encode(unsigned n, uint16_t data, uint16_t *out);
enum ec_status ec_decode(unsigned n, uint16_t received, struct ec_result *out);
/* Erased bits ignored. Admit 2*errors+popcount(erased)<4, errors0or1.
 * Nearest search is intentionally bounded/enumerative, not the fast decoder. */
enum ec_status ec_erasures(unsigned n, uint16_t received, uint16_t erased,
                           unsigned errors, struct ec_result *out);
/* Four immutable words, column-major bit interleave. Each byte is bit0or1.
 * Output needs4*n octets. Input/output extents must be disjoint. */
enum ec_status ec_interleave(unsigned n, const uint16_t words[4],
                             uint8_t bits[64]);
enum ec_status ec_deinterleave(unsigned n, const uint8_t bits[64],
                               uint16_t words[4]);
enum ec_status ec_interleave_phase(unsigned n, const uint16_t words[4],
                                   unsigned phase, uint8_t bits[64]);
enum ec_status ec_deinterleave_phase(unsigned n, const uint8_t bits[64],
                                     unsigned phase, uint16_t words[4]);
#endif
