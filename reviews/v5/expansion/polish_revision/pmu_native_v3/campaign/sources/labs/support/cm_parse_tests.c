/*
 * Tests for the checked decimal parser (cm_parse.h).
 *
 * Every boundary is derived from the actual widths of uintmax_t, unsigned
 * and size_t on the build target, so the same tests are meaningful on LP64,
 * ILP32 and Cortex-M targets. A case that cannot exist on a target (for
 * example "UINT_MAX + 1" when unsigned is as wide as uintmax_t) is reported
 * as skipped rather than silently passing.
 */
#include "cm_parse.h"

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned failure_count;
static unsigned skipped_count;

#define CM_EXPECT(condition)                                           \
    do                                                                 \
    {                                                                  \
        if (!(condition))                                              \
        {                                                              \
            (void)fprintf(stderr, "CM_PARSE_ASSERT:%d:%s\n", __LINE__, \
                          #condition);                                 \
            failure_count++;                                           \
        }                                                              \
    } while (0)

/* Decimal text of value + 1, computed digit by digit so it is exact even for
 * UINTMAX_MAX (where value + 1 is not representable). */
static void decimal_successor(uintmax_t value, char *text, size_t size)
{
    char digits[64];
    int length = snprintf(digits, sizeof digits, "%" PRIuMAX, value);
    int carry = 1;

    for (int i = length - 1; (i >= 0) && (carry != 0); --i)
    {
        if (digits[i] == '9')
        {
            digits[i] = '0';
        }
        else
        {
            digits[i]++;
            carry = 0;
        }
    }
    (void)snprintf(text, size, "%s%s", (carry != 0) ? "1" : "", digits);
}

static void test_grammar(void)
{
    static const char *const rejected[] = {
        " 1", "1 ",  "+1",    "-1",  "-0",  "0x10",
        "1a", "1.0", "1,000", "\t7", "1\n",
    };
    uintmax_t value = 42;

    CM_EXPECT(cm_parse_umax(NULL, 0, UINTMAX_MAX, &value) == CM_PARSE_EMPTY);
    CM_EXPECT(cm_parse_umax("", 0, UINTMAX_MAX, &value) == CM_PARSE_EMPTY);
    for (size_t i = 0; i < sizeof rejected / sizeof rejected[0]; ++i)
    {
        value = 42;
        CM_EXPECT(cm_parse_umax(rejected[i], 0, UINTMAX_MAX, &value) ==
                  CM_PARSE_SYNTAX);
        CM_EXPECT(value == 42); /* output untouched on failure */
    }
    CM_EXPECT(cm_parse_umax("0", 0, UINTMAX_MAX, &value) == CM_PARSE_OK);
    CM_EXPECT(value == 0);
    CM_EXPECT(cm_parse_umax("007", 0, UINTMAX_MAX, &value) == CM_PARSE_OK);
    CM_EXPECT(value == 7);
}

static void test_wide_limits(void)
{
    char text[80];
    uintmax_t value = 0;

    (void)snprintf(text, sizeof text, "%" PRIuMAX, UINTMAX_MAX);
    CM_EXPECT(cm_parse_umax(text, 0, UINTMAX_MAX, &value) == CM_PARSE_OK);
    CM_EXPECT(value == UINTMAX_MAX);

    /* One past the widest type: conversion overflow, not wrap to zero. */
    decimal_successor(UINTMAX_MAX, text, sizeof text);
    value = 5;
    CM_EXPECT(cm_parse_umax(text, 0, UINTMAX_MAX, &value) == CM_PARSE_RANGE);
    CM_EXPECT(value == 5);

    CM_EXPECT(cm_parse_umax("99999999999999999999999999999999999999999999", 0,
                            UINTMAX_MAX, &value) == CM_PARSE_RANGE);
}

static void test_requested_range(void)
{
    uintmax_t value = 0;

    CM_EXPECT(cm_parse_umax("1", 1, 3, &value) == CM_PARSE_OK);
    CM_EXPECT(cm_parse_umax("3", 1, 3, &value) == CM_PARSE_OK);
    CM_EXPECT(cm_parse_umax("0", 1, 3, &value) == CM_PARSE_RANGE);
    CM_EXPECT(cm_parse_umax("4", 1, 3, &value) == CM_PARSE_RANGE);
    CM_EXPECT(cm_parse_umax("2", 3, 1, &value) == CM_PARSE_RANGE); /* empty */
}

static void test_unsigned_narrowing(void)
{
    char text[80];
    unsigned value = 9;

    (void)snprintf(text, sizeof text, "%u", UINT_MAX);
    CM_EXPECT(cm_parse_unsigned(text, 0, UINT_MAX, &value) == CM_PARSE_OK);
    CM_EXPECT(value == UINT_MAX);

    if ((uintmax_t)UINT_MAX < UINTMAX_MAX)
    {
        /* The original defect: (unsigned)strtoul("4294967297") == 1 on LP64.
         * A value that would wrap to an accepted one must be rejected. */
        (void)snprintf(text, sizeof text, "%" PRIuMAX,
                       (uintmax_t)UINT_MAX + 2u);
        value = 9;
        CM_EXPECT(cm_parse_unsigned(text, 0, 3, &value) == CM_PARSE_RANGE);
        CM_EXPECT(value == 9);
        decimal_successor(UINT_MAX, text, sizeof text);
        CM_EXPECT(cm_parse_unsigned(text, 0, UINT_MAX, &value) ==
                  CM_PARSE_RANGE);
    }
    else
    {
        skipped_count++; /* unsigned is as wide as uintmax_t on this target */
    }
    CM_EXPECT(cm_parse_unsigned("2", 0, 1, &value) == CM_PARSE_RANGE);
    CM_EXPECT(cm_parse_unsigned("", 0, 1, &value) == CM_PARSE_EMPTY);
}

static void test_size_narrowing(void)
{
    char text[80];
    size_t value = 0;

    (void)snprintf(text, sizeof text, "%zu", SIZE_MAX);
    CM_EXPECT(cm_parse_size(text, 0, SIZE_MAX, &value) == CM_PARSE_OK);
    CM_EXPECT(value == SIZE_MAX);
    if ((uintmax_t)SIZE_MAX < UINTMAX_MAX)
    {
        decimal_successor(SIZE_MAX, text, sizeof text);
        CM_EXPECT(cm_parse_size(text, 0, SIZE_MAX, &value) == CM_PARSE_RANGE);
    }
    else
    {
        skipped_count++; /* size_t is as wide as uintmax_t on this target */
    }
    CM_EXPECT(cm_parse_size("4194304", 1, 4194304, &value) == CM_PARSE_OK);
    CM_EXPECT(cm_parse_size("4194305", 1, 4194304, &value) == CM_PARSE_RANGE);
}

static void test_names(void)
{
    CM_EXPECT(strcmp(cm_parse_status_name(CM_PARSE_OK), "ok") == 0);
    CM_EXPECT(strcmp(cm_parse_status_name(CM_PARSE_RANGE), "range") == 0);
    CM_EXPECT(strcmp(cm_parse_status_name((enum cm_parse_status)99),
                     "invalid-status") == 0);
}

int main(void)
{
    test_grammar();
    test_wide_limits();
    test_requested_range();
    test_unsigned_narrowing();
    test_size_narrowing();
    test_names();
    if (failure_count != 0u)
    {
        (void)fprintf(stderr, "CM_PARSE_FAIL:%u\n", failure_count);
        return EXIT_FAILURE;
    }
    (void)printf("CM_PARSE_PASS skipped=%u\n", skipped_count);
    return EXIT_SUCCESS;
}
