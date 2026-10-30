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

/* The nine permission bits (rwx for user, group, other). */
enum
{
    PN_PERMISSION_MASK = 0777
};

/* Opens `relative` for reading without ever leaving `directory`.
   openat2() with RESOLVE_BENEATH rejects absolute paths, a ".." component
   that escapes `directory`, and symlinks that escape it (a path such as
   "a/../a/b" that stays inside still resolves); RESOLVE_NO_SYMLINKS and RESOLVE_NO_MAGICLINKS also
   reject links inside the tree. A plain openat() cannot express this, which
   is why the weaker variants below are mutants. O_NONBLOCK keeps a FIFO from
   blocking the open; non-regular files are rejected by the caller anyway.
   Returns the descriptor or -1 with errno set. */
static int open_beneath(int directory, const char *relative)
{
#if PN_MUTANT == 1
    return openat(directory, relative,
                  O_RDONLY | O_CLOEXEC | O_NONBLOCK | O_NOFOLLOW);
#elif PN_MUTANT == 2
    return openat(directory, relative, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
#else
    struct open_how how = {0};
    how.flags = O_RDONLY | O_CLOEXEC | O_NONBLOCK;
    how.resolve = RESOLVE_BENEATH | RESOLVE_NO_SYMLINKS | RESOLVE_NO_MAGICLINKS;
    return (int)syscall(SYS_openat2, directory, relative, &how, sizeof how);
#endif
}

/* Contract: `directory` is borrowed. On success *owned_fd is a new
   regular-file descriptor owned by the caller; on failure it is untouched
   and no descriptor is left open. Returns a positive errno. */
int pn_open_confined(int directory, const char *relative, int *owned_fd)
{
    if (!relative || !owned_fd || relative[0] == '\0')
    {
        return EINVAL;
    }
    int fd = open_beneath(directory, relative);
    if (fd < 0)
    {
        /* ENOSYS/EAGAIN never weakens the policy. */
        return errno;
    }
    struct stat info;
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

/* st_blocks counts 512-byte units whatever st_blksize says, so blocks_512 is
   unrelated to preferred_io; logical_bytes is the file size. */
int pn_info(int directory, const char *path, bool follow, struct pn_info *out)
{
    if (!path || !out)
    {
        return EINVAL;
    }
#if PN_MUTANT == 3
    follow = true;
#endif
    struct stat st;
    if (fstatat(directory, path, &st, follow ? 0 : AT_SYMLINK_NOFOLLOW) < 0)
    {
        return errno;
    }
    struct pn_info result = {.device = st.st_dev,
                             .inode = st.st_ino,
                             .mode = st.st_mode,
                             .owner = st.st_uid,
                             .group = st.st_gid,
                             .links = st.st_nlink,
                             .logical_bytes = st.st_size,
                             .blocks_512 = st.st_blocks,
                             .preferred_io = st.st_blksize,
                             .access = st.st_atim,
                             .modification = st.st_mtim,
                             .status_change = st.st_ctim};
#if PN_MUTANT == 4
    result.blocks_512 = st.st_size;
#endif
    *out = result;
    return 0;
}

/* Restricted default-ACL model: with a default entry the new file gets
   requested & default_permissions (the umask is ignored); without one it
   gets requested & ~umask. */
int pn_creation_mode(unsigned requested, unsigned mask, bool has_default,
                     unsigned permissions, unsigned *installed)
{
    if (!installed || (requested | mask | permissions) > PN_PERMISSION_MASK)
    {
        return EINVAL;
    }
#if PN_MUTANT == 5
    has_default = false;
#endif
    *installed =
        requested & (has_default ? permissions : (~mask & PN_PERMISSION_MASK));
    return 0;
}

/* Search permission on the path comes first for every operation; then the
   operation decides which further permission is needed. Replacing a name
   needs write access to the parent directory, not to the file. */
int pn_authorize(struct pn_authority a, enum pn_operation operation)
{
    if (operation < PN_READ || operation > PN_REPLACE)
    {
        return EINVAL;
    }
    if (!a.searchable)
    {
        return EACCES;
    }
    if (operation == PN_READ)
    {
        return a.file_read ? 0 : EACCES;
    }
    if (operation == PN_WRITE)
    {
        return a.file_write ? 0 : EACCES;
    }
#if PN_MUTANT == 6
    return a.file_write ? 0 : EACCES;
#else
    return a.parent_write ? 0 : EACCES;
#endif
}
