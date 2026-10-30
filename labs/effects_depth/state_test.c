/*
 * CM-SPECIMEN: hidden-state probes; observe before explaining (see
 * CONTRACTS.md).
 *
 * Each probe is a function that returns 0 when its CHECKs hold. Observations
 * are printed as one JSON line for the campaign to record.
 */
#define _POSIX_C_SOURCE 200809L
#include "effects.h"
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "EFFECTS_STATE_ASSERT:%d:%s\n", __LINE__, #x); \
            return 1;                                                      \
        }                                                                  \
    } while (0)

#define SELECTED_LOCALE_BYTES 64u
#define RAND_SEED 7

static volatile sig_atomic_t pending;

static void handler(int signal_number)
{
    (void)signal_number;
    pending = 1;
}

static const unsigned char *parse_char(unsigned char c)
{
    static unsigned char scratch[2];
    scratch[0] = c;
    scratch[1] = 0;
    return scratch;
}

static int probe_scratch(void)
{
    const unsigned char *first = parse_char('a');
    const unsigned char *second = parse_char('b');
    CHECK(first == second && first[0] == 'b');
    return 0;
}

static int probe_signal(void)
{
    CHECK(signal(SIGUSR1, handler) != SIG_ERR && raise(SIGUSR1) == 0 &&
          pending == 1);
    return 0;
}

static int probe_stdio_position(void)
{
    FILE *stream = tmpfile();
    CHECK(stream);
    CHECK(fputs("ab", stream) >= 0 && ftell(stream) == 2 &&
          fseek(stream, 0, SEEK_SET) == 0 && fgetc(stream) == 'a' &&
          ftell(stream) == 1);
    CHECK(fclose(stream) == 0);
    return 0;
}

/*
 * Formats 12.5 before and after selecting a UTF-8 locale for LC_NUMERIC and
 * copies the selected locale's name into name (name_bytes long).
 */
static int probe_locale(char *name, size_t name_bytes)
{
    char before[64];
    char after[64];
    CHECK(setlocale(LC_ALL, "C"));
    CHECK(snprintf(before, sizeof before, "%.2f", 12.5) > 0);
    const char *selected = setlocale(LC_NUMERIC, "C.utf8");
    if (!selected)
    {
        selected = setlocale(LC_NUMERIC, "C.UTF-8");
    }
    CHECK(selected);
    CHECK(strlen(selected) < name_bytes);
    strcpy(name, selected);
    CHECK(snprintf(after, sizeof after, "%.2f", 12.5) > 0 &&
          strcmp(before, after) == 0);
    return 0;
}

static int probe_rand(void)
{
    srand(RAND_SEED);
    int first = rand();
    srand(RAND_SEED);
    CHECK(rand() == first);
    return 0;
}

/* The inputs needed to replay a parser run: component state plus locale. */
struct replay_snapshot
{
    size_t cursor;
    uint32_t seed;
    uint32_t generation;
    const char *locale_policy;
};

static int probe_replay(void)
{
    struct ef_parser first = {{0}, 0, 7, 1, 0};
    struct ef_parser second = {{0}, 0, 7, 1, 0};
    CHECK(ef_random(&first) == ef_random(&second));

    struct replay_snapshot snapshot = {first.cursor, first.seed,
                                       first.generation, "C"};
    struct ef_view view = {0};
    CHECK(ef_parse(&first, (const unsigned char *)"replay", 6, &view) == EF_OK);
    unsigned char expected[6];
    memcpy(expected, view.data, sizeof expected);
    CHECK(ef_parser_reset(&first, snapshot.seed) == EF_BUSY &&
          ef_view_release(&view) == EF_OK);
    CHECK(ef_parser_reset(&first, snapshot.seed) == EF_OK &&
          first.cursor == snapshot.cursor &&
          first.generation == snapshot.generation + 1);
    CHECK(setlocale(LC_NUMERIC, snapshot.locale_policy) &&
          ef_parse(&first, (const unsigned char *)"replay", 6, &view) ==
              EF_OK &&
          memcmp(expected, view.data, sizeof expected) == 0 &&
          ef_view_release(&view) == EF_OK);
    return 0;
}

/*
 * Serial trace of two actors that each read the counter before either writes
 * it back. It models the interleaving without executing a C data race.
 */
static unsigned model_interleaved_updates(void)
{
    unsigned shared = 0;
    unsigned left = shared;
    unsigned right = shared;
    shared = left + 1;
    shared = right + 1;
    return shared;
}

int main(void)
{
    char selected_name[SELECTED_LOCALE_BYTES];
    int saved_errno = 0;
    if (probe_scratch() || probe_signal())
    {
        return 1;
    }
    errno = E2BIG;
    saved_errno = errno;
    if (probe_stdio_position() ||
        probe_locale(selected_name, sizeof selected_name) || probe_rand() ||
        probe_replay())
    {
        return 1;
    }
    unsigned shared = model_interleaved_updates();
    CHECK(shared == 1);
    printf(
        "{\"static_scratch_overwrite\":true,\"signal_flag\":%d,\"saved_errno\":%d,\"locale\":\"%s\",\"equal_numeric_output\":true,\"lost_update_model\":%u}\n",
        (int)pending, saved_errno, selected_name, shared);
    return 0;
}
