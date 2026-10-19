# T52.08 assessor analysis

The reference uses actual retained CMX sleep/tick/cancel operations. Its wrapper
restricts creation and observations to a half-range, caches terminal results and
requires collection before reuse. The original CMX API alone does not establish
the caller's temporal provenance. Eight-bit exhaustive comparisons assess that
contract; they cannot recover how many whole counter cycles actually elapsed.

The conversion divides duration by the full denominator before multiplying the
quotient. The remainder term is bounded by the declared frequency parameter
limits. Checked shift/add handles a quotient whose multiplication would exceed
64 bits; restoring division avoids an implicit freestanding division library.
The absolute policy includes call phase before rounding. Relative rounding of
11ms at5ms/tick gives3 edges; a4.999ms call sees only10.001ms before the third
edge, while the absolute policy selects the20ms edge. Zero duration rounds the
current absolute instant onto the grid; it is not an infinite wait.

The prepared token binds one plan and cannot wrap. Mutations can occur between
planning and commit only under the declared serialized protocol. Commit observes
time, lets actual CMX publish due sleeps, recomputes the next deadline and refuses
sleep if the selected event changed or any wake is ready. A pending timer is
accounted before clearing its status. Exhaustion refuses new work while retaining
cancel/collect/manual-advance/abort cleanup. Retiring a timer means no new plans,
not leaking an infinitely waiting task.

Resume stores both the consumed auxiliary sample and a remainder. Reusing the
same sample contributes zero elapsed time. A fractional ratio may yield no new
integer tick on one call without losing time; later samples include that retained
fraction. The independent counter must run and remain within the externally
established observation horizon. No modular API can detect an arbitrary number
of unobserved complete wraps or establish a board's low-power clock wiring.

The actual QEMU adapter owns CMSDK timer0 and SysTick, serializes with saved
PRIMASK and reads the former after programming the latter. Its pending-delay test
lets the actual SysTick become pending while masked, then takes the retry path
and publishes the due event before clearing pending state. Earlier insertion
replaces the old programming. Equal deadlines share the same actual observation.
The stopped-clock test disables SysTick while the independent emulated timer
continues, then accounts the interval exactly once. These are real device/IRQ
observations in the emulator, not physical sleep or oscillator measurements.

Reject a learner who treats raw subtraction, a successful host test, an actual
IRQ, an emulator lateness threshold and a physical deadline proof as equivalent.
Require every mutated control to reach its intended semantic assertion, with
successful compilation and audited target helpers. Finite row agreement and
author-team inspection do not establish independent learner mastery.
