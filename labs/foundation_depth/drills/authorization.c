/* CM-SPECIMEN: deliberately flawed request handler under review; see the
 * "Local authorization composition" section of CONTRACTS.md.
 *
 * Original synthetic teaching target: no files, sockets, credentials or
 * privilege. */
#include "depth.h"
#include <stdio.h>

/* Initial synthetic counter value and the request the drill replays. */
#define DRILL_COUNTER_START 10
#define DRILL_UNAUTHORIZED 0

/* Handler under review: applies one wire record to *counter on behalf of a
 * caller whose authority is `allowed` (trusted, never taken from the wire).
 * Returns the decoder's status, FD_RANGE if the sum leaves int32_t, FD_DENIED
 * when `allowed` is zero, otherwise FD_OK. Compare it with fd_apply. */
static enum fd_status vulnerable(const unsigned char *wire, size_t length,
                                 int allowed, int32_t *counter)
{
    struct fd_record record;
    const enum fd_status status = fd_decode(wire, length, &record);
    if (status != FD_OK)
    {
        return status;
    }
    const int64_t next =
        (int64_t)*counter + (int64_t)record.count * record.value;
    if (next < INT32_MIN || next > INT32_MAX)
    {
        return FD_RANGE;
    }
    *counter = (int32_t)next;
    if (!allowed)
    {
        return FD_DENIED;
    }
    return FD_OK;
}

int main(void)
{
    /* Version 1, flags 0, count 2, value 3. */
    const unsigned char request[FD_WIRE_BYTES] = {1, 0, 0, 2, 0, 0, 0, 3};
    int32_t flawed_counter = DRILL_COUNTER_START;
    int32_t repaired_counter = DRILL_COUNTER_START;
    const enum fd_status flawed_status = vulnerable(
        request, sizeof request, DRILL_UNAUTHORIZED, &flawed_counter);
    const enum fd_status repaired_status = fd_apply(
        request, sizeof request, DRILL_UNAUTHORIZED, &repaired_counter);
    printf(
        "local unauthorized request: flawed status=%d counter=%ld; repaired status=%d counter=%ld\n",
        flawed_status, (long)flawed_counter, repaired_status,
        (long)repaired_counter);
    /* Exit 0 means the recorded observation matches the contract. */
    return !(flawed_status == FD_DENIED && flawed_counter == 16 &&
             repaired_status == FD_DENIED &&
             repaired_counter == DRILL_COUNTER_START);
}
