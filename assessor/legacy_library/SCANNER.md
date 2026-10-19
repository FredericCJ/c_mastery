# Withheld scanner reasoning

The correct %3s destination has four bytes. `%n` observes three consumed bytes
but is excluded from the assignment count. A width does not bound a numerical
value's representability. A wrong pointer and out-of-range numeric destination
are analysis-only undefined cases, never portable golden outputs or fuzz seeds
for a supposed safe numeric validator.

Missing `;` after12 still permits scanf's count assignment. `name=AB!` has two
assignments and consumption16, leaving ! unread. This differs from the parser's
transactional all-input contract. Original FCursor and f_parse_i32 are linked
directly. Scan count digits, match delimiter, validate range, then stage decoded
name and commit. A scratch record or independent offset plan is an acceptable
alternative; no requirement to imitate the reference's loop structure.

Escape states are ordinary and after-backslash. Only semicolon/backslash can
leave the latter, and EOF cannot. Checking raw length32 rejects valid escaped
32-byte names; failing to check decoded length overflows the destination. The
independent oracle uses a regular expression and Python arbitrary-precision
integer conversion rather than reproducing the cursor code. It checks19,180
inputs per compiler and compares rejection-state bytes, not merely a status.

For12x scanf assigns12 and leaves x. For x matching fails with zero assignments
and leaves x; retrying the same operation without recovery makes no progress.
Empty input fails before assignment with EOF. For12 at EOF the assignment
succeeds; an ensuing byte read returns EOF. EOF and error indicators are not
predictions. The finite line-drainer consumes the newline on an oversized
record, so retry cannot accidentally admit its suffix as a new record.

Reject mutants only after successful compilation and exact assertion exit1.
The commit mutant changes count before bad-name rejection; the suffix mutant
admits !; the drain mutant leaves an oversized suffix. Learner stubs are a
separate category. Withhold this analysis, source and generated oracle outputs
for delayed assessment. Keep original7-day criterion identities; no independent
learner performance or unrestricted parser correctness follows from author tests.
