/*
 * Replay driver for the admission-capacity exercise.
 *
 * Usage: replay HEX CHUNK_BYTES CAPACITY PAUSE_TURNS
 *
 * Feeds the wire bytes in HEX to a struct cp_parser in chunks of at most
 * CHUNK_BYTES, admitting each complete frame to a struct cp_queue through
 * cpa_admit() with the given CAPACITY. The queue is pre-filled to CAPACITY and
 * the consumer starts after PAUSE_TURNS turns, so a full queue is observed.
 *
 * Exit status: 0 and one "CPA_OBS ..." line on stderr when the stream is fully
 * drained; 41 and "CPA_ASSERT:<expression>" on a violated invariant (including
 * running out of the turn bound); 2 for malformed arguments.
 */
#include "admission.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                    \
    do                                              \
    {                                               \
        if (!(x))                                   \
        {                                           \
            fprintf(stderr, "CPA_ASSERT:%s\n", #x); \
            return 41;                              \
        }                                           \
    } while (0)

enum replay_limit
{
    REPLAY_EXIT_USAGE = 2,
    REPLAY_EXIT_ASSERT = 41,
    REPLAY_MAX_HEX_CHARS = 512,
    REPLAY_MAX_INPUT_BYTES = REPLAY_MAX_HEX_CHARS / 2,
    REPLAY_MAX_CHUNK_BYTES = 256,
    REPLAY_MAX_PAUSE_TURNS = 256,
    REPLAY_SEQUENCE_BASE = 900,
    REPLAY_TURN_FACTOR = 8
};

struct replay_args
{
    const char *hex;
    size_t chunk_bytes;
    size_t capacity;
    size_t pause_turns;
};

/*
 * Parses all of text as a decimal number in [min, max]. Returns 0 and stores
 * the value on success; returns -1 and leaves *value unchanged for empty text,
 * trailing characters, overflow or an out-of-range value (CERT ERR34-C).
 */
static int parse_decimal(const char *text, size_t min, size_t max,
                         size_t *value)
{
    char *end = NULL;
    errno = 0;
    const unsigned long parsed = strtoul(text, &end, 10);
    if (text[0] < '0' || text[0] > '9' || errno != 0 || *end != '\0')
    {
        return -1;
    }
    if (parsed < min || parsed > max)
    {
        return -1;
    }
    *value = (size_t)parsed;
    return 0;
}

/*
 * Validates argv into args. Returns 0, or -1 for a wrong count, odd or long
 * hex text, or a bad number. It uses the local checked parser above rather
 * than labs/support/cm_parse.h because the learner export build has no
 * -Ilabs/support and does not link cm_parse.c.
 */
static int parse_arguments(int argc, char **argv, struct replay_args *args)
{
    if (argc != 5)
    {
        return -1;
    }
    args->hex = argv[1];
    const size_t hex_chars = strlen(argv[1]);
    if (hex_chars % 2 != 0 || hex_chars > REPLAY_MAX_HEX_CHARS)
    {
        return -1;
    }
    if (parse_decimal(argv[2], 1, REPLAY_MAX_CHUNK_BYTES, &args->chunk_bytes) !=
            0 ||
        parse_decimal(argv[3], 1, CP_QUEUE, &args->capacity) != 0 ||
        parse_decimal(argv[4], 0, REPLAY_MAX_PAUSE_TURNS, &args->pause_turns) !=
            0)
    {
        return -1;
    }
    return 0;
}

/* Decodes count bytes (2 * count hex digits) of text; -1 on a bad digit. */
static int decode_hex(const char *text, unsigned char *input, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        unsigned byte;
        if (sscanf(text + 2 * i, "%2x", &byte) != 1)
        {
            return -1;
        }
        input[i] = (unsigned char)byte;
    }
    return 0;
}

