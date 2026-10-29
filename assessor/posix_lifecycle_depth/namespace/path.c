#define _GNU_SOURCE
#include "path.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/openat2.h>
#include <sys/syscall.h>
#include <unistd.h>
#ifndef PN_MUTANT
#define PN_MUTANT 0
#endif
int pn_open_confined(int directory, const char *relative, int *owned_fd)
{
    int fd;
    struct stat info;
    if (!relative || !owned_fd || relative[0] == '\0')
        return EINVAL;
#if PN_MUTANT == 1
    fd = openat(directory, relative,
                O_RDONLY | O_CLOEXEC | O_NONBLOCK | O_NOFOLLOW);
#elif PN_MUTANT == 2
    fd = openat(directory, relative, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
#else
    struct open_how how = {0};
    how.flags = O_RDONLY | O_CLOEXEC | O_NONBLOCK;
    how.resolve = RESOLVE_BENEATH | RESOLVE_NO_SYMLINKS | RESOLVE_NO_MAGICLINKS;
    fd = (int)syscall(SYS_openat2, directory, relative, &how, sizeof how);
#endif
    if (fd < 0)
        return errno; /* ENOSYS/EAGAIN never weakens the policy. */
    if (fstat(fd, &info) < 0)
    {
        int error = errno;
        close(fd);
        return error;
    }
    if (!S_ISREG(info.st_mode))
    {
        close(fd);
        return EINVAL;
    }
    *owned_fd = fd;
    return 0;
}

int pn_info(int directory, const char *path, bool follow, struct pn_info *out)
{
    struct stat s;
    struct pn_info result;
    if (!path || !out)
        return EINVAL;
#if PN_MUTANT == 3
    follow = true;
#endif
    if (fstatat(directory, path, &s, follow ? 0 : AT_SYMLINK_NOFOLLOW) < 0)
        return errno;
    result = (struct pn_info){.device = s.st_dev,
                              .inode = s.st_ino,
                              .mode = s.st_mode,
                              .owner = s.st_uid,
                              .group = s.st_gid,
                              .links = s.st_nlink,
                              .logical_bytes = s.st_size,
                              .blocks_512 = s.st_blocks,
                              .preferred_io = s.st_blksize,
                              .access = s.st_atim,
                              .modification = s.st_mtim,
                              .status_change = s.st_ctim};
#if PN_MUTANT == 4
    result.blocks_512 = s.st_size;
#endif
    *out = result;
    return 0;
}

int pn_creation_mode(unsigned requested, unsigned mask, bool has_default,
                     unsigned permissions, unsigned *installed)
{
    if (!installed || (requested | mask | permissions) > 0777u)
        return EINVAL;
#if PN_MUTANT == 5
    has_default = false;
#endif
    *installed = requested & (has_default ? permissions : (~mask & 0777u));
    return 0;
}

int pn_authorize(struct pn_authority a, enum pn_operation operation)
{
    if (operation < PN_READ || operation > PN_REPLACE)
        return EINVAL;
    if (!a.searchable)
        return EACCES;
    if (operation == PN_READ)
        return a.file_read ? 0 : EACCES;
    if (operation == PN_WRITE)
        return a.file_write ? 0 : EACCES;
#if PN_MUTANT == 6
    return a.file_write ? 0 : EACCES;
#else
    return a.parent_write ? 0 : EACCES;
#endif
}
