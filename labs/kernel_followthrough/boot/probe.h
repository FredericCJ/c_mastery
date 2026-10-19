#ifndef KF_BOOT_PROBE_H
#define KF_BOOT_PROBE_H
#include <stdint.h>
extern uint64_t kf_entry_rsp;
extern uint64_t boot_pml4[], boot_pdpt[], boot_pd[];
void kf_boot_check(uint32_t magic, uint32_t info);
#endif
