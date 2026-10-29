#ifndef B06_FLASH_MAINTENANCE_H
#define B06_FLASH_MAINTENANCE_H
#include "h745_flash.h"

/* Exact selected profiles: F439 2MiB unswapped, H745xI 2MiB unswapped.
 * Fresh must make subsequent reads observe flash, not pre-operation cache.
 * It must not fail, reenter, or access a bank while an operation is pending. */
struct fm_io
{
    struct fh_io bus;
    void (*fresh)(void *, uint32_t, uint32_t);
};

enum fm_profile
{
    FM_F439 = 4,
    FM_H745 = 7
};

struct fm_operation
{
    struct fm_io io;
    uint32_t address, span, checked, control, value;
    unsigned profile, kind, state;
};
enum fh_result fm_erase(struct fm_operation *, struct fm_io, enum fm_profile,
                        unsigned sector);
enum fh_result fm_f439_program(struct fm_operation *, struct fm_io,
                               uint32_t address, uint32_t value);
enum fh_result fm_poll(struct fm_operation *);
struct fm_io fm_f439_io(void);
struct fm_io fm_h745_io(void);

/* Copy validated new joint record to inactive sector6/7, validate it, then erase
 * old sector. Caller has authenticated images and owns both sectors/peers. */
struct fm_recycle
{
    struct fm_operation erase;
    struct fh_append append;
    struct fm_io io;
    uint32_t old_base, new_base;
    unsigned state;
    unsigned char record[128];
};
enum fh_result fm_recycle_begin(struct fm_recycle *, struct fm_io,
                                uint32_t old_base, const struct fw_peer *,
                                uint32_t sequence, const unsigned char[64],
                                const uint32_t[2], const uint32_t[2]);
enum fh_result fm_recycle_poll(struct fm_recycle *);
/* Readable, fault-contained snapshots only. Highest sequence wins; unequal
 * records with equal sequences are corruption, never an arbitrary tie-break. */
int fm_recover(const unsigned char a[128], const unsigned char b[128]);
#endif
