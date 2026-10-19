# Private corrective-edition assessment

## T05.06.a and changed allocator policy

The central repair is an interface boundary, not a guess about what a NULL
pointer means. Refuse zero before calling a provider; require its nonzero
failure to retain the old owner; stage the returned pointer and publish only
after success. Newly exposed bytes are explicitly initialized. Release first
revokes the control's ownership, then invokes the matching callback once.
The no-reentry and valid-provider premises matter: a callback that consumes
the old allocation before returning failure cannot satisfy the interface.

The direct policy controls demonstrate two defined original behaviors with
real storage. The wrapper's zero-call count remains zero for both. A proof
must identify the admission branch, not merely compare two final outputs.
Mutation1 deliberately forwards zero from an empty control, producing a safe
count violation; mutation2 drops the old pointer on injected failure; mutation3
omits release; mutation4 retains the adapter's0xA5 extension. The harness tracks
allocation authority independently and frees it even after an assertion, so
these are semantic controls rather than deliberate leaks or dangling accesses.
The real-realloc companion never evaluates its saved old pointer after success.

## T05.06.b and DR452

N1570's general zero-size paragraph and realloc failure wording created the
interpretation problem described in DR400. The initial suggestion mentions
errno; the final corrigendum instead distinguishes nonzero failed allocation
from zero and explicitly allows implementation-defined old-object deallocation
in the latter case. The converted issue's Fixed-in-C17 metadata supports a
bounded change-record claim. It is not an inspected copy of published C17.

DR452 is a different issue. Its discussion moves from non-lvalue expressions
and loop-invariant/effective-type concerns to a final addition about temporary
objects behaving as if declared with their value type for effective type,
without requiring unique addresses. Check its own final text and status.
The test observes address equality only inside one full expression; it does
not assert either boolean as a universal result or use temporary storage after
its lifetime. The proposed response's implementation-defined answer must not
be inflated into arbitrary pointer retention or temporary mutation permission.

## T05.06.c and release-note scope

The three features appear in an actually inspected2011 draft, so their claimed
introduction by C17 is false. The N1570 foreword attributes designated
initializers to the second-edition changes, and atomics/generic expressions
to the third-edition list. Keep that evidence distinct from reading the exact
published earlier editions. Current compilation in earlier modes is useful
support but cannot by itself date historical adoption.

GCC8's release note announces compiler mode/options and a version macro. It
does not certify an external libc, every defect correction, helper availability
or a different ABI. Current installed compiler runs must never be labeled GCC8
runs. Require a learner to name one concrete untested layer and a meaningful
next probe. The preserved capability helper-negative experiment supplies an
actual example of language acceptance without a complete linked operation.

## Evidence boundary

The synthetic fixture's2018/2028 numbers are ordering data, not a prediction
of an ISO edition. Its output is true,false in both introduced and corrected
columns under the stated model. No finite oracle, supplied analysis or source
status metadata constitutes independent learner assessment or whole-standard
conformance. Grade the exact three objectives and their transfers separately.
