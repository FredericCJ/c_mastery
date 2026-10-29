#ifndef KF_DEVICE_H
#define KF_DEVICE_H
#include <stddef.h>
#include <stdint.h>

struct kf_bar
{
    uint64_t base, size;
    unsigned memory;
};

struct kf_pci
{
    uint64_t common, notify, device;
    uint32_t notify_size, multiplier;
    unsigned visited;
};

/* Numeric capability snapshot; supplied BAR apertures are trusted
 * bus-enumerator authority, not inferred from the capability's own advertised
 * length. */
int kf_pci(const uint8_t config[256], const struct kf_bar bars[6],
           struct kf_pci *out);
int kf_negotiate(uint64_t features, unsigned queue_size, uint16_t offset,
                 const struct kf_pci *, uint64_t *notify);

enum
{
    KF_FREE,
    KF_SUBMITTED,
    KF_SUCCESS,
    KF_DEVICE_ERROR,
    KF_UNKNOWN
};

struct kf_header
{
    uint32_t type, reserved;
    uint64_t sector;
};

struct kf_descriptor
{
    uint64_t address;
    uint32_t length;
    uint16_t flags, next;
};

struct kf_request
{
    struct kf_header header;
    uint8_t bytes[4096], status;
    unsigned state;
    uint32_t generation;
};

struct kf_queue
{
    struct kf_request requests[2];
    struct kf_descriptor descriptors[8];
    uint16_t available, used, ring[8];
    unsigned poisoned;
    uint64_t completed_writes, stable_writes;
};

struct kf_ticket
{
    unsigned slot;
    uint32_t generation;
};

void kf_queue_init(struct kf_queue *, uint16_t);
int kf_submit(struct kf_queue *, unsigned type, uint64_t sector,
              const uint8_t *, struct kf_ticket *);
int kf_complete(struct kf_queue *, uint16_t used_index, uint32_t wire_head,
                uint8_t status);
int kf_timeout(struct kf_queue *, struct kf_ticket);
int kf_release(struct kf_queue *, struct kf_ticket);
/* Only externally observed device-reset acknowledgement authorizes reclamation.
 * The model cannot establish hardware quiescence by setting this argument. */
int kf_reset(struct kf_queue *, int reset_acknowledged);
int kf_page_sector(uint64_t raw_sectors, unsigned sector_bytes, uint32_t page,
                   uint64_t *sector);
int kf_slice(uint64_t raw_pages, uint32_t base, uint32_t count,
             uint32_t logical, uint32_t *physical);
#endif
