# T28.01/.02/.04 assessor analysis

Do not export this file or the reference implementations. Retain original task
clauses, rubric and delayed IDs; each leaf must receive its own judgment.

* .01.a: show CONTROL16 rejection with unchanged value/state; revision1 rejects
  bit2 and revision2 accepts it. Do not change the model's W1C meaning to make a
  revised mask test pass. The target GPIO map is a separate register contract.
* .01.b: direct5 leaves10, flawed RMW10 leaves5; with new bit16 the latter leaves21,
  correct5 leaves26. DATA0 is valid data and the next255 remains queued. RTC5 then0
  is expected; diagnostic code must use one captured result. Mutants invert W1C,
  consume two bytes, or forget RTC clearing and must fail semantically.
* .01.c: step-ready at polls1,2,3 succeeds exactly once; no readiness times out
  after3 with zero publications. IRQ mode implements a distinct clock/deadline adapter over the same readiness
  graph; arming rechecks existing readiness and a separate deadline event prevents
  waiting forever when no IRQ occurs. Mutants omit the arm recheck or time out
  one tick early; both compile and fail. The service cannot infer actual interrupt masking,
  scheduling or device acknowledgment from that equivalence.
* .02.a: static layout has IDR0x10 and BSRR0x18. Audit emitted word accesses and
  empty imports on linked M4 images. Actual big-endian PPC64 executes model
  fixtures, not F439 MMIO. The verifier reuses the declared ELFv2 Linux bootstrap;
  its original byte helpers are host-verification dependencies, not board runtime.
* .02.b: `payload=7; volatile_ready=1` versus reader polling volatile_ready then
  reading payload has no synchronization edge between ordinary C threads. Both
  flag and payload can race; never assign a portable output. With release/acquire
  normal-memory publication, also establish that the acquire reads the relevant
  release, and separately handle the device/cache/ownership contract. A DMB or
  cache clean chosen without the specific platform protocol is not a universal
  repair. The inspected N1570 is C11 draft evidence, not published C17/C23 text.
* .02.c: the scripted sequence yields6 for ODR RMW and14 for BSRR. A later ISR
  whole-ODR assignment can overwrite14, so bit atomicity is not application
  ownership. The deliberate GPIO RMW mutant fails its access-log/encoded-mask
  check before it can be misdescribed as a correct hardware transaction.
* .04.a: mode/AF/output type/pull/speed/package/bridge are separate columns.
  Open-drain-high with no pull floats; push-pull-high plus peer-low contends.
  PB0 pin46(LQFP144) becomes35(LQFP100), while LD1's routing still depends on
  assembly. A renamed port bit is not evidence of a verified physical wire.
* .04.b: include pin15, empty mask, overlap and out-of-range masks; no ODR read.
  Actual frozen `fw_gpio` is composed through this side-effect adapter for all
  16 pins and both levels. Two separate BSRR writes cannot make two ports atomic;
  derive the intermediate pair and either tolerate it or require a separate
  latch/peripheral synchronization mechanism with its own inspected contract.
* .04.c: threshold3 changes only on the final sample of0,1,0,1,1,1. In duration
  mode, samples at UINT32_MAX-4,1,3 are insufficient for10ticks;5 completes the
  interval. A counter-based duration mutant accepts too early. Gaps restart the
  candidate, backward/half-range timestamps reject, and counter exhaustion is
  transactional. An alternative integrator filter is not equivalent unless its
  acceptance semantics are explicitly changed and assessed against a new model.

Score0 for a false access/lifetime/physical claim,1 for a reproduced happy path,
2 for the specific mechanism, boundary and falsification above,3 only for the
original delayed variant performed independently after at least7days. Record
each artifact/hash, input premise, prediction, trace, counterexample and remaining
physical/source limitation. Shared tests do not award untested leaves.

The independent oracle uses sets for flags, union/difference for bit transactions
and a backward suffix of observations for debounce; it does not call the C
implementation to compute expected values. Finite cases bound the conclusion.
No native data race or physical contention is executed. All targets are original
synthetic programs; no external circuit or production equipment is contacted.
