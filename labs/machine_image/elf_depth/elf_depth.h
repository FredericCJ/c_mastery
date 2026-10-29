#ifndef CM_ELF_DEPTH_H
#define CM_ELF_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum ed_status
{
    ED_OK,
    ED_ARGUMENT,
    ED_FORMAT,
    ED_RANGE,
    ED_UNSUPPORTED
};

/* Borrowed immutable file, <=16MiB, <=4096 sections/program headers, <=16384
 * total symbols, <=1024 bytes/name and <=8MiB aggregate name scanning. No allocation.
 * Accessors require a successful view and an unchanged live file. View/output
 * objects are disjoint from file storage. Output is
 * unchanged on failure. Supported subset: ELF32/64, little/big endian, ordinary
 * counts; extended indexes/counts explicitly unsupported. This is an inspector,
 * not a loader, relocator, signature verifier or full ELF conformance validator. */
struct ed_view
{
    const unsigned char *bytes;
    size_t length;
    unsigned bits, order, type, machine;
    uint64_t entry, shoff, phoff;
    unsigned shnum, phnum, shstr, shentsize, phentsize;
};

struct ed_section
{
    uint32_t name, type, link, info;
    uint64_t flags, address, offset, size, align, entsize;
};

struct ed_program
{
    uint32_t type, flags;
    uint64_t offset, address, physical, filesz, memsz, align;
};

struct ed_symbol
{
    uint32_t name;
    unsigned info, other, section;
    uint64_t value, size;
};
enum ed_status ed_open(const unsigned char *, size_t, struct ed_view *);
enum ed_status ed_section(const struct ed_view *, unsigned,
                          struct ed_section *);
enum ed_status ed_program(const struct ed_view *, unsigned,
                          struct ed_program *);
enum ed_status ed_symbol(const struct ed_view *, unsigned, size_t,
                         struct ed_symbol *);
enum ed_status ed_name(const struct ed_view *, unsigned, uint32_t,
                       const char **, size_t *);
#endif
