#include "taxonomy.h"
#include <stdio.h>
#define CHECK(x)                                                     \
    do                                                               \
    {                                                                \
        if (!(x))                                                    \
        {                                                            \
            fprintf(stderr, "TAXONOMY-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                \
        }                                                            \
    } while (0)

int main(void)
{
    unsigned out = 999;
    const unsigned char invalid[] = "x", valid[] = "7";
    CHECK(ae_boundary(invalid, 1, 0, 0, 0, &out) == AE_INVALID && out == 999);
    CHECK(ae_boundary(valid, 1, 4, 0, 0, &out) == AE_CONTRACT && out == 999);
    CHECK(ae_boundary(valid, 1, 1, 1, 0, &out) == AE_EXHAUSTED && out == 999);
    CHECK(ae_boundary(valid, 1, 1, 0, 1, &out) == AE_DEVICE && out == 999);
    CHECK(ae_boundary(valid, 1, 1, 0, 0, &out) == AE_OK && out == 7);
    struct ae_digit digit = {99, 0};
    CHECK(ae_admit_digit(invalid, 1, &digit) == AE_INVALID &&
          digit.value == 99 && !digit.admitted);
    CHECK(ae_admit_digit(valid, 1, &digit) == AE_OK &&
          ae_trusted_digit(digit, &out) == AE_OK && out == 7);
    CHECK(ae_map(2, 0) == AE_UNKNOWN);
    for (unsigned backend = 0; backend < 2; ++backend)
    {
        int base = backend ? 101 : 10;
        CHECK(ae_map(backend, 0) == AE_OK &&
              ae_map(backend, base) == AE_TIMEOUT &&
              ae_map(backend, base + 1) == AE_EXHAUSTED &&
              ae_map(backend, base + 2) == AE_INVALID &&
              ae_map(backend, 999) == AE_UNKNOWN);
    }
    CHECK(ae_map(1, 104) == AE_TIMEOUT);
    CHECK(ae_outcome(AE_AGAIN) == AE_RETRYABLE &&
          ae_outcome(AE_DEVICE) == AE_TERMINAL &&
          ae_outcome(AE_INVALID) == AE_RECOVERABLE &&
          ae_outcome(AE_AMBIGUOUS) == AE_DEGRADED);
    struct ae_error error = {0};
    CHECK(ae_first(&error, AE_EXHAUSTED, 9, 2, 0, 11) == AE_OK);
    CHECK(ae_first(&error, AE_DEVICE, 9, 1, 0, 77) == AE_CONFLICT);
    for (unsigned i = 0; i < 40; ++i)
        CHECK(ae_secondary(&error, i % 4, 77 + (int)i) == AE_OK);
    CHECK(error.result.code == AE_EXHAUSTED && error.first == 11 &&
          error.stage == 2 && error.stored == 2 &&
          error.secondary[0].cause == 77 && error.secondary[1].cause == 78 &&
          error.dropped == 31 && error.saturated);
    struct ae_counter counter = {0};
    CHECK(ae_submit(&counter, 1, 2, AE_BUSY) == AE_AGAIN &&
          counter.effects == 0);
    CHECK(ae_submit(&counter, 1, 2, AE_FAIL_BEFORE) == AE_DEVICE &&
          counter.effects == 0);
    CHECK(ae_submit(&counter, 1, 2, AE_LOSE_ACK) == AE_AMBIGUOUS &&
          counter.value == 2 && counter.effects == 1);
    CHECK(ae_submit(&counter, 1, 2, AE_SEND) == AE_OK && counter.effects == 1);
    CHECK(ae_submit(&counter, 1, 3, AE_SEND) == AE_CONFLICT &&
          counter.value == 2);
    CHECK(ae_submit(&counter, 2, 1, AE_SEND) == AE_OK &&
          ae_submit(&counter, 3, 1, AE_SEND) == AE_OK &&
          ae_submit(&counter, 1, 2, AE_SEND) == AE_OK && counter.effects == 4 &&
          counter.value == 6);
    struct ae_write_result write = {AE_UNKNOWN, 999};
    CHECK(ae_write_result(5, 2, AE_DEVICE, &write) == AE_OK &&
          write.code == AE_DEVICE && write.accepted == 2);
    CHECK(ae_write_result(0, 0, AE_OK, &write) == AE_OK &&
          write.code == AE_OK && write.accepted == 0);
    CHECK(ae_write_result(5, 2, AE_OK, &write) == AE_INVALID &&
          write.accepted == 0);
    CHECK(ae_lookup(1).found && ae_lookup(1).value == 0 && !ae_lookup(2).found);
    struct av_text fields[4] = {{(const unsigned char *)"x1", 2},
                                {(const unsigned char *)"32768", 5},
                                {(const unsigned char *)"-", 1},
                                {(const unsigned char *)"7", 1}};
    struct ae_fields result;
    CHECK(ae_fields(fields, 4, &result) == AE_INVALID && result.count == 3 &&
          result.values[3] == 7);
    CHECK(result.errors[0].field == 0 && result.errors[0].offset == 0 &&
          result.errors[1].field == 1 && result.errors[1].offset == 4 &&
          result.errors[2].field == 2 && result.errors[2].offset == 1);
    struct ae_public public_result;
    unsigned char bytes[8];
    CHECK(ae_capture(1, 104, 9, 2, &public_result, bytes) == AE_OK &&
          public_result.code == AE_TIMEOUT && public_result.operation == 9);
    unsigned b = 9, op = 9, stage = 9;
    int cause = 9;
    CHECK(ae_decode(bytes, 8, &b, &cause, &op, &stage) == AE_OK && b == 1 &&
          cause == 104 && op == 9 && stage == 2);
    for (size_t n = 0; n < 8; ++n)
        CHECK(ae_decode(bytes, n, &b, &cause, &op, &stage) == AE_INVALID);
    bytes[0] = 2;
    CHECK(ae_decode(bytes, 8, &b, &cause, &op, &stage) == AE_INVALID);
    puts("TAXONOMY-PASS:categories=4;namespaces=2;secondary_attempts=40;expiry_"
         "duplicate=1;fields=4;");
    return 0;
}
