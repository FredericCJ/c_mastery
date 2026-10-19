# Exact original variant analysis

Read the learner contract before these solutions. These arguments assume its admitted pointers, lifetimes, initialization, synchronization and environment; they do not repair arbitrary corrupted controls.

## X-T41-001-R1 and V1/V2

For bytes b[i], use coefficient256 at even positions and1 at odd positions, reducing by end-around carry. The numeric checksum is the bitwise complement of the folded sum. Concatenation shifts all right coefficients iff the left length is odd. Multiplication by256 modulo65535 swaps the two sum bytes, including both zero representations; thus combine((s,p),(t,q))=(s+swap^p(t),p XOR q). The swap action distributes over end-around addition and squares to identity, establishing associativity. It does not establish commutativity of concatenation. A one-byte01 followed by02 yields word0102; independently padded segment sums0100 and0200 require swapping the second before addition. Appending these padded sums directly gives0300, a concrete wrong result that ordinary even partitions miss.

The provider's pending byte is not yet committed padding. Repeated finish observes a padded final value without changing the state; a later add can still complete that word. Independent summaries do materialize each local final padded sum but parity restores its position when combining. For the supplied pseudoheader, the TCP length25 excludes the12-byte pseudoheader while including20 header bytes and5 payload bytes. Never insert a hidden boundary pad. The result is a numeric value; serializing its bytes is a distinct operation and cannot use native object layout.

Mutant1 drops the parity swap; mutant2 truncates an end-around carry. The independent Python oracle sums whole big-endian words with arbitrary precision before folding, not calls to the same combine formula. Associativity tests alone could pass a consistently wrong algebra, so the fixed whole-message oracle is necessary. Both zero representations and empty spans have dedicated checks. No checksum result establishes authenticity.

## X-T42-001-R1/R2/R3 and V1/V2

Retain the strict normalized grammar and the actual incremental parser corpus. The local CL6 versus zero-chunk5 disagreement gives two different next-message offsets for the same original input; it demonstrates a framing property violation, not code execution or a complete deployed proxy exploit. The repair rejects the combination before body dispatch and closes the connection; a parser error followed by continued parsing does not implement that repair.

The neutral header may change consumed byte counts while leaving framing unchanged. Space before colon and obsolete folding remain rejected in the selected strict subset. Every split exercises actual retained prefix/CRLF state. An independent expected response/body/consumed-count table is required; two adapters using the same wrong parser are not independent expected results. End-of-stream, interruption, would-block and readiness are separate events. The report binds the concrete 144-case successor corpus, not merely its heading.

## X-T42-002-R1/R2/R3 and V1/V2

For declared maximum M, write M=10q+r,0<=r<=9. A prior value v and next digit d admit iff v<q or v=q and d<=r. This is valid for M=0..9 and UINT32_MAX without computing a negative M-d or overflowing10v+d. Arbitrary leading zeroes remain legal. Maximum controls the wire domain; cap controls storage. A malformed suffix is a400 even if the previous numeric prefix already exceeds cap. Output publication follows all syntax/domain checks, then cap admission.

The original fixed-domain AVR executable establishes actual16-bit size_t with32-bit arithmetic for hd_length. It is separately bound as inherited evidence; native Clang is not a changed data model. The generalized variant's future target compilation must state whether code executes or only links. Neither SIZE_MAX nor int width defines the wire domain. Mutant3 restores the underflowing subtraction and admits9 under maximum0; mutant4 returns413 before reaching the malformed suffix.

## X-T42-003-R1/R2/R3 and V1/V2

A400-year Gregorian era contains400*365+97=146097 days. Starting with year1 modulo400, the first three centuries have36524 days and the last36525. Therefore the quotient for century must cap at3 on the final era day. Within each century, four-year groups contain1461 days; the final day of a leap group requires capping the final year quotient at3. Subtract complete groups before the bounded month loop. Derive719162 from1969 ordinary years and their leap exceptions. Largest admitted epoch day plus this offset is3652058, safely within uint32. Weekday advances by days modulo7 from Thursday. The complete2000–2399 comparison includes the troublesome end of the preceding era in2000 and all century boundaries; Python datetime supplies independent samples across1970–9999.

The old formatter performs up to8029 year subtractions plus month steps. The successor uses a fixed number of unsigned divides/remainders and at most11 month steps. Division cost varies by ISA; a fixed operation count does not guarantee lower wall time for all dates or CPUs. Assembly/object/helper audits and timed equivalent-result runs should be kept distinct from that algebraic bound.

Representation metadata is an approximation to an instant in UTC; monotonic deadlines need a different source. Availability and trust are explicit. Leap-year arithmetic concerns days, not adjustments of UTC seconds. The extended source supplies an attested positive-leap UTC day and second86400; rendering23:59:60 follows RFC9110's wire range. No attestation means refusal, not guesswork. Untrusted data omits Date. TAI/smear conversion requires an additional specified mapping and is rejected here. Mutant5 misses the last-day century cap; mutant6 accepts unauthenticated leap-day authority. Neither a passing leap fixture nor a correctly formatted string establishes a physically synchronized clock.

## X-T42-004-R1/R2/R3 and V1/V2

The original generation check rejects stale queued observations but alone cannot protect an already active descriptor use. Borrow increment and closing admission share one mutex. If borrow wins, close sees its pin and retains fd+storage. If close wins, borrow rejects without publication. Existing borrowers may finish after close was requested; this is intentional draining, not revocation of their live lease. The final release decrements the pending count, performs actual Linux close while holding the mutex and only then allows another admission to see the free slot. The HF provider is accessed solely under this wrapper mutex.

At full capacity a competing new admission can returnFULL before close linearizes, or succeed after final close; neither outcome loses the caller's ownership. A failed flags check occurs while the caller still exclusively owns its candidate. GenerationMAX never wraps; cleanup remains possible even though new admission is permanently refused for retired slots. The test's forced seeds are before publication, not a supported reset API. Repeated close/release cannot close a newly recycled descriptor. Unique lease ownership is a C contract, not a language-enforced linear type.

Linux close errors do not justify retrying a reused integer. The wrapper assumes the documented Linux release behavior for a valid exclusively owned fd, discards late close errors and makes no durability claim. pthread cancellation is disabled during API calls; callers must arrange borrower cleanup and join before destroy. Destroy cannot check whether a future caller will dereference freed storage. Mutant7 closes while a borrower remains; mutant8 admits a new borrower after close linearizes. Both are tested through harmless owned descriptors and deterministic thread gates. Native runs and ThreadSanitizer do not prove all schedules or POSIX implementations.

## Assessment limits

Required learner artifacts include the code, source/version dossier, independent expected rows, exact publication/linearization argument, mutation diagnosis and original changed-condition reconstruction. Current author implementations and tests are assessment support. External review, learner observation after the original delay and physical timing/clock/network evidence remain separate unperformed dimensions. No scores or finite-run result are promoted into absolute mastery.
