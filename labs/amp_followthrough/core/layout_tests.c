#include "amp.h"
#include <stdio.h>

struct af_wide
{
    uint32_t epoch;
    uint64_t sequence;
    uint32_t length;
    unsigned char bytes[32];
};

enum
{
    WIDE_LAYOUT = 0x41463228u
};

static int wide_read(struct af_boot *b, const struct af_wide *payload,
                     uint64_t *out, unsigned *reads)
{
    int r = af_boot_ack(b, 2, WIDE_LAYOUT, 1);
    if (r != AF_OK)
    {
        return r;
    }
    (*reads)++;
    *out = payload->sequence;
    return AF_OK;
}

int main(void)
{
    struct af_boot b = {
        .ready = 1, .version = 2, .layout = AF_LAYOUT, .epoch = 1};
    struct af_wide changed = {
        .epoch = 1, .sequence = UINT64_C(0x100000001), .length = 0};
    uint64_t out = 99;
    unsigned reads = 0;
    if (offsetof(struct af_wide, sequence) ==
            offsetof(struct af_message, sequence) ||
        sizeof(struct af_wide) == sizeof(struct af_message))
    {
        return 2;
    }
    if (wide_read(&b, &changed, &out, &reads) != AF_STALE || reads || out != 99)
    {
        return 1;
    }
    atomic_store(&b.layout, WIDE_LAYOUT);
    if (wide_read(&b, &changed, &out, &reads) != AF_OK || reads != 1 ||
        out != UINT64_C(0x100000001))
    {
        return 1;
    }
    printf(
        "AF_PASS:layout old-size%zu wide-size%zu old-sequence-offset%zu wide-offset%zu rejected-before-payload-read\n",
        sizeof(struct af_message), sizeof(struct af_wide),
        offsetof(struct af_message, sequence),
        offsetof(struct af_wide, sequence));
    return 0;
}
