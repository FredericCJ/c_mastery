#ifndef CMASTERY_SMP_DEPTH_ACPI_H
#define CMASTERY_SMP_DEPTH_ACPI_H
#include <stddef.h>
#include <stdint.h>
enum SaStatus {SA_OK,SA_INVALID,SA_CHECKSUM,SA_UNSUPPORTED,SA_DUPLICATE,SA_FULL,SA_EMPTY};
enum SaProfile {SA_XAPIC,SA_X2_RECORDS};
typedef struct {uint32_t uid,apic;unsigned kind;} SaCpu;
typedef struct {uint32_t lapic,flags;unsigned count,disabled,online_capable,unknown;SaCpu cpu[16];} SaTopology;
/* Caller supplies a live byte span, disjoint live output, capacity1..16 and
 * explicit record profile. Output is unchanged on every refusal. Extended
 * record decoding does not enable x2APIC hardware or authorize an IPI target. */
int sa_parse(const void *bytes,size_t available,unsigned capacity,unsigned profile,SaTopology *out);
/* Original finite physical-region adapter: regions remain live and immutable,
 * never overlap in physical address space, and cover complete table extents.
 * No integer address is cast to a C pointer. RSDP revisions 0 and 2 only. */
typedef struct {uint64_t address;const unsigned char *bytes;size_t length;} SaRegion;
int sa_discover(const void *rsdp,size_t available,const SaRegion *regions,unsigned region_count,unsigned capacity,unsigned profile,SaTopology *out);
#endif
