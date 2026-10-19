# Assessor analysis: allocation and exact original transfers

The reference imports accepted f_checked_size rather than reproducing its overflow
test. It separately checks rounding addition because a valid product can still
overflow after alignment padding. Count/width zero is refused; release is a
distinct operation. Preserve pointer, length, capacity and alignment as one owner.
The operations/context pairing and unique ownership are caller obligations, not
properties expressible through an ordinary C function signature.

The supplied bounded backend keeps3 physical slots alive while tracking logical
allocation status. This safely exposes the lost-owner and leaked-staging mutants
through balance assertions, without requiring a use-after-free. The native notebook
then exercises real allocation lifetimes on admitted arguments. Do not infer native
allocator failure behavior from the synthetic backend; each has a separate claim.
Copy-failure cuts0..4 cover before-copy, every partial prefix and failure after all
four bytes were copied. Even the last failure cannot authorize public commit.
After a successful native realloc, do not inspect or compare the prior dangling
pointer; verify the new owner and preserved bytes instead.

Withhold these original DELAYED packages: T12.07a supplies an alignas64 concrete
type, requires an actual declared supported profile and shows why ordinary realloc
does not promise preservation of extended alignment. T12.07b gives a callback
whose zero-null result retains storage and requires exactly-one explicit release,
with no ambiguous zero request delegated by the adapter. T12.07c replaces native
resize with a copy that can fail after modifying the new destination; require
rollback at all five cuts, allocation failure, retry, shrink and resource accounting.
These are implemented assessor variants, not completed7-day learner assessments.

Alternative valid approaches include a no-fail-copy precondition if the original
changed task's choice is explicitly justified and a failing callback is rejected
before ownership transfer. It cannot silently claim to support fallible copying.
Byte zeroing is insufficient evidence for typed null/float initialization. A
support predicate naming align64 is a selected backend contract, not proof that
every ISO implementation supports it. Native library measurements are bounded
observations; exact published edition access remains a separate source gap.
