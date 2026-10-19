# T30.05 technical analysis

The all-progress baseline advances only after every task qualifies. Therefore a
slow task's earlier commit remains usable until the next successful feed. Epoch
publication before useful commit supplies a defined false-health counterexample:
the monitor sees exactly its advertised facts but those facts do not imply health.

All accesses participating in the sequence/epoch snapshot are SC. Writers hold
the sequence odd, and no finite sequence value is reused. Equal even sequence
reads therefore exclude an intervening writer in the SC order; the atomic epoch
reads belong to one completed publication state. This is a sufficient chosen
memory order, not a claim that every compiler's atomics are ISR-safe or wait-free.
On the audited M4 target the operations are lowered inline; task callers retry
BUSY outside the primitive, whereas an ISR must preserve report ownership.

Atomic exchange moves bitmap bits to the supervisor. An OR before it is in the
returned set; an OR after it stays pending. The deliberate load/clear mutant
loses the intervening bit. Accumulating seen bits across unsuccessful windows is
necessary to preserve the original last-successful-feed semantics.

Three retry permissions increment before the caller acts. The fourth failure is
terminal. Mirrors detect each tested one-bit corruption, but simultaneous paired
changes and malicious changes are not excluded. The API is a logical record;
no torn-flash or physical reset durability is claimed. An unauthorized fresh
initialization is a policy failure outside the record's detection power.

WFI uses saved PRIMASK so the IRQ can remain pending until the core resumes and
the handler runs after restoration. WFE first consumes a previous event and then
uses SEVONPEND to bridge publication before the wait; every return rechecks work.
Neither instruction promises power savings or nonspurious sleep. QEMU's WFE
implementation is only yield/NOP, and its SLEEPDEEP is RAZ/WI. The first exploratory
guest incorrectly assumed the latter bit could be set, entered ordinary WFI with
no remaining wake source, and was killed by the owned timeout. The corrected
harness explicitly verifies that model limitation, tests SLEEPONEXIT refusal,
and keeps the independent event-latch model separate from executed instructions.

F439 Stop differs materially: HSI/HSE/PLLs stop, SRAM remains, and HSI resumes as
SYSCLK. This package admits only HSI16/noPLL/normal-regulator before and after;
it cannot pretend that a previous PLL frequency was restored. PVU/RVU busy must
refuse entry per ES0206 Rev24, despite the generic RM note. A timestamp backed
by a retained source permits a delta; external wake alone provides no elapsed
time, so the new epoch is explicit and elapsed_known is false. Phase1 prevents
consumer restart before the clock check. Standby needs reset-entry and retained
backup-record logic, not a return from this API.

Energy is an exact sum in fJ: mV·µA·µs. Overflow rejection is before publication.
The lower current of an idle state alone says nothing about energy per completed
transaction; wake cost and batch size alter the denominator. Physical integration
requires actual bandwidth/timebase/shunt/trace configuration and uncertainty.

Assess every leaf and its changed condition separately. Reject a nominal LSI
number advertised as a worst-case timeout, a QEMU run advertised as physical
Stop/WFE validation, a crash advertised as intended mutant evidence, or a passing
sample advertised as independent mastery. No independent reviewer is invented.
