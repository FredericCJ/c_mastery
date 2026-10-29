#include "linux_raw.h"
uintptr_t sd_original_sp, sd_call_sp, sd_helper_sp;
static unsigned char payload[4096];
static unsigned initialized = 19;
static unsigned *relocated = &initialized;
static unsigned hook_state;

static void init_hook(void)
{
    hook_state = 23;
}

int sd_entry(const uintptr_t *entry)
{
    /* Check instrumentation before reading a possibly wrong saved pointer. */
    if ((uintptr_t)entry != sd_original_sp)
        return 71;
    if ((sd_call_sp & 15) != 0 || sd_helper_sp + 8 != sd_call_sp)
        return 72;
#ifdef SD_EARLY_FAILURE
    return 73;
#endif
#ifdef SD_REPAIR_RELOCATION
    relocated = &initialized;
#endif
    /* Reject the original unrelocated static-PIE slot before dereference. */
    if (relocated != &initialized)
        return 86;
    if (initialized != 19 || *relocated != 19 || hook_state != 0)
        return 74;
#ifdef SD_INIT_HOOK
    init_hook();
    if (hook_state != 23)
        return 75;
#else
    (void)init_hook;
#endif
    /* This adapter reads the kernel-created original process table. Arbitrary
     * byte fixtures go through the bounded decoder, never through this scan. */
    size_t n = (size_t)entry[0];
    if (n > 256)
        return 76;
    size_t pos = n + 2;
    if (entry[n + 1] != 0)
        return 77;
    for (size_t count = 0; entry[pos] != 0; ++count, ++pos)
        if (count == 256)
            return 78;
    ++pos;
    for (size_t count = 0; entry[pos] != 0; ++count, pos += 2)
        if (count == 64)
            return 79;
    pos += 2;
    struct sd_stack view;
    if (sd_stack_decode((const unsigned char *)entry, pos * sizeof(*entry), 8,
                        &view) != SD_OK)
        return 80;
    for (size_t i = 0; i < sizeof(payload); ++i)
        payload[i] = (unsigned char)('A' + i % 26);
    struct sd_transfer t;
    struct sd_fd fd = {1, 0};
    if (sd_transfer_init(&t, payload, sizeof(payload), 0) != SD_OK)
        return 81;
    if (sd_transfer_step(&t, sd_fd_backend, &fd, 16, 0) != SD_DONE)
        return 82;
#if defined(SD_DECIMAL) || defined(SD_PARSE)
    uint32_t value = 0;
    static const unsigned char maximum[] = "4294967295";
    if (sd_parse_u32(maximum, 10, &value) != SD_OK)
        return 83;
#endif
#ifdef SD_DECIMAL
    unsigned char buffer[11];
    size_t length = 0;
#ifdef SD_NODIV
    if (sd_format_u32_nodiv(value, buffer, sizeof(buffer), 0, &length) != SD_OK)
        return 84;
#else
    if (sd_format_u32(value, buffer, sizeof(buffer), 0, &length) != SD_OK)
        return 84;
#endif
    if (sd_transfer_init(&t, buffer, length, 0) != SD_OK ||
        sd_transfer_step(&t, sd_fd_backend, &fd, 16, 0) != SD_DONE)
        return 85;
#endif
    return 42;
}
