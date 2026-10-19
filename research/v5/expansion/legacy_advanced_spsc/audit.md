# T26 SPSC successor audit

All five T26 briefs and all48 original criterion instances/changed tasks were
inspected. This wave covers only PR-T26-01/PR-T26-04 T26.03.a/b/c, twelve exact
ordinary/delayed IDs. LRUN-18 retains the other advanced queue, identity,
hazard/epoch/RCU and history work; LRUN-19 retains separately bound kernel work.
Historical awareness/elective labels do not make these expansion duties optional.

Read actual `assessor/execution/concurrent.c` and public `cmx.h`, the complete
SPSC operations and related fixed-storage/participant contracts in the execution
README. The scalar queue exists. Its initialization returns boolean0/1, not an
enum named INVALID; the original supplement's wording requires a versioned
mapping, not rewriting the accepted API. The required ordinary multiword
payload, controlled crossover, reentry and N−1 changed-capacity experiments are
new obligations. The new AS adapter names its enum status explicitly, consumes
the existing CMX artifact in actual tests, and never claims padding or scalar
atomicity proves its payload ownership.

Directly inspected N1570 C11 draft6.2.5p9,5.1.2.4p5–25 and7.17.3p1–16;
the draft is not final C17. Read Linux circular-buffer documentation complete
lines15–138 on2026-10-17, including producer/consumer exclusivity, N−1 occupancy,
one-sided bounds and producer/consumer barrier examples. Those kernel examples
use LKMM/locking contracts; the new code derives its separate ISO atomic
relations instead of copying READ_ONCE or treating wake-up as synchronization.

Authoring/implementation is in progress until the source-bound campaign,
negative controls and actual exports finish. No independent learner, review or
physical target observation is inferred from a model or compiler result.

The first complete native campaign passed eight profiles before its composed
GCC M0 link exposed the retained initializer's `__atomic_is_lock_free` import.
That failed receipt remains `attempt01_composed_m0.json`. The successor audits
AS core and composed producer separately. An earlier compile-only pilot exposed
implicit memcpy from aggregate copying; explicit field copies repair the AS
core. The simplified standalone four-word copy did not emit that helper in the
second campaign; `attempt02_small_copy_no_helper.json` preserves the failed
expectation. The diagnostic now changes the payload to64 words and audits its
actual generated imports before expecting strict link rejection. Neither source
assignment nor freestanding compilation alone predicts a universal helper set.
