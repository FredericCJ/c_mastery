#ifndef ARCHITECTURE_PIPELINE_H
#define ARCHITECTURE_PIPELINE_H
#include <stddef.h>
#include <stdint.h>
#include "systems.h"

enum ap_status
{
    AP_OK,
    AP_INVALID,
    AP_LIMIT
};

struct ap_instruction
{
    unsigned op, dst, left, right, target, taken;
};

struct ap_config
{
    unsigned forwarding, memory_cycles, write_first, branch_in_id, unified_port;
};

struct ap_schedule
{
    unsigned cycles, instances, retired_count, flushed_count, data_stalls,
        structural_stalls, port_stalls;
    int slots[128][5];
    unsigned pc[64], flush_cycle[64], resolve_cycle[64], retired[64],
        retire_cycle[64];
};

struct ap_ooo
{
    unsigned cycles, issue[16], complete[16], retire[16];
    uint8_t registers[8], value[16];
    int left_tag[16], right_tag[16];
};
enum ap_status ap_schedule(const struct ap_instruction *, size_t,
                           struct ap_config, struct ap_schedule *);
enum ap_status ap_ooo(const struct ap_instruction *, size_t, const uint8_t[8],
                      const uint8_t[16], unsigned, struct ap_ooo *);
int ap_test(const char *);
void ap_failure(const char *, unsigned);
void ap_emit(const char *, const int *, unsigned);
#endif
