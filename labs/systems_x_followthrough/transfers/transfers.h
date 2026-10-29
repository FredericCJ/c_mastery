#ifndef SYSTEMS_X_TRANSFERS_H
#define SYSTEMS_X_TRANSFERS_H
#include "b04.h"
#include "b05.h"
#include "systems.h"

struct sx_map
{
    struct b5_page pages[4];
    unsigned version[4];
};

typedef void (*sx_between_fn)(struct sx_map *, void *);
int sx_remap(struct sx_map *, unsigned page, unsigned present, unsigned user,
             unsigned writable, unsigned char fill);
int sx_copy(struct sx_map *, size_t, const unsigned char *, size_t,
            sx_between_fn, void *);

struct sx_copy_result
{
    size_t copied;
    int status;
};

int sx_partial(struct sx_map *, size_t, const unsigned char *, size_t,
               struct sx_copy_result *);

struct sx_alias
{
    unsigned slot, generation;
    uint32_t identity;
};

struct sx_ledger
{
    struct b4_ledger ledger;
    unsigned generation[4], owner[4], allocator[4];
    size_t attributed[2];
};

int sx_allocate(struct sx_ledger *, uint32_t, size_t, unsigned owner,
                unsigned allocator, struct sx_alias *);
int sx_resize(struct sx_ledger *, struct sx_alias, unsigned actor, size_t,
              int succeeded, struct sx_alias *);
int sx_transfer(struct sx_ledger *, struct sx_alias, unsigned old_owner,
                unsigned new_owner);
int sx_release(struct sx_ledger *, struct sx_alias, unsigned actor,
               unsigned allocator);
int sx_lookup(const struct sx_ledger *, struct sx_alias, unsigned actor,
              size_t *);

struct sx_hotplug
{
    struct s_shootdown request;
    unsigned online, draining, quiescent;
    uint32_t incarnation[8], pending_incarnation[8];
};

int sx_online(struct sx_hotplug *, unsigned cpu);
int sx_begin(struct sx_hotplug *, unsigned targets);
int sx_offline_request(struct sx_hotplug *, unsigned cpu, uint32_t incarnation);
int sx_offline_quiescent(struct sx_hotplug *, unsigned cpu,
                         uint32_t incarnation);
int sx_ack(struct sx_hotplug *, unsigned cpu, uint32_t incarnation,
           uint32_t generation);
int sx_reclaim(struct sx_hotplug *);
#endif
