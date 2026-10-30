#include "receipt.h"

/* Reference implementation of receipt.h over the retained transactional
 * store. SDK_MUTANT_* macros select one deliberate defect each for the
 * mutation campaign; undefined builds the correct code.
 *
 * Store layout (all integers little-endian u32):
 *   "effect"         4 bytes   running signed sum, the externally visible effect
 *   'm', client      12 bytes  epoch, highest id ever accepted, next ring slot
 *   'r', client, k   12 bytes  receipt k of 3: id, delta, resulting value
 * The effect, the receipt and the metadata are written in ONE transaction, so
 * a retry can never see the effect without its receipt. */

enum
{
    CLIENT_FIRST = 1,
    CLIENT_LAST = 2,
    RECEIPT_SLOTS = 3,
    EFFECT_BYTES = 4,
    META_BYTES = 12,
    META_HIGH_WATER_AT = 4,
    META_NEXT_SLOT_AT = 8,
    RECORD_BYTES = 12,
    RECORD_DELTA_AT = 4,
    RECORD_VALUE_AT = 8,
    META_KEY_BYTES = 2,
    RECORD_KEY_BYTES = 3,
    /* Internal result of lookup_receipt(); never returned to callers. */
    LOOKUP_MISS = 1
};

#define EFFECT_KEY "effect"
#define EFFECT_KEY_BYTES (sizeof EFFECT_KEY - 1)

static uint32_t read32(const unsigned char *b)
{
    return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 |
           (uint32_t)b[3] << 24;
}

static void write32(unsigned char *b, uint32_t n)
{
    for (unsigned i = 0; i < 4; i++)
    {
        b[i] = (unsigned char)(n >> (8 * i));
    }
}

static int32_t signed32(uint32_t n)
{
    return n <= INT32_MAX ? (int32_t)n
                          : INT32_MIN + (int32_t)(n - UINT32_C(0x80000000));
}

static void make_meta_key(unsigned char key[META_KEY_BYTES], unsigned client)
{
    key[0] = 'm';
    key[1] = (unsigned char)client;
}

static void make_record_key(unsigned char key[RECORD_KEY_BYTES],
                            unsigned client, unsigned slot)
{
    key[0] = 'r';
    key[1] = (unsigned char)client;
    key[2] = (unsigned char)slot;
}

/* Reads exactly `length` bytes of a key that must exist. */
static int get_exact(store *s, const unsigned char *key, size_t key_bytes,
                     unsigned char *value, size_t length)
{
    size_t got = 0;
    int status = store_get(s, key, key_bytes, value, length, &got);
    return status == STORE_OK && got == length ? SDR_OK : SDR_STORAGE;
}

/* Commits the open transaction. An indeterminate commit means the data may or
 * may not be durable, so it is reported as SDR_UNCERTAIN, never as failure. */
static int commit(store *s)
{
    int status = store_commit(s);
    if (status == STORE_OK)
    {
        return SDR_OK;
    }
    return status == STORE_INDETERMINATE ? SDR_UNCERTAIN : SDR_STORAGE;
}

int sdr_initialize(store *s)
{
    if (!s || store_begin(s) != STORE_OK)
    {
        return SDR_STORAGE;
    }
    unsigned char zero[EFFECT_BYTES] = {0};
    unsigned char meta[META_BYTES] = {0};
    if (store_put(s, EFFECT_KEY, EFFECT_KEY_BYTES, zero, EFFECT_BYTES) !=
        STORE_OK)
    {
        goto fail;
    }
    for (unsigned client = CLIENT_FIRST; client <= CLIENT_LAST; client++)
    {
        unsigned char key[META_KEY_BYTES];
        make_meta_key(key, client);
        /* Epoch 1, nothing accepted yet, ring starts at slot 0. */
        write32(meta, 1);
        if (store_put(s, key, META_KEY_BYTES, meta, META_BYTES) != STORE_OK)
        {
            goto fail;
        }
    }
    return commit(s);
fail:
    store_abort(s);
    return SDR_STORAGE;
}

int sdr_value(store *s, int32_t *out)
{
    unsigned char value[EFFECT_BYTES];
    if (!s || !out)
    {
        return SDR_INVALID;
    }
    if (get_exact(s, (const unsigned char *)EFFECT_KEY, EFFECT_KEY_BYTES, value,
                  EFFECT_BYTES))
    {
        return SDR_STORAGE;
    }
    *out = signed32(read32(value));
    return SDR_OK;
}

/* Looks for a remembered receipt with this id among the client's ring slots.
 * Returns SDR_OK and the recorded value (a replay), SDR_CONFLICT when the id
 * was used with another delta, SDR_STORAGE for an unreadable or malformed
 * record, or LOOKUP_MISS. */
