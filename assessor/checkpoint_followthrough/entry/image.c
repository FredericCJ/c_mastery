/*
 * Reference process-entry adapter for the custom x86-64 Linux entry packet.
 *
 * cpw_image() receives the initial stack pointer captured by _start before any
 * adjustment. It addresses, in order: argc, argc argument pointers, a null
 * slot, the environment pointers, a null slot, then auxiliary-vector
 * (key, value) pairs ended by AT_NULL. The adapter trusts that this table was
 * built by the Linux loader; the loop bounds below limit work, they do not make
 * a forged pointer valid.
 *
 * Return value: 0 on success, otherwise one of the CE_FAIL_* exit codes. All
 * input validation completes before the first byte reaches stdout.
 */
#include "entry.h"
#include "checkpoint_writer.h"

enum ce_limit
{
    CE_STACK_ALIGNMENT_MASK = 15,
    CE_MAX_ARGC = 3,
    CE_MAX_PAYLOAD_BYTES = 32,
    CE_MAX_ENVIRONMENT = 64,
    CE_MAX_AUXV_PAIRS = 64,
    CE_MAX_TURNS_PER_ARGUMENT = 16,
    CE_HEX_DIGITS = 16,
    CE_REPORT_BYTES = 88
};

/*
 * Process exit codes; the host harness matches these values exactly.
 * CE_FAIL_STACK also covers the .bss/.data sentinels and argc.
 * CE_FAIL_ENVIRONMENT also covers auxv faults and a missing or
 * non-power-of-two page size.
 */
enum ce_failure
{
    CE_FAIL_STACK = 70,
    CE_FAIL_PAYLOAD_LENGTH = 71,
    CE_FAIL_ENVIRONMENT = 72,
    CE_FAIL_COPY = 73,
    CE_FAIL_TRANSFER = 74,
    CE_FAIL_EBADF_PROBE = 75,
    CE_FAIL_REPORT = 76
};

/* Auxiliary-vector keys read from the Linux x86-64 initial stack. */
enum ce_auxv_key
{
    CE_AT_NULL = 0,
    CE_AT_PHDR = 3,
    CE_AT_PAGESZ = 6,
    CE_AT_ENTRY = 9
};

#define CE_EBADF 9
#define CE_SYS_WRITE 1L
#define CE_DATA_SENTINEL 0x13579bdu

/* Distinct sentinels: the loader must supply zero .bss and stored .data. */
volatile unsigned cpw_bss;
volatile unsigned cpw_data = CE_DATA_SENTINEL;
#ifdef CE_BAD_RELOCATION
/* An absolute stored pointer forces a dynamic relocation the image cannot do. */
volatile unsigned *volatile ce_absolute = &cpw_data;
#endif

struct cpw_large
{
    unsigned char bytes[65536];
};

/*
 * Provided by the aggregate probe object (labs/checkpoint_depth/tests/
 * helper_probe.c), not by libc; byte_helpers.c supplies memcpy and memset.
 */
void cpw_copy(struct cpw_large *, const struct cpw_large *);
static struct cpw_large copy_source;
static struct cpw_large copy_destination;

/* Values recovered from the initial stack. Lengths are in bytes. */
struct ce_startup
{
    size_t argc;
    size_t payload_bytes[2];
    size_t environment_count;
    unsigned test_code;
    uintptr_t page_size;
    uintptr_t entry;
    uintptr_t program_headers;
};

/* Raw write(2); returns the kernel result, a negative errno on failure. */
static long raw_write(long fd, const unsigned char *bytes, size_t count)
{
    long result;
    __asm__ volatile("syscall"
                     : "=a"(result)
                     : "a"(CE_SYS_WRITE), "D"(fd), "S"(bytes), "d"(count)
                     : "rcx", "r11", "memory");
    return result;
}

static struct cpw_result stdout_writer(void *context, const unsigned char *p,
                                       size_t n)
{
    (void)context;
    return cpw_raw(raw_write(1, p, n));
}

/*
 * Returns the single character X of an exact "CE_TEST=X" entry, otherwise 0.
 */
