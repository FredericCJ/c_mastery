#ifndef CMASTERY_RESILIENCE_JOURNAL_POSIX_H
#define CMASTERY_RESILIENCE_JOURNAL_POSIX_H
#include "journal.h"

struct rzj_posix
{
    int descriptor;
    size_t max_io;
};

int rzj_posix(int, size_t, struct rzj_posix *, struct rzj_io *);
#endif
