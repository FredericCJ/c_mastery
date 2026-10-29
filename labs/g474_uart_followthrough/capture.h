#ifndef G4_CAPTURE_H
#define G4_CAPTURE_H
#include <stdint.h>

/* Version2: 24 little-endian uint32_t words, copied only after halted/done is observed.
   Host decoder binds this to the exact ELF and independent wire capture. */
struct g4_capture
{
    uint32_t magic, version, done, status, stage, polls, brr, baud, cr1, cr2,
        cr3, presc, ccipr, moder, afrl, isr, bytes, transfer_status,
        count_ticks, part_id, reserved[4];
};

/* reserved[0..3] in version2 are ready, released, RCC_CFGR, GPIO_AFRH. */
_Static_assert(sizeof(struct g4_capture) == 96, "capture ABI");
extern volatile struct g4_capture g4_capture;
#endif
