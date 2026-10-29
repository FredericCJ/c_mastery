#ifndef CMASTERY_OBJECT_RUNTIME_CARD_EPOCH_H
#define CMASTERY_OBJECT_RUNTIME_CARD_EPOCH_H
#include "object_runtime.h"
#include <pthread.h>

enum
{
    ORCC_REV_LIMIT = 65535
};

struct orcc_meta
{
    uint32_t revision;
    uint8_t dirty;
};

struct orcc_padded
{
    _Alignas(64) struct orcc_meta meta;
};

struct orc_cards
{
    pthread_mutex_t mutex;
    uint8_t edge[8][2];
    unsigned width, padded;

    union
    {
        struct orcc_meta packed[8];
        struct orcc_padded padded[8];
    } metadata;

    int initialized;
};

struct orcc_snapshot
{
    const struct orc_cards *owner;
    unsigned card, count;
    uint32_t revision;
    uint8_t edge[16];
};
enum or_status orcc_init(struct orc_cards *, unsigned objects_per_card,
                         int padded);
enum or_status orcc_store(struct orc_cards *, unsigned object, unsigned field,
                          unsigned target);
enum or_status orcc_prepare(struct orc_cards *, unsigned card,
                            struct orcc_snapshot *);
enum or_status orcc_scan_commit(struct orc_cards *,
                                const struct orcc_snapshot *, uint16_t *roots,
                                unsigned *scanned);
enum or_status orcc_close(struct orc_cards *);
#endif