static int lookup_receipt(store *s, unsigned client, uint32_t id, int32_t delta,
                          int32_t *out)
{
    for (unsigned slot = 0; slot < RECEIPT_SLOTS; slot++)
    {
        unsigned char key[RECORD_KEY_BYTES];
        unsigned char record[RECORD_BYTES];
        size_t got = 0;
        make_record_key(key, client, slot);
        int status =
            store_get(s, key, RECORD_KEY_BYTES, record, sizeof record, &got);
        if (status == STORE_NOT_FOUND)
        {
            continue;
        }
        if (status != STORE_OK || got != RECORD_BYTES)
        {
            return SDR_STORAGE;
        }
        if (read32(record) != id)
        {
            continue;
        }
#ifndef SDK_MUTANT_PAYLOAD_BINDING
        /* The id is bound to its payload: reuse with another delta is a
         * conflict, not a replay. */
        if (signed32(read32(record + RECORD_DELTA_AT)) != delta)
        {
            return SDR_CONFLICT;
        }
#else
        (void)delta;
#endif
#ifndef SDK_MUTANT_REAPPLY
        *out = signed32(read32(record + RECORD_VALUE_AT));
        return SDR_OK;
#else
        (void)out;
#endif
    }
    return LOOKUP_MISS;
}

/* Writes the new effect, receipt and metadata in one transaction and commits.
 * Any failed put aborts the whole transaction. */
static int publish(store *s, const unsigned char *meta_key,
                   const unsigned char *effect, const unsigned char *record_key,
                   const unsigned char *record, const unsigned char *meta)
{
    if (store_begin(s) != STORE_OK)
    {
        return SDR_STORAGE;
    }
    if (store_put(s, EFFECT_KEY, EFFECT_KEY_BYTES, effect, EFFECT_BYTES) !=
        STORE_OK)
    {
        goto rollback;
    }
#ifdef SDK_MUTANT_SPLIT_RECEIPT
    /* Defect: the effect commits alone; a cut before the receipt is written
     * leaves an effect with no receipt, so a retry applies it twice. */
    if (commit(s) != SDR_OK)
    {
        return SDR_UNCERTAIN;
    }
    if (store_begin(s) != STORE_OK)
    {
        return SDR_STORAGE;
    }
#endif
    if (store_put(s, record_key, RECORD_KEY_BYTES, record, RECORD_BYTES) !=
            STORE_OK ||
        store_put(s, meta_key, META_KEY_BYTES, meta, META_BYTES) != STORE_OK)
    {
        goto rollback;
    }
    return commit(s);
rollback:
    store_abort(s);
    return SDR_STORAGE;
}

int sdr_apply(store *s, uint32_t client, uint32_t epoch, uint32_t id,
              int32_t delta, int32_t *out)
{
    if (!s || !out || client < CLIENT_FIRST || client > CLIENT_LAST || !epoch ||
        !id)
    {
        return SDR_INVALID;
    }
    unsigned char meta_key[META_KEY_BYTES];
    unsigned char meta[META_BYTES];
    make_meta_key(meta_key, client);
    if (get_exact(s, meta_key, META_KEY_BYTES, meta, META_BYTES))
    {
        return SDR_STORAGE;
    }
    uint32_t current_epoch = read32(meta);
    uint32_t high_water = read32(meta + META_HIGH_WATER_AT);
    uint32_t next_slot = read32(meta + META_NEXT_SLOT_AT);
    if (epoch != current_epoch)
    {
        return SDR_STALE;
    }
    if (next_slot >= RECEIPT_SLOTS)
    {
        return SDR_STORAGE;
    }
    int found = lookup_receipt(s, client, id, delta, out);
    if (found != LOOKUP_MISS)
    {
        return found;
    }
#ifndef SDK_MUTANT_REAPPLY
    /* An evicted receipt is indistinguishable from a never-seen id except
     * through the high-water mark, which keeps old ids stale for good. */
    if (id <= high_water)
    {
        return SDR_STALE;
    }
#else
    (void)high_water;
#endif
    int32_t previous;
    if (sdr_value(s, &previous))
    {
        return SDR_STORAGE;
    }
    int64_t sum = (int64_t)previous + delta;
    if (sum < INT32_MIN || sum > INT32_MAX)
    {
        return SDR_OVERFLOW;
    }
    int32_t value = (int32_t)sum;
    unsigned char effect[EFFECT_BYTES];
    unsigned char record[RECORD_BYTES];
    unsigned char record_key[RECORD_KEY_BYTES];
    make_record_key(record_key, client, next_slot);
    write32(effect, (uint32_t)value);
    write32(record, id);
    write32(record + RECORD_DELTA_AT, (uint32_t)delta);
    write32(record + RECORD_VALUE_AT, (uint32_t)value);
    write32(meta + META_HIGH_WATER_AT, id);
    write32(meta + META_NEXT_SLOT_AT, (next_slot + 1) % RECEIPT_SLOTS);
    int status = publish(s, meta_key, effect, record_key, record, meta);
    if (status != SDR_OK)
    {
        return status;
    }
    *out = value;
    return SDR_OK;
}
