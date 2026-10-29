#ifndef CMASTERY_MATCH_TRANSFER_H
#define CMASTERY_MATCH_TRANSFER_H
#include "lookup.h"

enum
{
    MT_BUSY = 7,
    MT_CLOSED = 8
};

struct mt_stream
{
    struct al_stream prefix;
    uint64_t pending[65];
    size_t count, next;
    unsigned state;
};

struct mt_report
{
    size_t attempted, accepted;
};

int mt_init(struct mt_stream *, const unsigned char *, size_t);
int mt_feed(struct mt_stream *, const unsigned char *, size_t, al_publish_fn,
            void *, struct mt_report *);
int mt_drain(struct mt_stream *, al_publish_fn, void *, struct mt_report *);
int mt_tests(unsigned);
int mt_failure(const char *, unsigned);
void mt_observation(const char *, unsigned, unsigned);
#endif
