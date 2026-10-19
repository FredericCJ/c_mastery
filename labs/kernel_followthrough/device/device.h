#ifndef KF_DEVICE_H
#define KF_DEVICE_H
#include <stddef.h>
#include <stdint.h>
typedef struct {
  uint64_t base, size;
  unsigned memory;
} KfBar;
typedef struct {
  uint64_t common, notify, device;
  uint32_t notify_size, multiplier;
  unsigned visited;
} KfPci;
/* Numeric capability snapshot; supplied BAR apertures are trusted
 * bus-enumerator authority, not inferred from the capability's own advertised
 * length. */
int kf_pci(const uint8_t config[256], const KfBar bars[6], KfPci *out);
int kf_negotiate(uint64_t features, unsigned queue_size, uint16_t offset,
                 const KfPci *, uint64_t *notify);
enum { KF_FREE, KF_SUBMITTED, KF_SUCCESS, KF_DEVICE_ERROR, KF_UNKNOWN };
typedef struct {
  uint32_t type, reserved;
  uint64_t sector;
} KfHeader;
typedef struct {
  uint64_t address;
  uint32_t length;
  uint16_t flags, next;
} KfDescriptor;
typedef struct {
  KfHeader header;
  uint8_t bytes[4096], status;
  unsigned state;
  uint32_t generation;
} KfRequest;
typedef struct {
  KfRequest requests[2];
  KfDescriptor descriptors[8];
  uint16_t available, used, ring[8];
  unsigned poisoned;
  uint64_t completed_writes, stable_writes;
} KfQueue;
typedef struct {
  unsigned slot;
  uint32_t generation;
} KfTicket;
void kf_queue_init(KfQueue *, uint16_t);
int kf_submit(KfQueue *, unsigned type, uint64_t sector, const uint8_t *,
              KfTicket *);
int kf_complete(KfQueue *, uint16_t used_index, uint32_t wire_head,
                uint8_t status);
int kf_timeout(KfQueue *, KfTicket);
int kf_release(KfQueue *, KfTicket);
/* Only externally observed device-reset acknowledgement authorizes reclamation.
 * The model cannot establish hardware quiescence by setting this argument. */
int kf_reset(KfQueue *, int reset_acknowledged);
int kf_page_sector(uint64_t raw_sectors, unsigned sector_bytes, uint32_t page,
                   uint64_t *sector);
int kf_slice(uint64_t raw_pages, uint32_t base, uint32_t count,
             uint32_t logical, uint32_t *physical);
#endif
