# T12.16b: a library still needs a platform

Required PR-T12-05 ordinary and original seven-day UART-only variant, families
A/B/C/D/F/G/I/J. Prerequisites: FILE ownership and orientation, synchronous
character callbacks, linking/static archive extraction, platform runtime closure.
Read R-LLIB-PICO1810-OS,R-LLIB-PICO1810-BUILD,R-LLIB-PICO1810-OPTIONS and
R-LLIB-PICO1810-SEEK. C17 source; exact Picolibc1.8.10 tinystdio headers and
native archives. This hosted library investigation is not a strict freestanding
exercise: the native support library and loader are explicit dependencies.
No flashing, UART register accesses or physical latency observations occur.

Apply correction LLIB-T12-PICO-2026-01 to the original task, without deleting
its ID or historical wording. The tagged doc/os.md describes -Dposix-io=false,
but tagged meson_options.txt has no posix-io option. tests-enable-posix-io only
controls tests and is not a substitute. Run the exact rejected configuration
and require the Unknown option diagnostic. A claim that the old A/B build
executed is incorrect. Compare supported configurations A':tinystdio=true,
posix-console=false and B':tinystdio=true,posix-console=true. Both keep whatever
file support the release actually builds; static archive extraction determines
which dependencies a particular linked application needs.

Use the maintained campaign's recorded configure commands. Native testing
selects global-dynamic TLS, automatic errno function, no multilib,picolib,
picocrt or semihosting, and explicitly uses the native standard library.
These choices delegate OS/startup services; they do not remove those services
from a board port's obligations. Keep the full configuration and archive hash,
not just a short human label. Inspect final maps, imports and loader metadata.
GCC/Clang -O0/-O2 execute the console adapter. The linked prebuilt Picolibc is
not sanitizer-instrumented; these observations make no whole-library sanitizer
claim. Its native support is not evidence of an embedded runtime closure.

Complete picolib.c against picolib.h. Supply stdin/stdout/stderr as constant
pointers to a live, mutable FILE configured by FDEV_SETUP_STREAM. The original
teaching endpoint offers input abc then EOF, and owns exactly four output bytes.
Its put callback accepts a character only if capacity remains; return EOF on
full rather than silently dropping. Its get callback returns the next unsigned
character or EOF. Callbacks borrow FILE synchronously and keep no transient
caller pointers. This endpoint is single-owner and serial; no thread/interrupt
reentrancy is promised. lp_reset clears length/input position and stream flags;
lp_output copies exactly the currently accepted prefix to a live four-byte
destination, returning its length. Neither operation allocates. Work per
character is constant; a whole formatting call retains libc's separate costs.

The public test writes xy17, confirms exactly four bytes, then requires Z to
fail on the full endpoint. Read abc and EOF. A UART-like console has no seek
operation: the actual tagged fseek implementation returns failure with ESPIPE
for this configured FILE. Reset and prove reuse. Never invent seek success
by changing a cursor unrelated to the device. Public exit codes11..16 identify
scaffold, output, overflow, input, nonseekable and reset failures; stdout is the
target and cannot also serve as an unconstrained diagnostic channel.

For B', execute a synthetic seekable file roundtrip using the exact archive:
fopen w+, write synthetic, flush, seek, read and close. Inspect final references
to open,close,read,write,lseek and identify how memory allocation is supplied.
Deliberately omit the wrapped read and lseek definitions in separate links;
require the intended missing-symbol error, not any compilation failure. Compare
this closure with A'. Choosing console callbacks does not promise that every
other library call is allocation-free or has no POSIX dependency. On an actual
board, the documented allocation route may additionally require sbrk and heap
symbols; the native runner delegates those responsibilities instead.

The changed UART-only target has no seekable files. Select A' with original
console callbacks and expose only the supported stream operations. Map the
backend boundary to a driver contract: byte acceptance, busy/error policy,
reset behavior, synchronization, buffer lifetime and bounded/blocking behavior
must be supplied by that driver. The executable memory surrogate verifies the
FILE adapter and failure behavior, not physical register or timing claims.
Do not link a general fopen surface and claim it works merely because printf
does. A hypothetical B' board needs explicit POSIX-compatible backends or must
reject unsupported services under a documented contract.

Deliver configuration rejection/correction evidence, adapter, ownership/state
diagram, two actual maps and dependency ledgers, failure checks and UART transfer
analysis. Rubric:0 invents option/driver behavior;1 prints a nominal message;
2 implements failure/ownership and explains exact closure;3 independently derives
the original UART-only change after at least7days. Hints: inspect option names
in the tag; a FILE pointer is not the mutable FILE; trace a final unresolved
symbol back to the archive member that introduced it. The dropped-byte mutant
must compile and fail at exit13; the unsolved scaffold fails at11. Physical,
non-author review and learner evidence remain separately pending.