static unsigned test_code_of(const unsigned char *entry)
{
    const unsigned char name[] = "CE_TEST=";
    for (unsigned i = 0; i < sizeof name - 1; ++i)
    {
        if (entry[i] != name[i])
        {
            return 0;
        }
    }
    const unsigned char value = entry[sizeof name - 1];
    if (value == 0)
    {
        /* "CE_TEST=" ends here: the byte after this NUL is not ours to read. */
        return 0;
    }
    const unsigned char terminator = entry[sizeof name];
    return (terminator == 0) ? value : 0;
}

/*
 * Counts bytes of text, stopping one past limit so that "too long" is
 * distinguishable without reading the whole (possibly unbounded) string.
 */
static size_t bounded_length(const unsigned char *text, size_t limit)
{
    size_t length = 0;
    while (length <= limit && text[length] != 0)
    {
        ++length;
    }
    return length;
}

/*
 * Checks stack alignment, loader-zeroed .bss, stored .data, the argc range and
 * the null slot that ends argv. Every failure here is CE_FAIL_STACK, including
 * the .bss and .data sentinels and argc.
 */
static int check_entry_state(const uintptr_t *initial)
{
    if (((uintptr_t)initial & CE_STACK_ALIGNMENT_MASK) != 0 || cpw_bss != 0 ||
        cpw_data != CE_DATA_SENTINEL)
    {
        return CE_FAIL_STACK;
    }
    const size_t argc = (size_t)initial[0];
    if (argc < 1 || argc > CE_MAX_ARGC || initial[argc + 1] != 0)
    {
        return CE_FAIL_STACK;
    }
    return 0;
}

/* Measures every payload argument; each must fit in CE_MAX_PAYLOAD_BYTES. */
static int measure_payloads(const uintptr_t *initial, struct ce_startup *s)
{
    for (size_t a = 1; a < s->argc; ++a)
    {
        const unsigned char *text = (const unsigned char *)initial[a + 1];
        const size_t length = bounded_length(text, CE_MAX_PAYLOAD_BYTES);
        if (length > CE_MAX_PAYLOAD_BYTES)
        {
            return CE_FAIL_PAYLOAD_LENGTH;
        }
        s->payload_bytes[a - 1] = length;
    }
    return 0;
}

/* Counts environment entries (bounded) and records the CE_TEST character. */
static int scan_environment(const uintptr_t *environment, struct ce_startup *s)
{
    while (environment[s->environment_count] != 0)
    {
        if (s->environment_count == CE_MAX_ENVIRONMENT)
        {
            return CE_FAIL_ENVIRONMENT;
        }
        const unsigned code = test_code_of(
            (const unsigned char *)environment[s->environment_count]);
        if (code != 0)
        {
            s->test_code = code;
        }
        ++s->environment_count;
    }
    return 0;
}

/*
 * Reads AT_PHDR, AT_PAGESZ and AT_ENTRY. The vector must end with AT_NULL
 * within CE_MAX_AUXV_PAIRS pairs; a missing terminator is refused.
 */
static int scan_auxv(const uintptr_t *auxv, struct ce_startup *s)
{
    for (unsigned i = 0; i < CE_MAX_AUXV_PAIRS; ++i)
    {
        const uintptr_t key = auxv[2 * i];
        const uintptr_t value = auxv[2 * i + 1];
        if (key == CE_AT_NULL)
        {
            return 0;
        }
        if (key == CE_AT_PHDR)
        {
            s->program_headers = value;
        }
        else if (key == CE_AT_PAGESZ)
        {
            s->page_size = value;
        }
        else if (key == CE_AT_ENTRY)
        {
            s->entry = value;
        }
        else
        {
            /* Other keys are irrelevant to this packet. */
        }
    }
    return CE_FAIL_ENVIRONMENT;
}

static unsigned char pattern_byte(size_t index)
{
    return (unsigned char)(index * 13u + 7u);
}

/*
 * Forces a 64 KiB aggregate assignment, which the compiler may lower to a
 * memcpy call resolved by the declared helper, and verifies every byte.
 */
static int check_aggregate_copy(void)
{
    for (size_t i = 0; i < sizeof copy_source.bytes; ++i)
    {
        copy_source.bytes[i] = pattern_byte(i);
    }
    cpw_copy(&copy_destination, &copy_source);
    for (size_t i = 0; i < sizeof copy_destination.bytes; ++i)
    {
        if (copy_destination.bytes[i] != pattern_byte(i))
        {
            return CE_FAIL_COPY;
        }
    }
    return 0;
}

