# Required defense transfers DT1-DT3

Retain PR-T63-02,04,08 and `curriculum/v5/contracts/b06.md`; families A/B/C/D/E/F/H/I/J
apply. Prerequisites are checked-extent arithmetic, valid C object lifetimes,
borrowed-view contracts and source/disassembly comparison. Implement public
`defense_transfer.h` stubs using hosted C17. Rust2021 is an external language
comparison, OpenSSL CRYPTO_memcmp an external benchmark. Neither may implement
the first-principles C admission, view check or equality scan.

## DT1: changed data model and allocation shape

Map T63.02.a/b/c. `dt_extent(count,width,header,alignment,tail,limit,out)` returns
the aligned-header extent plus payload and tail. Width is nonzero; alignment is
a power of two in1..4096; all values are uint64. Model a32-bit or64-bit allocation
limit explicitly rather than inheriting host size_t. Reject any intermediate or
total exceeding limit before evaluating an overflowing operation; leave out
unchanged. Count0 is valid. Base allocation alignment is a caller precondition;
this value calculation does not create storage or validate an arbitrary pointer.

For count0x40000000,width8,header24,alignment16,tail16, derive8589934640. It rejects
at UINT32_MAX and succeeds at UINT64_MAX. Model the flawed wrapped32 calculation
as an undersized synthetic capacity; no real out-of-bounds write is necessary.
Include header-padding overflow, nonpower alignment, zero width, exact limit and
UINT64_MAX count. D1 supplies division-before-multiplication and subtractive
admission proofs, changed-width counterexamples and unchanged-failure tests.

## DT2: public C view versus safe-language borrow

Map T63.04.a/b/c. Two live synthetic arrays contain19/27 and77/83. A public C view
can be forged with the second array's pointer and the first owner's tag. The
vulnerable tag-only check reads77 without undefined memory access. Repair the
original target by comparing the view's pointer/extent with a trusted expected
owner selected outside attacker input, then checking index before access.
The attacker's capabilities are editing view fields; they cannot replace trusted
owner storage or execute arbitrary C inside the validator. All denied reads leave
output unchanged. This is an application authority boundary, not an unforgeable
ISO C capability or a general proof that arbitrary pointers can be validated.

The supplied Rust negative fixture mutates a vector while an immutable slice is
used afterward. Require the actual E0502 diagnostic; the positive fixture ends
the borrow's last use before mutation and executes. C permits mutation through
another valid pointer while a const-qualified view exists; demonstrate the new
value31 through that view. D2 separates ISO const access, runtime owner checks,
safe Rust enforcement, diagnostics, unsafe/FFI obligations and programmer
discipline. A C struct's similar API does not supply Rust's language guarantee.

## DT3: equal-length full scan and public-length policy

Map T63.08.a/b/c. `dt_equal` accepts live byte spans0..256 and distinct writable
steps storage; input spans may overlap each other but must not overlap steps.
Invalid arguments return -1. Lengths are public: unequal lengths return false
with0 steps. Equal lengths scan every byte using accumulated xor/or, returning
equality and exactly n steps, including zero-length equality. The attacker controls
synthetic bytes and observes result, scan count and local timing; no external
service, credential or private data is involved. The intentionally early-exit
mutant violates the full-scan property at the first differing byte.

D3 tests every length and mismatch position, then compares GCC/Clang optimized
code with vectorization enabled/disabled. Retain all conditional branches and
trace their inputs to lengths/addresses or contents; source-level uniform work
alone is not a machine constant-time guarantee. D4 retains72 raw timing samples
per configuration for first/middle/last mismatch and equal inputs, including
the external CRYPTO_memcmp comparison. That library's equality result cannot be
used as lexicographic order. Changing unequal-length policy changes observable
information; document it before interpreting timing. No timing distribution
proves absence of all side channels, speculation effects or compiler changes.

## Verification, rubric, hints and variants

Run learner tests first; the opt-in author command is
`python3 tools/run_b06_defense_transfer.py --rustc <actual-compiler>`.
Require semantic failure of missing-overflow, forged-owner and early-exit mutants
and unsolved stubs, plus actual Rust positive/negative controls. Retain exact
compiler flags, library/compiler versions and generated-code review. The report
is `research/v5/expansion/b06/defense_transfer_validation.json`.

For each DT criterion, `.a` is derivation, `.b` implementation and boundaries,
`.c` independent changed-condition reasoning. Level0 states an overstated safety
claim; level1 passes ordinary cases only; level2 satisfies the stated attacker,
ownership, arithmetic and observation contracts; level3 solves the delayed
variant unaided after seven days. Hints: reserve overhead before dividing;
distinguish trusted control from a copied tag; trace the final branch condition
back to its inputs. DT1-DELAYED adds aligned element stride, DT2-DELAYED changes
the trusted owner lifetime/generation, DT3-DELAYED hides lengths with a fixed
maximum scan and explicitly defines padding/accessibility.

Inspected references are R-B06-FOLLOWUP-RUST-BORROW and
R-B06-FOLLOWUP-OPENSSL-COMPARE, bounded to the registered passages. These author
checks do not claim independent learner mastery, external review or physical
timing guarantees. The separate assessor analysis and implementation stay withheld.
