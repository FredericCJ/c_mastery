#ifndef CMASTERY_REPLICATED_HASHING_H
#define CMASTERY_REPLICATED_HASHING_H
#include <stddef.h>
#include <stdint.h>

enum
{
    RPH_OK,
    RPH_INVALID,
    RPH_RANGE,
    RPH_STATE,
    RPH_UNIMPLEMENTED
};

#define RPH_MAX_BYTES (UINT64_MAX / UINT64_C(8))

struct rph_sha
{
    uint32_t words[8];
    uint64_t bytes;
    unsigned char block[64];
    unsigned used, final;
};

struct rph_hmac
{
    struct rph_sha inner, outer;
};

int rph_init(struct rph_sha *state);
int rph_update(struct rph_sha *state, const unsigned char *bytes, size_t count);
int rph_final(struct rph_sha *state, unsigned char out[32]);
int rph_hash(const unsigned char *bytes, size_t count, unsigned char out[32]);
int rph_hmac_init(struct rph_hmac *state, const unsigned char *key,
                  size_t count);
int rph_hmac_update(struct rph_hmac *state, const unsigned char *bytes,
                    size_t count);
int rph_hmac_final(struct rph_hmac *state, unsigned char out[32]);
int rph_hmac(const unsigned char *key, size_t key_count,
             const unsigned char *bytes, size_t count, unsigned char out[32]);
/* Fixed 32-byte comparison. Source work is fixed; generated timing is audited,
   never inferred to be constant from ISO C or a finite timing sample. */
int rph_equal(const unsigned char left[32], const unsigned char right[32]);
/* Live writable object span, no concurrent access. Clears that span only. */
void rph_clear(void *object, size_t count);
#endif