/*
 * Writes each payload argument to stdout through a struct cpw_transfer. A raw
 * write to a blocking stdout pipe can block, so CE_MAX_TURNS_PER_ARGUMENT
 * bounds retries and partial writes, not time: a transfer that is not done
 * within that many steps is a failure.
 */
static int write_payloads(const uintptr_t *initial, const struct ce_startup *s)
{
    for (size_t a = 1; a < s->argc; ++a)
    {
        struct cpw_transfer transfer;
        const unsigned char *text = (const unsigned char *)initial[a + 1];
        const size_t length = s->payload_bytes[a - 1];
        if (cpw_begin(&transfer, text, length, (unsigned)a) == CPW_INVALID)
        {
            return CE_FAIL_TRANSFER;
        }
        for (unsigned turn = 0;
             turn < CE_MAX_TURNS_PER_ARGUMENT && transfer.status == CPW_PENDING;
             ++turn)
        {
            (void)cpw_step(&transfer, stdout_writer, 0);
        }
        if (transfer.status != CPW_DONE || transfer.accepted != length)
        {
            return CE_FAIL_TRANSFER;
        }
    }
    return 0;
}

/* Writes value as CE_HEX_DIGITS lowercase hex digits, most significant first. */
static void format_hex(unsigned char *out, uintptr_t value)
{
    for (unsigned i = 0; i < CE_HEX_DIGITS; ++i)
    {
        const unsigned digit = (unsigned)(value & 15);
        out[CE_HEX_DIGITS - 1 - i] =
            (unsigned char)(digit < 10 ? '0' + digit : 'a' + digit - 10);
        value >>= 4;
    }
}

/* Appends "<16 hex digits><separator>" and returns the next write position. */
static unsigned char *append_field(unsigned char *out, uintptr_t value,
                                   unsigned char separator)
{
    format_hex(out, value);
    out[CE_HEX_DIGITS] = separator;
    return out + CE_HEX_DIGITS + 1;
}

/*
 * Emits "CE:<argc>:<envc>:<test>:<&cpw_data>:<AT_ENTRY>\n" on stderr. Fixed
 * width fields let the host derive the load bias independently.
 */
static int emit_report(const struct ce_startup *s)
{
    unsigned char report[CE_REPORT_BYTES];
    report[0] = 'C';
    report[1] = 'E';
    report[2] = ':';
    unsigned char *at = report + 3;
    at = append_field(at, s->argc, ':');
    at = append_field(at, s->environment_count, ':');
    at = append_field(at, s->test_code, ':');
    at = append_field(at, (uintptr_t)&cpw_data, ':');
    at = append_field(at, s->entry, '\n');
    if (at != report + CE_REPORT_BYTES ||
        raw_write(2, report, CE_REPORT_BYTES) != (long)CE_REPORT_BYTES)
    {
        return CE_FAIL_REPORT;
    }
    return 0;
}

int cpw_image(uintptr_t *initial)
{
    struct ce_startup startup = {0, {0, 0}, 0, 0, 0, 0, 0};
    int failure = check_entry_state(initial);
    if (failure != 0)
    {
        return failure;
    }
    startup.argc = (size_t)initial[0];
    failure = measure_payloads(initial, &startup);
    if (failure != 0)
    {
        return failure;
    }
    uintptr_t *environment = initial + startup.argc + 2;
    failure = scan_environment(environment, &startup);
    if (failure != 0)
    {
        return failure;
    }
    failure = scan_auxv(environment + startup.environment_count + 1, &startup);
    if (failure != 0)
    {
        return failure;
    }
    const uintptr_t page_size = startup.page_size;
    if (page_size == 0 || (page_size & (page_size - 1)) != 0 ||
        startup.entry == 0 || startup.program_headers == 0 ||
        startup.test_code == 0)
    {
        return CE_FAIL_ENVIRONMENT;
    }
    failure = check_aggregate_copy();
    if (failure != 0)
    {
        return failure;
    }
    failure = write_payloads(initial, &startup);
    if (failure != 0)
    {
        return failure;
    }
    /* Writing to descriptor -1 must fail inside the child with EBADF. */
    const long probe = raw_write(-1, (const unsigned char *)initial[1], 1);
    if (cpw_raw(probe).cause != CE_EBADF)
    {
        return CE_FAIL_EBADF_PROBE;
    }
    return emit_report(&startup);
}
