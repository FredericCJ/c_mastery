# Separate assessor analysis: transceiver evidence boundaries

The original six AX criterion objects and V1 tasks are preserved. Assess the
specific relation below in addition to the common rubric. Source/physical
claims must be checked independently of C test output.

## T38.07.a

Idle1 maps through the synthetic negative line to1; zero is unknown and−3V is1.
The selected MAX3232 receiver is an intentionally different contract: the
guaranteed extrema partition at600/2400mV, not a typical1500mV decision. Across
integer samples−25000..25000 inclusive,3600 lower-center and600 upper-center
values differ from the ±3000 fixture, total4200. The selected open receiver
high is not receipt of a transmitted idle symbol. Mutation1 converts typical
threshold into a false guarantee; it must fail within the exhaustive interval.
The V1 explicitly uses the selected actual datasheet and keeps intermediate
hysteresis/history unknown rather than asserting an uninspected transfer curve.

## T38.07.b

The original three-contact adapter has a precise published drawing. Straight
DTE/DCE is the only acceptable mapping among27 ordered right-contact triples;
two DTEs require1↔2 and3↔3. Outputs cannot be joined just because their labels
look alike. Mutation3 accepts the output-output counterexample. No handshake
facility exists, and no DB9 mapping follows. The V1 is the independently
documented original null-modem adapter, with actual continuity required before
physical use; it is not a claim about an arbitrary commercial adapter.

## T38.07.c

The repeated formatter experiment exercises256 byte values×10bits. The actual
UaTx/PuDriver16-byte composition performs two8-byte chunks and returns only
after final TC; its RAM callbacks make no electrical measurement. A capture
missing line voltage must preserve the result and return INCOMPLETE even if
logic/returned bytes agree. Mutation4 violates this admission boundary.
The changed cable/load/rate is a new physical run: source guarantees, probe
loading, analog envelope and settling must be reconsidered. Rejecting a
2500pF contract in this deliberately narrow checker is not proof the device
cannot operate there; it requires a separate justified profile and evidence.

## T38.08.a

The synthetic +200mV threshold yields1, +199mV unknown. Compute A−B and A+B in
a wider signed representation before admission; a narrow signed subtraction
can overflow before a later check. The two sign conventions cover28002 input
decisions. The selected powered AM26LV32E has open-input high and disabled
high impedance; a connected zero differential is unknown. Its documented
revision history removed the broader short/terminated fail-safe claim. Mutation2
must fail at zero. The V1 adapts only sign; it does not silently change the
application's byte meaning or authorize an out-of-range common mode.

## T38.08.b

Multidrop fan-out is not multipoint drive permission. The scoped one-driver
topology rejects two active drivers and eleven receivers. The changed original
three-peer coordinator enforces a single generation-tagged grant, delayed
release after transmitter completion plus checked disable/guard, and no wrap.
100601ns with TXE but without TC retains the lease; TC at200600 plus200+2000
allows readiness at202800. Mutation5 admits a second owner. This protocol does
not make AM26LV31E an RS-485-compliant transceiver, prove arbitrary peer
discipline, or supply hardware fail-safe. Require a separately selected circuit
and physical evidence before that extension is claimed operational.

## T38.08.c

Stable swapped samples contain five opposite decisions and no crossings;
the sign adapter explains them. The supplied recrossing sequence has three
known-state transitions and two opposite samples. Adding equal10V offsets to
both lines preserves the differential yet violates common mode; the analyzer
must classify that sample outside the envelope, not “good differential”.
Mutation6 hides the transitions. These synthetic results identify observables;
they cannot uniquely infer reflection rather than noise/probe error. The
changed cable-length task requires actual raw differential and common-mode
captures, controlled intervention and uncertainty analysis. A disconnected
receiver producing zeros proves neither valid traffic nor the selected open
guarantee; check power, enable, short/termination and actual pin identities.

## Validation and limits

Require source-bound six-profile positive/negative results, actual closed Thumb
image and compiler frame audit, original fixture/producers, and isolated learner
export rejection. Nonrecursive callback paths and fixed test storage support
the stack argument; IRQ nesting and physical watermark are outside it. All
current physical observations, external review and independent delayed learner
attempts remain not-run. Finite checks can falsify these mechanisms, not prove
universal correctness or absolute mastery.
