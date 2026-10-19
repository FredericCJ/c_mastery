# Failed interpretation builders

The original final suite checks an evaluation callback that writes then fails,
and direct addition text exhaustion. It does not observe a rendering builder
writing then failing, the builder's retained context effect, or a successful
maximum-length body whose enclosing lambda runs out of space. This companion
tests those exact delayed clauses without changing the frozen provider.

Evaluation construction saves the body pointer and live borrowed context. Its
failure occurs later at application: a local HEInt receives the callback write,
but the caller's value91 remains unchanged and the context count becomes1.
Rendering construction calls its builder first. HO_FAILED prevents publishing
HPFun, although the context count becomes1. A successful127-character HPInt is
also insufficient: lambda prefix10 plus body127 plus suffix1 exceeds127. The
append sequence stops at HO_FULL and the original six-character sentinel remains
byte-for-byte unchanged, including bytes after the terminator.

The127-character HPFun application is a distinct boundary: prefix1 and argument
plus separator/suffix cannot fit. Output remains the original HPInt. Three
compiled controls publish an evaluation failure value, publish partial lambda
state, or publish partial application state. Each must produce MFT_CHECK, not a
compiler failure or sanitizer trap. Six native profiles retain actual binaries,
logs and source hashes. These finite author observations neither inspect arbitrary
callback internals nor substitute for the delayed independent attempt.
