#include "bounds.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "BOUNDS-CHECK:%s line%d\n", #x, __LINE__); \
            return 1;                                                  \
        }                                                              \
    } while (0)

int main(void)
{
    unsigned char storage[8] = {1, 2, 3, 4};
    CHECK(ac_admit((struct ac_view){storage, 3, 2}) == AC_INVALID);
    CHECK(ac_admit((struct ac_view){NULL, 1, 1}) == AC_INVALID);
    CHECK(ac_admit((struct ac_view){NULL, 0, 0}) == AC_OK);
    CHECK(ac_admit((struct ac_view){NULL, 0, 1}) == AC_INVALID);
    struct ac_wait w = {AC_POLL, 99};
    CHECK(ac_timeout(INT_MIN, &w) == AC_OK && w.tag == AC_INFINITE &&
          !w.milliseconds);
    CHECK(ac_timeout(0, &w) == AC_OK && w.tag == AC_POLL);
    CHECK(ac_timeout(60000, &w) == AC_OK && w.milliseconds == 60000);
    CHECK(ac_timeout(60001, &w) == AC_INVALID && w.milliseconds == 60000);
    unsigned histories = 0;
    for (unsigned start = 0; start < 256; ++start)
        for (unsigned elapsed = 0; elapsed < 256; ++elapsed)
        {
            struct ac_deadline d;
            int expired = 77;
            CHECK(ac_deadline(start, (struct ac_wait){AC_FINITE, 17}, &d) ==
                  AC_OK);
            enum ac_status r = ac_expired(d, (start + elapsed) % 256, &expired);
            CHECK((elapsed > 127 && r == AC_INVALID && expired == 77) ||
                  (elapsed <= 127 && r == AC_OK && expired == (elapsed >= 17)));
            ++histories;
        }
    struct ac_deadline d;
    CHECK(ac_deadline(255, (struct ac_wait){AC_FINITE, 128}, &d) == AC_INVALID);
    uint_least16_t words[2] = {0x1234, 0xabcd};
    unsigned char octets[4] = {9, 9, 9, 9};
    CHECK(ac_octets16(words, 2, octets, 3) == AC_FULL && octets[0] == 9);
    CHECK(ac_octets16(words, 2, octets, 4) == AC_OK);
    CHECK(octets[0] == 0x34 && octets[1] == 0x12 && octets[2] == 0xcd &&
          octets[3] == 0xab);
    CHECK(ac_octets16(NULL, SIZE_MAX, NULL, 0) == AC_INVALID);
    struct api_buffer b = {{1, 2}, 2, 4}, s = {{3, 4}, 2, 32};
    CHECK(ac_append_slice(&b, (struct ac_slice){&s, 0, 2}) == AC_OK &&
          b.length == 4 && b.data[3] == 4);
    CHECK(ac_append_slice(&b, (struct ac_slice){&s, 0, 1}) == AC_FULL &&
          b.length == 4);
    b.length = 0;
    b.capacity = 32;
    CHECK(ac_append_slice(&b, (struct ac_slice){&b, 0, 1}) == AC_OVERLAP &&
          !b.length);
    CHECK(ac_append_slice(&b, (struct ac_slice){&s, SIZE_MAX, 1}) ==
          AC_INVALID);
    CHECK(ac_append_slice(&b, (struct ac_slice){&b, 32, 0}) == AC_OK);
    CHECK(api_append(&b, NULL, SIZE_MAX) == API_INVALID);
    CHECK(api_append(&b, storage, SIZE_MAX) == API_FULL && !b.length);
    CHECK(api_append(&b, b.data, 1) == API_OVERLAP && !b.length);
    unsigned append_cases = 0;
    for (size_t cap = 0; cap <= 32; ++cap)
        for (size_t len = 0; len <= cap; ++len)
            for (size_t n = 0; n <= 32; ++n)
            {
                struct api_buffer ordinary = {{0}, len, cap},
                                  changed = {{0}, len, cap},
                                  source = {{0}, 32, 32};
                for (size_t i = 0; i < 32; ++i)
                {
                    ordinary.data[i] = changed.data[i] = (unsigned char)i;
                    source.data[i] = (unsigned char)(99 - i);
                }
                enum api_status a = api_append(&ordinary, source.data, n);
                enum ac_status c =
                    ac_append_slice(&changed, (struct ac_slice){&source, 0, n});
                int full = n > cap - len;
                CHECK(a == (full ? API_FULL : API_OK) &&
                      c == (full ? AC_FULL : AC_OK));
                CHECK(ordinary.length == (full ? len : len + n) &&
                      changed.length == ordinary.length);
                for (size_t i = 0; i < 32; ++i)
                {
                    unsigned char expected =
                        (!full && i >= len && i - len < n)
                            ? (unsigned char)(99 - (i - len))
                            : (unsigned char)i;
                    CHECK(ordinary.data[i] == expected &&
                          changed.data[i] == expected);
                }
                ++append_cases;
            }
    struct f_buffer f = {{1, 2, 8, 9}, 2, 1};
    CHECK(ac_append_allow(&f, 2, 2) == F_OK && f.length == 4 &&
          f.data[2] == 8 && f.data[3] == 9);
    struct ac_value value = {99, storage};
    CHECK(ac_lookup(0, &value) == AC_OK && value.value == 0 &&
          value.nullable == NULL);
    CHECK(ac_lookup(1, &value) == AC_OK && value.value == 7 && value.nullable);
    CHECK(ac_lookup(2, &value) == AC_MISSING && value.value == 7);
    CHECK(ac_lookup(3, &value) == AC_BACKEND && value.value == 7);
    for (unsigned k = 0; k < 4; ++k)
    {
        struct ac_value v = {99, NULL};
        enum ac_status st = ac_lookup(k, &v);
        struct ac_result r = ac_lookup_tagged(k);
        CHECK(st == r.status &&
              (st != AC_OK ||
               (v.value == r.value.value && v.nullable == r.value.nullable)));
    }
    unsigned char data[4] = {65, 66, 255, 67}, out[4] = {9, 9, 9, 9};
    size_t written = 77;
    CHECK(ac_copy_ascii(data, 4, out, 4, &written, 0) == AC_INVALID &&
          written == 77 && out[0] == 9);
    CHECK(ac_copy_ascii(data, 4, out, 2, &written, 1) == AC_FULL &&
          written == 0 && out[0] == 9);
    CHECK(ac_copy_ascii(NULL, 0, out, 4, &written, 0) == AC_OK && !written);
    struct ac_progress p = ac_ascii_prefix(data, 4, out, 4);
    CHECK(p.status == AC_INVALID && p.consumed == 2 && p.produced == 2 &&
          out[0] == 65 && out[2] == 9);
    p = ac_ascii_prefix(data, 2, out, 1);
    CHECK(p.status == AC_FULL && p.consumed == 1 && p.produced == 1);
    unsigned v = 1, evaluations = 0;
    CHECK(ac_checked_add(2, 4, &v, &evaluations) == AC_OK && v == 3 &&
          evaluations == 1);
    CHECK(ac_checked_add(2, 4, &v, &evaluations) == AC_FULL && v == 3 &&
          evaluations == 2);
    v = 5;
    CHECK(ac_checked_add(0, 4, &v, &evaluations) == AC_INTERNAL && v == 5 &&
          evaluations == 3);
    unsigned instrumentation = 0;
    AC_INSTRUMENT(++instrumentation);
    CHECK(instrumentation == (AC_TRACE_ENABLED ? 1u : 0u));
    unsigned char external[3] = {5, 4, 0};
    v = 77;
    CHECK(ac_admit_wire(external, 3, &v) == AC_INVALID && v == 77);
    external[0] = 3;
    external[2] = 2;
    CHECK(ac_admit_wire(external, 3, &v) == AC_FULL && v == 77);
    external[2] = 1;
    CHECK(ac_admit_wire(external, 3, &v) == AC_OK && v == 4);
    printf(
        "BOUNDS-PASS:histories=%u;append_cases=%u;host_CHAR_BIT=%d;model_unit_bits=16;size_t=%zu;pointer=%zu;value=%zu;tagged=%zu\n",
        histories, append_cases, CHAR_BIT, sizeof(size_t), sizeof(void *),
        sizeof(struct ac_value), sizeof(struct ac_result));
    return 0;
}
