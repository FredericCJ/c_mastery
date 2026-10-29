#ifndef PN_PATH_H
#define PN_PATH_H
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

/* Linux hosted contract. All errors are positive errno values; outputs remain
 * unchanged on failure. Descriptors supplied by callers remain borrowed. */
struct pn_info
{
    dev_t device;
    ino_t inode;
    mode_t mode;
    uid_t owner;
    gid_t group;
    nlink_t links;
    off_t logical_bytes;
    blkcnt_t blocks_512;
    intmax_t preferred_io;
    struct timespec access, modification, status_change;
};

int pn_open_confined(int directory, const char *relative, int *owned_fd);
int pn_info(int directory, const char *path, bool follow, struct pn_info *out);
/* Deliberately restricted three-entry default-ACL model, not an ACL engine. */
int pn_creation_mode(unsigned requested, unsigned mask, bool has_default,
                     unsigned default_permissions, unsigned *installed);

enum pn_operation
{
    PN_READ,
    PN_WRITE,
    PN_REPLACE
};

struct pn_authority
{
    bool searchable, file_read, file_write, parent_write;
};

/* Serialized teaching adapter: no privileges, named ACLs, sticky directories,
 * immutable flags or read-only mounts. REPLACE requires an existing entry. */
int pn_authorize(struct pn_authority authority, enum pn_operation operation);
#endif