static void print_frame(const struct cp_message *message)
{
    printf("%u:", (unsigned)message->sequence);
    for (size_t i = 0; i < message->length; ++i)
    {
        printf("%02x", message->payload[i]);
    }
    putchar('\n');
}

/*
 * A full queue must refuse admission without losing the parser's complete
 * frame, and the frame must stay unchanged while it waits.
 */
static int check_refusal_keeps_frame(struct cp_parser *parser,
                                     const struct cp_message *before,
                                     const unsigned char *rest,
                                     size_t rest_bytes)
{
    struct cp_message after;
    CHECK(cp_peek(parser, &after) == CP_FRAME &&
          after.sequence == before->sequence &&
          after.length == before->length &&
          !memcmp(after.payload, before->payload, before->length));
    /* Feeding while a frame is held must consume nothing. */
    size_t extra = 9;
    CHECK(cp_feed(parser, rest, rest_bytes, &extra) == CP_FRAME && extra == 0);
    return 0;
}

int main(int argc, char **argv)
{
    struct replay_args args;
    if (parse_arguments(argc, argv, &args) != 0)
    {
        return REPLAY_EXIT_USAGE;
    }
    const size_t n = strlen(args.hex) / 2;
    const size_t capacity = args.capacity;
    unsigned char input[REPLAY_MAX_INPUT_BYTES];
    if (decode_hex(args.hex, input, n) != 0)
    {
        return REPLAY_EXIT_USAGE;
    }
    struct cp_parser p;
    struct cp_queue q;
    cp_parser_init(&p);
    cp_queue_init(&q);
    /*
     * Fill the queue to capacity with empty frames numbered from 900. Kept in
     * main so a failure message names these objects exactly as written.
     */
    for (size_t i = 0; i < capacity; ++i)
    {
        p.ready = 1;
        p.message.sequence = (uint16_t)(REPLAY_SEQUENCE_BASE + i);
        p.message.length = 0;
        CHECK(cpa_admit(&p, &q, capacity) == CP_OK);
    }
    /* The bound only needs to be generous; exhausting it is a failure. */
    const size_t turn_bound =
        REPLAY_TURN_FACTOR * (n + CP_WIRE + args.pause_turns + capacity + 1);
    size_t offset = 0;
    size_t refused = 0;
    size_t peak = q.count;
    for (size_t turn = 0; turn < turn_bound; ++turn)
    {
        if (turn >= args.pause_turns)
        {
            struct cp_message out;
            if (cp_take(&q, &out) == CP_OK)
            {
                print_frame(&out);
            }
        }
        const size_t offer =
            (n - offset > args.chunk_bytes) ? args.chunk_bytes : n - offset;
        size_t used = 999;
        const enum cp_status status = cp_feed(&p, input + offset, offer, &used);
        CHECK(used <= offer && p.used <= CP_WIRE);
        offset += used;
        if (status == CP_FRAME)
        {
            struct cp_message before;
            CHECK(cp_peek(&p, &before) == CP_FRAME);
            const enum cp_status admitted = cpa_admit(&p, &q, capacity);
            CHECK(admitted == CP_OK || admitted == CP_FULL);
            if (admitted == CP_FULL)
            {
                ++refused;
                if (check_refusal_keeps_frame(&p, &before, input + offset,
                                              n - offset) != 0)
                {
                    return REPLAY_EXIT_ASSERT;
                }
            }
        }
        else
        {
            CHECK(status == CP_NEED_MORE || status == CP_REJECT);
        }
        CHECK(q.count <= capacity);
        if (q.count > peak)
        {
            peak = q.count;
        }
        if (offset == n && status == CP_NEED_MORE && !p.ready && !q.count)
        {
            fprintf(
                stderr,
                "CPA_OBS capacity=%zu peak=%zu refused=%zu turns=%zu bound=%zu bytes=%zu\n",
                capacity, peak, refused, turn + 1, turn_bound, n);
            return 0;
        }
    }
    CHECK(0);
    return REPLAY_EXIT_ASSERT;
}
