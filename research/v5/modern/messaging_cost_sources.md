# T73 cost/progress passage inspection

On2026-10-17 the named POSIX.1-2024 wait and broadcast passages were retrieved
and read. The initial browser requests returned403; Python urllib retrieved the
actual pages, whose byte hashes are in the resource record. The wait contract
couples mutex release with blocking, requires predicate re-evaluation, binds a
condition to its current wait mutex and specifies reacquisition during POSIX
cancellation. Broadcast order depends on scheduling policy. These facts support
the registry-window argument and its explicit exclusion of `pthread_cancel`;
they do not establish fairness or a hard cancellation deadline. Only the named
sections are relied upon; no system-library source was audited.

The Linux man-pages6.19 `pthread_getattr_np` page was also retrieved and read
after the browser timed out. The actual running thread's attributes may differ
from requested attributes; the returned attribute object must be destroyed.
This is a GNU extension. The cost probe records actual stack/guard attributes
and distinguishes them from resident memory or maximum stack usage. The page's
example is contextual evidence, not a measurement performed by this repository.

The measurement implementation is original curriculum code. Its conservation,
ownership and atomicity checks are independent of elapsed-time ordering. Host
measurements and the controlled finite steal schedule remain separate from
RC11 model checking, learner assessment and any physical board result.
