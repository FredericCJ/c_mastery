# T12.12 and16a/c: ordering, library mechanisms and random-source contracts

Required PR-T12-05 ordinary and original delayed criteria; families
A/B/C/D/E/F/G/H/I/J. C17 hosted with binary64 for the changed floating fixture;
GNU interposition and Linux randomness are explicitly separate platform cases.
Prerequisites: integer bounds, NaN comparisons, synchronous callbacks, actual
learner f_sort/FRecord/FWork component and byte-span ownership. Complete
sorting.c against sorting.h. References: R-LLIB-N1570-SORT-RAND,
R-LLIB-GLIBC241-SORT,R-LLIB-MUSL125-SORT and R-LLIB-LINUX-RANDOM. No imported
library may replace the original f_sort in lr_sort; qsort is a separate
comparison provider whose mechanism is the subject of source investigation.

Derive an int32 comparator from relational truth values. Subtracting the keys
can overflow even when both inputs are valid; casting or suppressing a warning
does not repair that contract. Check reflexive equality, antisymmetry and
transitivity on INT32_MIN,-1,0,1,INT32_MAX. Sort minimum,maximum,0,0,7 and search
for0 and8. Both zeros exist; bsearch may choose either. Zero-element calls use
valid pointer arguments. A duplicate's identity is not guaranteed by qsort.
Comparators borrow live objects synchronously, cannot mutate the array and must
remain consistent after movement; an element's current address is not its key.

lr_sort accepts at most32 int32 values and a live disjoint FWork output. Stage
FRecord values with original ordinals, call the actual f_sort once, and publish
only after success. Reject invalid arguments unchanged. There is no allocation;
storage is32 records, work is O(n²), and comparisons are at most n(n-1)/2 for
this actual insertion-sort provider. This bounded wrapper is not a generic
unbounded replacement for qsort. Compare both against an independent histogram
oracle on every array of length0..5 over the five-key alphabet:3906 cases.
The histogram constructs sorted multiplicities without using either sorting
algorithm. Require ordering and every input occurrence, not merely extrema.

For changed records define numeric values ascending, all NaNs after numbers,
then unsigned identity ascending for equal numerical values or two NaNs.
Identities are unique in the supplied eight-record fixture. Signed zeros compare
equal before identity; NaN payload bits are not inspected. Derive the policy's
lexicographic ordering and test512 triples before calling qsort. Ordinary
less-than alone makes NaN appear equal to mutually unequal numbers, so it cannot
justify the required ordering. Never feed a detected inconsistent comparator
to the library and assign portable meaning to the resulting behavior.

Inspect the complete pinned source files, preserving identity and license.
glibc2.41 qsort uses a1024-byte stack buffer; for width<=32 its temporary size
is n*width, while larger records use2*n*sizeof(void*)+width. It sorts indirect
pointers then cycles records for the larger case. If allocation fails it uses
in-place heapsort and preserves the saved errno in this implementation. Record
which comparisons and copy paths occur. Contrast musl1.2.5's smoothsort,
Leonardo-size heap representation, sift/trinkle operations and bounded256-byte
cycle chunks. Read the source before asserting absence of allocation; one
malloc hook cannot observe every possible hidden allocation mechanism.

Execute widths32,33,64 at counts20 and96 in the exact glibc runtime, both with
normal allocation and with every traced allocation denied. The original local
interposer activates only around qsort; it has no effect on other processes.
Verify every payload identity byte as well as key order. Execute corresponding
musl widths with an explicitly static runtime and inspect link closure. The
public contract still holds on fallback, but equal-key order may change.
Record allocation counts/bytes as measurements; source-derived movement costs
are analytical predictions, not fabricated memory-traffic measurements.

Build a ledger with three columns: public requirement, implementation choice,
application-visible extension. Ascending order/permutation belongs in the first;
thresholds, stability on one path and stack layout in the second; GNU alias
attributes, qsort_r signature conventions, interposition and void-pointer
arithmetic in the third. The GNU source's may_alias typedefs and internal
pointer operations are not permission to copy those constructs into strict
ISO learner code. Switching to f_sort or musl should preserve tests for the
shared contract and invalidate tests that accidentally demanded glibc details.

The randomness exercise has a different contract. Record32 rand outputs after
seed120617, reseed and verify exact same-runtime repetition. Keep the actual
runtime/version and RAND_MAX. There is no portable golden sequence or implied
cryptographic quality. For a synthetic source uniform over0..7, enumerating
all eight values gives modulo3 counts3,3,2. Accept only0..5 before reduction:
each outcome then has two antecedents. lr_three invokes a synchronous source
at most budget times, rejects out-of-domain/negative source results, returns
RETRY unchanged on exhaustion, and publishes only an accepted value. Under
independent uniform draws the expected number of attempts is4/3; the budget is
the only deterministic bound, and a pathological source can always exhaust it.
Rejection removes this arithmetic bias; it cannot create missing entropy.

Security scope: the original three-bit recurrence state=(5*state+1)%8 is the
intentionally weak target. The attacker observes each output, which reveals
the entire state, and predicts the next output in the same local process.
Eight exact predictions are the harmless success condition. No credentials,
external service or third-party generator is attacked. This proves failure of
this toy unpredictability property, not a break of every rand implementation.
The repair is a separately sourced OS randomness boundary, never homemade
production cryptography layered on rand. The Linux adapter uses getrandom
with GRND_NONBLOCK; unavailable entropy or unsupported service is failure,
with no fallback. It checks returned counts and distinguishes interruption.
See the [Linux6.19 getrandom contract](https://man7.org/linux/man-pages/man2/getrandom.2.html).

lr_entropy stages exactly32 bytes. Positive prefixes advance the offset;
-2 consumes retry budget; zero/other negative or oversized counts fail. All
failure/budget exits leave caller output unchanged, but consumed source bytes
are not rolled back. A new invocation starts a new request. The source must
write no more than its offered capacity and not retain the borrow. Test all33
failure cuts and a retry after a partial prefix. Actual OS output is discarded
after checking acquisition; statistical appearance is not an unpredictability
proof. No real secrets are used and no secure-erasure claim is made for this
teaching buffer. Compromise of the kernel/source, fork policy and application
secret handling are outside this small acquisition contract.

Deliver derivations, code, source call/movement maps, type/layer ledger,
allocation traces, independent checks and attacker/repair analysis. Six
GCC/Clang profiles pass; NaN-policy, biased-admission and partial-publication
mutants must fail their intended assertions after compiling. Rubric:0 broken
order/ownership or universal claims;1 nominal sorting/reseed only;2 derives
mechanisms and verifies boundaries with declared evidence;3 independently
solves the original NaN, allocation-denial, security-source, width-change and
provider-change variants after at least7days. Hints: compare without subtracting;
separate key equivalence from identity; count preimages before measuring a
histogram; a source observation is not an API guarantee. Author execution does
not replace external review, learner transfer or board measurements.
