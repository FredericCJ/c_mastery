#include "entry.h"
#include "checkpoint_writer.h"
volatile unsigned cpw_bss;
volatile unsigned cpw_data = 0x13579bdu;
#ifdef CE_BAD_RELOCATION
volatile unsigned *volatile ce_absolute = &cpw_data;
#endif
struct cpw_large
{
    unsigned char bytes[65536];
};

void cpw_copy(struct cpw_large *, const struct cpw_large *);
static struct cpw_large from, to;

static long raw_write(long fd, const unsigned char *p, size_t n)
{
    long r;
    __asm__ volatile("syscall"
                     : "=a"(r)
                     : "a"(1L), "D"(fd), "S"(p), "d"(n)
                     : "rcx", "r11", "memory");
    return r;
}

static struct cpw_result writer(void *c, const unsigned char *p, size_t n)
{
    (void)c;
    return cpw_raw(raw_write(1, p, n));
}

static int is_test(const unsigned char *p)
{
    const unsigned char name[] = "CE_TEST=";
    for (unsigned i = 0; i < 8; ++i)
        if (p[i] != name[i])
            return 0;
    return p[8] && !p[9] ? p[8] : 0;
}

static void hex(unsigned char *p, uintptr_t n)
{
    for (unsigned i = 0; i < 16; ++i)
    {
        unsigned d = (unsigned)(n & 15);
        p[15 - i] = (unsigned char)(d < 10 ? '0' + d : 'a' + d - 10);
        n >>= 4;
    }
}

int cpw_image(uintptr_t *initial)
{
    size_t lengths[2] = {0, 0}, argc, envc = 0;
    uintptr_t *env, *aux, pages = 0, entry = 0, phdr = 0;
    unsigned test = 0;
    if (((uintptr_t)initial & 15) || cpw_bss || cpw_data != 0x13579bdu)
        return 70;
    argc = (size_t)initial[0];
    if (argc < 1 || argc > 3 || initial[argc + 1])
        return 70;
    for (size_t a = 1; a < argc; ++a)
    {
        const unsigned char *p = (const unsigned char *)initial[a + 1];
        while (lengths[a - 1] <= 32 && p[lengths[a - 1]])
            ++lengths[a - 1];
        if (lengths[a - 1] > 32)
            return 71;
    }
    env = initial + argc + 2;
    while (env[envc])
    {
        if (envc == 64)
            return 72;
        int value = is_test((const unsigned char *)env[envc]);
        if (value)
            test = (unsigned)value;
        ++envc;
    }
    aux = env + envc + 1;
    for (unsigned i = 0; i < 64; ++i)
    {
        uintptr_t key = aux[2 * i], value = aux[2 * i + 1];
        if (!key)
            break;
        if (key == 3)
            phdr = value;
        else if (key == 6)
            pages = value;
        else if (key == 9)
            entry = value;
        if (i == 63)
            return 72;
    }
    if (!pages || (pages & (pages - 1)) || !entry || !phdr || !test)
        return 72;
    for (size_t i = 0; i < sizeof from.bytes; ++i)
        from.bytes[i] = (unsigned char)(i * 13u + 7u);
    cpw_copy(&to, &from);
    for (size_t i = 0; i < sizeof from.bytes; ++i)
        if (to.bytes[i] != (unsigned char)(i * 13u + 7u))
            return 73;
    for (size_t a = 1; a < argc; ++a)
    {
        struct cpw_transfer t;
        const unsigned char *p = (const unsigned char *)initial[a + 1];
        if (cpw_begin(&t, p, lengths[a - 1], (unsigned)a) == CPW_INVALID)
            return 74;
        for (unsigned turns = 0; turns < 16 && t.status == CPW_PENDING; ++turns)
            (void)cpw_step(&t, writer, 0);
        if (t.status != CPW_DONE || t.accepted != lengths[a - 1])
            return 74;
    }
    if (cpw_raw(raw_write(-1, (const unsigned char *)initial[1], 1)).cause != 9)
        return 75;
    /* Fixed-width identity fields let the host independently derive load bias. */
    unsigned char report[90];
    report[0] = 'C';
    report[1] = 'E';
    report[2] = ':';
    hex(report + 3, argc);
    report[19] = ':';
    hex(report + 20, envc);
    report[36] = ':';
    hex(report + 37, test);
    report[53] = ':';
    hex(report + 54, (uintptr_t)&cpw_data);
    report[70] = ':';
    hex(report + 71, entry);
    report[87] = '\n';
    if (raw_write(2, report, 88) != 88)
        return 76;
    return 0;
}
