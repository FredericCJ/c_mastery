/*
 * Cortex-M4 reference driver for the qualified-views component.
 *
 * Freestanding: no C library. Output and exit use ARM semihosting (BKPT 0xAB),
 * which the AN386 emulator provides. The startup code (labs/qualified_views/
 * startup.S) calls qv_target_main with interrupts masked and a bounded stack;
 * qv_fault is the target of every unused vector.
 */
#include "views.h"

/* ARM semihosting operation numbers and exit reason (r0 / args[0]). */
enum
{
    SEMIHOST_SYS_WRITE0 = 0x04, /* write a NUL-terminated string */
    SEMIHOST_SYS_EXIT_EXTENDED = 0x20, /* exit with a reason and a code */
    SEMIHOST_APPLICATION_EXIT = 0x20026
};

enum
{
    EXIT_CODE_PASS = 0,
    EXIT_CODE_CHECK_FAILED = 1,
    EXIT_CODE_FAULT = 2
};

/* Writes text (NUL-terminated, caller-owned) to the semihosting console. */
static void emit(const char *text)
{
    register unsigned operation __asm__("r0") = SEMIHOST_SYS_WRITE0;
    register const char *argument __asm__("r1") = text;
    __asm__ volatile("bkpt 0xab" : "+r"(operation), "+r"(argument)::"memory");
}

/* Ends the run with exit_code; never returns even if the host ignores it. */
_Noreturn static void stop(unsigned exit_code)
{
    unsigned block[2] = {SEMIHOST_APPLICATION_EXIT, exit_code};
    register unsigned operation __asm__("r0") = SEMIHOST_SYS_EXIT_EXTENDED;
    register unsigned *argument __asm__("r1") = block;
    __asm__ volatile("bkpt 0xab" : "+r"(operation), "+r"(argument)::"memory");
    for (;;)
    {
    }
}

#define TARGET_REQUIRE(condition)         \
    do                                    \
    {                                     \
        if (!(condition))                 \
        {                                 \
            emit("FV_TARGET_FAIL\n");     \
            stop(EXIT_CODE_CHECK_FAILED); \
        }                                 \
    } while (0)

void qv_fault(void)
{
    emit("FV_TARGET_FAULT\n");
    stop(EXIT_CODE_FAULT);
}

/* Record, view, copy and update with caller-owned storage. */
static void check_record_and_copy(struct fv_record *record, int out[4])
{
    struct fv_view view = fv_initialize(record);
    TARGET_REQUIRE(view.data == record->values && view.length == 4 &&
                   view.data[1] == 7 && record->link == NULL);

    TARGET_REQUIRE(fv_copy((struct fv_destination){out, 3}, view) == FV_FULL &&
                   out[0] == 91);
    TARGET_REQUIRE(fv_copy((struct fv_destination){out, 4}, view) == FV_OK &&
                   out[1] == 7 && out[3] == 9);

    /* The live view sees the update; the copy in out does not. */
    TARGET_REQUIRE(fv_update(record, 1, 17) == FV_OK && view.data[1] == 17 &&
                   out[1] == 7);
    TARGET_REQUIRE(fv_update(record, 4, 17) == FV_RANGE);
}

/* Overlapping moves in both directions and the bounds checks. */
static void check_moves(int out[4])
{
    TARGET_REQUIRE(fv_move(out, 4, 1, 0, 3) == FV_OK && out[1] == 0 &&
                   out[2] == 7 && out[3] == 0);
    TARGET_REQUIRE(fv_move(out, 4, 0, 1, 3) == FV_OK && out[1] == 7);
    TARGET_REQUIRE(fv_move(NULL, 0, 0, 0, 0) == FV_OK);
    TARGET_REQUIRE(fv_move(out, 4, 4, 0, 1) == FV_RANGE);
}

/* Reset clears the borrowed link; the opaque owner aliases one record. */
static void check_reset_and_owner(struct fv_record *record)
{
    int linked = 7;
    record->link = &linked;
    TARGET_REQUIRE(fv_reset(record) == FV_OK && record->link == NULL &&
                   record->values[1] == 0);

    struct fv_owner *owner = fv_owner();
    TARGET_REQUIRE(owner != NULL);
    struct fv_view opaque = fv_owner_view(owner);
    int *mutable_alias = fv_owner_mutable(owner);
    TARGET_REQUIRE(mutable_alias != NULL && opaque.data == mutable_alias &&
                   opaque.length == 4);
    mutable_alias[2] = 19;
    TARGET_REQUIRE(opaque.data[2] == 19);
}

void qv_target_main(void)
{
    struct fv_record record;
    int out[4] = {91, 92, 93, 94};

    check_record_and_copy(&record, out);
    check_moves(out);
    check_reset_and_owner(&record);

    volatile unsigned status = 4;
    TARGET_REQUIRE(fv_device_twice(&status) == 0);
    emit("FV_TARGET_PASS caller_owner live_snapshot overlap reset opaque "
         "volatile_model\n");
    stop(EXIT_CODE_PASS);
}
