#ifndef B06_IMAGE_TRANSFER_H
#define B06_IMAGE_TRANSFER_H
#include "lifecycle_extension.h"

struct fi_image
{
    unsigned char bytes[32];
    unsigned unit, completed;
};

int fi_init(struct fi_image *, unsigned unit);
/* Program one aligned indexed unit from an immutable32-byte intended image.
 * bits models a prefix of physical bit attempts; complete is controller status.
 * It is not an attacker-supplied claim. No overlapping state/input objects. */
int fi_program(struct fi_image *, const unsigned char *, unsigned index,
               unsigned bits, int complete);
int fi_commit(const struct fi_image *, const unsigned char *, uint32_t sequence,
              unsigned char record[24]);
int fi_record_matches(const unsigned char image[32],
                      const unsigned char record[24]);

struct fi_rotation
{
    uint32_t epoch, sequence, limit, floor;
    unsigned key, revoked;
};

typedef int (*fi_persist_fn)(void *, const struct fi_rotation *);
int fi_advance(struct fi_rotation *);
/* Recovery authority is a trusted verified-key result, never image metadata.
 * Persist returns exactly1 only after durable atomic policy publication. */
int fi_rotate(struct fi_rotation *, uint32_t epoch, unsigned new_key,
              uint32_t floor, unsigned verified_key, int verdict, fi_persist_fn,
              void *);
#endif
