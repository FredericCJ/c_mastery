#include "common.h"

int hs_dup_owned(int fd)
{
    return fcntl(fd, F_DUPFD_CLOEXEC, 0);
}
