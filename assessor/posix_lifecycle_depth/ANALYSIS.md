# Assessor analysis

The error pipe cannot be replaced by an exit-code convention: the original
helper intentionally returns127. Preserve the child's exec errno before any
other call. Close-on-exec has an actual lifetime role; ordinary EOF can also
result from premature death and must not be described as authentication of an
executed image. Pipe/fork failures leave the output sentinel unchanged and
release acquired descriptors. Wait owns the exact child and is retried only
on EINTR. A child-side truncated/error write remains a protocol limitation,
explicitly outside the narrow happy-report assumption.

The pool's created prefix is the authority for joins. Every argument is stable
before creation and remains live until successful join; workers modify distinct
result objects, which the caller reads only after synchronization. Return codes
from pthread_create are error numbers directly. A failure at ordinal3 leaves
two created borrowers whose results sum to5. Detach does not solve caller-owned
argument lifetime. A recoverable general join-error API would need retained
heap/state ownership and an eventual reaper, which this bounded stack-based
exercise deliberately excludes and asks the learner to derive as a variant.

Four compiled semantic mutants test independent mistaken behaviors. They do not
exercise actual unsafe dangling-thread cleanup; the assessor keeps all joins
even in its accounting mutants. Sanitizer or signal failure alone is never
counted as the intended mutation witness. Finite tests do not establish the
complete broader PR/AX contracts or learner understanding.
