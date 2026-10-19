# Assessor: precise load recovery and staged snapshots

The admitted CFSR contains only BFARVALID and PRECISERR. If any other fault
flag is present, its provenance cannot be erased by recovering this load.
Stacked PC must equal the original LDR and BFAR the admitted address. The
basic MSP thread-return frame is the only supported layout; accepting an
extended frame without changing offsets would modify the wrong words.

The hardware test's second negative control faults on STR to an invalid output
address after the valid load. Active state alone is therefore insufficient:
PC/address identity must reject it. The default handler never returns from an
unrecognized fault. The teaching override logs and ends only that disposable
emulator. No ISR calls libc, allocates memory or repairs unknown application
state. A live stack and code outside the affected flash bank are prerequisites.

The scanner's local128-byte staging preserves output for all32 failure points.
Two candidates are independent; marking one unreadable cannot invalidate the
other. Actual CRC/generation selection is reused rather than mocked. Worst
case is64 reads plus two bounded CRC validations; fixed stack storage replaces
heap allocation. Exhaustion/physical ECC behavior remain outside this finite
fault model, and fail-stop is distinct from successful recovery.
