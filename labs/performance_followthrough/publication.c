#define _POSIX_C_SOURCE 200809L
#include "publication.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
int pf_sync_system(void *unused, int fd) { (void)unused; return fsync(fd); }
struct pf_publication pf_publish(int stage, int directory, const char *source,
                          const char *target, struct pf_synchronizer sync) {
    struct pf_publication result = {PF_STAGE_RETAINED, 0, 0};
    if (!source || !target || stage < 0 || directory < 0 || !sync.synchronize) {
        result.error = EINVAL;
        return result;
    }
    ++result.calls;
    if (sync.synchronize(sync.context, stage)) {
        result.error = errno;
        return result;
    }
    if (rename(source, target)) { result.error = errno; return result; }
    result.state = PF_VISIBLE_UNCONFIRMED;
    ++result.calls;
    if (sync.synchronize(sync.context, directory)) {
        result.error = errno;
        return result;
    }
    result.state = PF_SYNCED;
    return result;
}
