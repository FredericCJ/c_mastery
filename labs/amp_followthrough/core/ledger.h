#ifndef AF_LEDGER_H
#define AF_LEDGER_H
#include "amp.h"

enum
{
    AF_ACCEPTED = 1,
    AF_DONE,
    AF_DROPPED,
    AF_UNCERTAIN
};

struct af_record
{
    uint32_t epoch, state;
};

struct af_ledger
{
    uint32_t epoch, last;
    struct af_record item[128];
};

int af_admit(struct af_ledger *, uint32_t, uint32_t);
int af_terminal(struct af_ledger *, uint32_t, uint32_t, unsigned);
int af_ledger_restart(struct af_ledger *, unsigned, unsigned);
#endif
