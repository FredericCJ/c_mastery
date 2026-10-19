# T12.14: clocks, borrowed process state and termination

Required AX-PR-T12-04-T12.14.a/b/c and the original DELAYED variants; families
A/B/C/D/F/H/I/J. Use C17 hosted C, then the explicitly separate POSIX2024/Linux
adapters. Prerequisites are checked arithmetic, owned versus borrowed objects,
FILE lifecycle, process/thread lifetime and the learner's PR-T12-04 notebook.
Implement processlab.c against processlab.h. Public tests are examples, not a
complete solution. Reference implementations and transfer analysis stay in
assessor/legacy_library. Read R-LLIB-N1570-PROCESS, R-LLIB-N1570-TIME and
R-LLIB-POSIX2024-PROCESS in the inspected source register. Their C text is the
C11 draft, accurately identified; exact published C17 access remains a gap.

For .14a derive seconds = tick difference / CLOCKS_PER_SEC. Check clock's -1
sentinel before arithmetic. Reject decreasing observations under this adapter's
no-wrap contract; two samples cannot infer how many wraps occurred. Convert
each operand to long double before subtraction so signed clock_t subtraction
cannot overflow. This avoids that integer overflow, not all precision loss:
large nearby integers can round to the same floating value. State the actual
clock_t type, range and long-double precision, and bound the interval accordingly.
The output is written only for LT_OK; null output, unavailable and reversed
samples leave it unchanged. Work/storage are constant and there is no allocation.
CLOCKS_PER_SEC is a conversion factor, not observed clock resolution.

Measure a checksum-producing CPU loop and a controlled 30ms wait with clock and
POSIX CLOCK_MONOTONIC. Record clock_getres, warmup, three repetitions, errors,
CPU seconds and elapsed seconds; retain the checksum so optimization cannot
erase the workload. MONOTONIC's origin is unspecified and its resolution does
not guarantee scheduling latency. The wait can overshoot. Never assert an exact
CPU/elapsed ratio or infer a worst-case latency from the largest observation.
Repeat with four real threads, with the process constrained to one of its
already permitted CPUs, and with three owned competing CPU processes. Restore
the caller's affinity and reap every child. A process CPU clock can aggregate
several threads; descheduling generally advances elapsed time without advancing
that process's CPU time. These are explanatory models, not exact run goldens.

For .14b implement lt_environment(name,out[64]). name is a live terminated
string; out is writable caller storage disjoint from the environment. Copy up
to63 bytes plus NUL. Missing name returns UNAVAILABLE; a longer value returns
RANGE; both preserve out. No borrowed getenv pointer escapes and it is never
modified or freed. Inspection and copying cost O(min(length,64)), with constant
space. The caller must serialize the entire lookup-through-copy interval
against every environment mutation. A mutex around only memcpy is too late.
The application controls all getenv/setenv callers in this fixture; an unknown
plugin bypassing that mutex invalidates the policy. Repeat 1000 updates and1000
snapshots using the same mutex. An immutable startup snapshot is an alternative
when process-wide mutation cannot be controlled, with its own allocation/error
contract. POSIX setenv and pthread operations are not ISO C library entitlements.

Use separate child executables for normal exit, quick_exit and _Exit. Register
two ordinary handlers A then B and two quick handlers X then Y. Record handler
events through a separate, exclusively owned file descriptor, and place
"buffered:" in fully buffered stdout. Keep these output channels distinct:
mixing buffered FILE and raw writes on the same open-file description requires
the active-handle protocol taught in FILEIO.md. Predict reverse registration
order for the selected handler family. Normal exit flushes; for quick/_Exit
record the selected implementation's permitted flushing behavior without a
portable zero-output assertion. Do not invoke system: analyze how an opaque
command string delegates interpretation/execution to the host command processor,
why untrusted text changes the boundary, and why return status requires that
host's contract rather than interpreting it as the command's raw exit value.

For .14c inventory time_t and clock_t without assuming an integer representation,
Unix epoch, width or interchangeability. lt_calendar accepts years1970..2100,
chooses February29 for2000 and January1 otherwise, calls mktime then gmtime and
copies the returned struct tm before another conversion. Pin TZ=UTC0 and locale
for this experiment; mktime interprets local calendar time whereas gmtime uses
UTC. Check both failures and preserve caller output. Calendar conversion is
library-dependent work with a constant-size copied result; do not claim a
constant-time implementation from the wrapper alone. Caller synchronization
must protect shared conversion results and timezone/locale policy if another
thread can disturb them. A reentrant platform alternative needs its own source.

Execute actual i386 binaries with time32 and with _TIME_BITS=64 plus
_FILE_OFFSET_BITS=64 against the named sysroot/loader. Attempt2040: this selected
time32 ABI rejects the conversion, and the widened ABI accepts it. Record errno
and unchanged output on failure. That ABI observation is not an ISO2038 rule.
For interchange use an explicitly specified calendar/epoch/range encoding with
validated conversions; raw time_t bytes disclose implementation layout and do
not define a portable wire format. A timestamp of -1 can also denote a real
instant in some representations, so this bounded wrapper treats the library's
sentinel as unavailable instead of claiming universal timestamp disambiguation.

Deliver implementations, prediction tables, ownership/lifetime arguments,
twelve timing samples per profile, termination events, type/ABI manifest and
failure evidence. Six native debug/release/sanitized profiles, intended scale
and missing-terminator rejections, scaffold rejection and actual32/64 executions
are required. Bounds qualify the observations; they do not establish scheduling
fairness, all calendar ranges or global process thread safety.

Rubric:0 incorrect lifetime/units;1 reproduces a table without causal reasoning;
2 derives and implements every ordinary contract with discriminating failures;
3 independently handles multi-thread/descheduling, concurrent environment and
narrow-representation changes after the original minimum7days. Hints: separate
units from resolution; identify who can invalidate each borrowed pointer; place
the lock before lookup; keep termination handlers and buffered output separate.
Synthetic values, owned children and temporary files only. No shell commands,
credentials, global timezone changes or host signal/security settings are needed.
Author evidence is separate from external review and actual delayed assessment.
