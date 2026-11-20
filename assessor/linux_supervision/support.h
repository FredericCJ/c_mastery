#ifndef C_MASTERY_SUPERVISION_SUPPORT_H
#define C_MASTERY_SUPERVISION_SUPPORT_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

_Noreturn void fail_sys(const char *operation);
_Noreturn void fail_protocol(const char *operation);
int64_t now_ms(void);
void await_fd(int fd, short events, int64_t deadline);
void read_full(int fd, void *data, size_t size, int64_t deadline);
void write_full(int fd, const void *data, size_t size, int64_t deadline);
int collect(pid_t child, int64_t deadline);
void read_field(const char *path, char *out, size_t capacity);
void leaf_membership(char *out, size_t capacity);
void leaf_field(const char *leaf, const char *name, char *out, size_t capacity);
unsigned long long leaf_counter(const char *leaf, const char *file, const char *key);
unsigned long long namespace_inode(const char *name);
int count_fds(void);

#endif
