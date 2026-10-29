#include "linux_raw.h"

struct sd_result sd_fd_backend(void *context, unsigned char *bytes, size_t n)
{
    struct sd_fd *f = context;
    return sd_linux_result(
        sd_raw6(f->reading ? 0 : 1, f->fd, (uintptr_t)bytes, n, 0, 0, 0));
}
