# T12.09: format contracts and publication policy

This required increment implements the original three T12.09 criteria and their
seven-day variants in PR-T12-01. Other T12 criteria retain their separate work.
Use C17 on the declared hosted profile. Here snprintf is the mechanism under
investigation, so using the library is permitted. This is not an implementation
of printf and is not a freestanding runtime. Reuse prior checked-size, ownership
and explicit result reasoning. Inspect N1570 (C11 draft), 7.1.4p1,
7.21.6.1p5–8 and 7.21.6.5p1–3. Published C17/C23 wording remains a source gap.

Complete format.c using format.h. The caller owns live output/result objects
and a stable callback context. They must be disjoint from the formatter's input;
the callback borrows output synchronously, cannot retain it or reenter, and must
respect its extent. ISO C does not enforce this ownership or callback contract.

For prefix publication, invoke the callback once. Keep its return signed until
checking for encoding failure. A nonnegative value is the required byte count
excluding NUL. With capacity c>0, publish min(required,c-1) bytes and a terminator;
completion requires required<c. Capacity0 permits a null destination and writes
nothing, including no terminator. A negative result leaves result metadata
unchanged; output effects follow the underlying callback's failure contract and
are not assumed transactional. Do not cast a negative result into a capacity.

For transactional publication, first query with null/zero, then render into at
most64 bytes of original local staging. Required counts0..63 are supported only
when the caller has room for the terminator. Query/capacity/render failure,
changed required length or missing terminator leaves both public objects
unchanged. Compare the second result before copying. This is an observable
policy change: two calls,64 staging bytes, and a stable-input requirement. A
query can itself fail; it is not an infallible allocation oracle. The callback
must still obey its memory bounds; a wrapper cannot repair arbitrary corruption.

Implement a configuration parser accepting exactly `text` and `u32`. Match that
policy to a tagged value and use fixed literal formats with the proper argument
types. Reject unknown policies and tag mismatch before output. In particular,
the payload `%n` is ordinary text. C checks neither a dynamic tag/union agreement
nor a runtime format string's argument list; this adapter supplies runtime
admission, while the caller remains responsible for valid objects and lifetimes.

Predict abcdef at capacities0,1,4,6,7; preserve bytes beyond capacity as guards.
For width8/precision3 derive five spaces plus abc. A negative width requests left
justification; a negative precision omits the precision restriction. Precision
on a narrow string bounds bytes: E2 82 AC truncated to2 bytes is not a complete
Euro scalar. A Unicode-facing policy must use the later strict decoder rather
than claiming that snprintf validates a character boundary.

Compile and run format_tests.c with your implementation under GCC and Clang,
O0/O2 and the separately hosted sanitizer profile. Produce a format/type ledger
for int, unsigned long, size_t, ptrdiff_t, int32_t, double and long double; compile
wrong pairs only. Keep the intended diagnostic separate from a semantic mutant
that compiles and fails a runtime assertion. Required evidence includes actual
commands, exact bytes, result fields, query/render call counts, preserved guards,
an ownership/invariant argument and a minimized incorrect implementation.

Level0 violates a contract; level1 reproduces a visible output; level2 explains
why the negative and boundary cases discriminate; level3 independently implements
the original changed condition after at least7days. First hint: distinguish a
signed failure from a size. Second: reserve the terminator in the completeness
inequality. Third: identify the first write visible to the public destination.
Separate assessor analysis and mutants are not part of learner initialization.

Status: authored executable support with exact scoped execution in the current
library support/validation records. Source changes require a refreshed run. No
independent review, learner attempt, complete T12 gate or physical claim follows.
