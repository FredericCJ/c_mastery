# T12.08: admission grammar before numeric conversion

Required PR-T12-03 original T12.08a/b/c and their delayed variants, families
A/B/C/D/F/I/J. Use C17 with the actual prior f_parse_i32 learner artifact and the
declared hosted conversion library. No library replaces the ASCII grammar being
taught. Prerequisites: counted input, checked integer arithmetic, errno/end-pointer
interpretation, object lifetime, locale ownership and explicit result publication.
Read R-LLIB-N1570-NUMERIC and the separately sourced POSIX locale contracts.
The inspected ISO text is a C11 draft; published edition access remains separate.

Complete conversion.c. Inputs are stable counted spans, including possible
embedded NULs; outputs are disjoint live caller-owned objects. Token length is at
most128 bytes. All failure statuses preserve output. A null input is allowed only
with zero extent, which is empty syntax. Reject excess length distinctly. Every
grammar check precedes conversion; a valid prefix is insufficient admission.
Time is O(n), bounded token staging129 bytes, no allocation or retained borrows.

`lc_i8` admits `-?[0-9]+`, or `[+-]?[0-9]+` in the changed policy. Validate all
octets first, then adapt the actual prior signed32 parser and require -128..127.
The plus adapter must not turn `+-12` into a valid negative number. Reject any
leading/trailing whitespace regardless of locale. For0,-12,12x,empty,999 derive
OK0,OK-12,SYNTAX,SYNTAX,RANGE. Compare actual atoi and strtol separately: atoi
cannot distinguish a legitimate zero from no conversion and accepts a numeric
prefix, while strtol exposes the end pointer. Never run an out-of-range atoi as
a portable experiment; the specified result-representation failure is undefined.

`lc_unsigned` uses an explicitly stated ordinary grammar `\+?[0-9]+`, base10,
and a caller-supplied uintmax_t limit. Thus +1 is admitted,010 means10, and -1,
0x10,12x and whitespace-only input are syntax failures. Reset errno immediately
before strtoumax, require the end pointer to equal the full staged token end,
check ERANGE, then compare against the target limit before any narrowing. Test
UINTMAX_MAX and its decimal spelling followed by0, and a target limit255 with256.
A successful return value equal to UINTMAX_MAX is not alone evidence of failure.

The changed base-zero policy admits C17 decimal, leading-zero octal and0x/0X
hexadecimal forms with optional plus. It still rejects minus and whitespace.
010 now means8,0x10 means16,08 is invalid, and0b10 is rejected rather than silently
inheriting a newer library extension. Specify this syntax before asking base0 to
convert. A library compiled in a particular language mode is not proof that all
its runtime conversion extensions disappear.

For the float transfer use this precise canonical decimal-rational notation:
zero is exactly `0`; a nonzero value is an optional minus, one digit1..9, optional
fraction whose final digit is nonzero, then lowercase e and a normalized exponent
(`0` or optional minus followed by a nonzero digit and further digits). Examples:
`1.5e0`, `-1e2`, `1e-3`; reject `+1e0`, `01e0`, `1.0e0`, `1e+0`, `1e-0`, `-0`,
NaN, infinity and hexadecimal syntax. This normalizes a decimal rational's
spelling, not the binary floating result: distinct rationals may round to the
same double. It is not a correctly rounded decimal-converter implementation or
a shortest-roundtrip formatter. Those mechanisms require their separate work.

`lc_wire_float` validates before invoking a synchronous converter callback. The
callback receives a borrowed NUL-terminated staging token, returns the value,
consumed byte count and explicit range status, and cannot retain/reenter. Require
full consumption, no range error and a finite result before publishing. A missing
consumption count is a backend-contract failure. The hosted adapter owns a POSIX
C-locale object, temporarily installs it in the current thread, captures errno
before restoring the prior locale, and never changes another thread's locale.
Record the floating rounding mode, radix and mantissa width. The core signature
cannot enforce locale or callback discipline; state these external assumptions.

Compare actual strtod on1.5,leading space,infinity and0x1.8p+0 under C and an
installed second locale. Preserve exact consumed lengths and value classifications.
Show that the integer grammar still rejects1.5. Do not invent decimal-comma
behavior when only C.utf8 is installed, or silently call the float grammar a
locale-independent conversion algorithm. Its admission is ASCII; its backend
has a separately verified locale contract. Overflow and underflow remain distinct
from syntactic admission and preserve public output.

Deliver grammar, status table, unchanged-output evidence, locale/rounding manifest,
implementation and independent cases. Run conversion_tests.c under both compilers
O0/O2 and hosted sanitizers. Compare against independent regex/unbounded integers,
not an oracle that duplicates the accumulator loop. Diagnose stale errno, missing
target-limit checks and bypassed float admission using separately compiled mutants.

Rubric:0 accepts forbidden syntax or publishes failed results;1 reproduces one
conversion;2 separates grammar/range/library/locale obligations and explains the
misconceptions;3 solves the original plus, base-zero or canonical-wire transfer
independently after at least7days. Hints: distinguish no conversion from value0;
validate the entire token; check the target bound while the value is still wide.
Inputs are synthetic and local. No independent learner, reviewer, complete
numeric-library implementation or physical evidence follows from author tests.
