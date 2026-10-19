# Shared component contracts and evidence cards

Apply these cards to the boundary named by a project. The project brief supplies
its specific inputs, allowed operations, limits and observations. A reusable
record may support several objectives; link the exact claim instead of copying
the entire dossier into every assignment.

## Card 1: representation before mutation

Write an abstraction function: what logical value does the concrete state mean?
Then state the representation invariant and every admitted state, including empty,
full, failed, destroyed and partially constructed states where applicable.
For a ring with capacity C and count n, the live sequence has n elements and
0 <= n <= C; equal cursors do not determine empty versus full by themselves.
At each mutator, establish its precondition before forming an invalid pointer or
executing arithmetic whose result is not representable.

The useful proof boundary is a state transition. If adding a node needs both a
free slot and a link update, acquire the slot first or provide an explicit rollback
path. An allocation failure must not leave the old logical value ambiguous.

## Card 2: storage, ownership and dependency profile

Record live objects, their extents, owners, borrowers and mutation rights. A
numerically unchanged pointer after resize does not establish that a previous
alias remains valid. A cancellation request does not establish completion or
device quiescence. Name the event that permits each buffer to be reclaimed.

Under the C17 baseline, alignment alone does not turn a declared character array
into arbitrary typed allocated storage. Use actual typed objects, byte operations
and value copies, or genuinely allocated storage according to the selected
contract. A target-specific mapping or firmware storage convention needs its own
implementation assumptions. Check alignment, type, lifetime and bounds separately.

Use [profiles A–F](profiles.md). A hosted test harness can call a mature oracle;
the studied target mechanism must be implemented locally. Audit source effects,
undefined symbols and the final image. Compiler helpers, callbacks, startup,
libraries and transitive allocation belong in the declared closure. Source spelling
alone does not establish an allocation-free or no-library property.

## Card 3: failure policy and progress

State whether a failing operation leaves the old state, publishes a partial result,
or enters an explicit recovery-required state. For transfers, record requested
length, positive returned counts, cursor movement, zero progress, retry and error.
For persistence, distinguish return, completion, synchronization and the actual
durability assumptions. An uncertain commit outcome must not be relabeled as a
definite abort merely because the caller saw an error.

Exercise empty and singleton cases, exact capacity, one beyond capacity and every
acquisition-failure prefix that can occur. Use invalid-input tests only through
interfaces that admit the invalid representation. Deliberate native undefined
behavior belongs in a bounded supervised fault exercise, never in a supposedly
portable functional oracle.

## Card 4: independent evidence

Begin with a handwritten prediction and a small independent model. Then add a
boundary input, a generated sequence, a metamorphic relation or a deliberately
wrong implementation that the check rejects. Merely translating the target code
into Python is weak independence. State the distinct reasoning used by the oracle.

Keep four identities: the source revision, built artifact, input and environment.
An observation belongs to that combination. A fixture replay establishes a finite
model result; a compiler run establishes a build observation; a board trace
establishes a physical observation under its recorded setup.

## Card 5: resources and explanation

Name the question and unit before collecting numbers. Keep algorithmic work,
instruction count, CPU cycles, elapsed time, CPU time, stack/heap peaks, allocation
counts and I/O traffic separate. Explain inclusive/exclusive accounting and
overlapping asynchronous spans. Compare instrumented and ordinary builds before
attributing a change to the studied algorithm.

Give a bound with its assumptions, a measurement with its uncertainty, and a
counterexample that would reject your interpretation. Measured high-water marks
and observed maximum latency are not universal worst-case bounds.

## Progressive dossier sizes

| Stage | Minimum record | Reused evidence |
|---|---|---|
| Early mechanism | Prediction, small program, observed result, one boundary and explanation | Toolchain identity and common build commands |
| Reusable component | Interface/invariant, implementation, independent tests, failure history, resource/dependency ledger | Accepted prerequisite component revisions |
| Systems integration | Ownership/timing trace across boundaries, adverse campaign, platform records and defense | Portable-core evidence whose assumptions still hold |
| Delayed transfer | Changed capacity/representation/failure schedule; explanation without the old solution | Prior record as a comparison after the new attempt |

A failed delayed gate reopens the affected claims. A stronger performance score
does not compensate for incorrect lifetime, arithmetic, authority or failure
semantics. See [assessment](assessment.md) for the common rubric.
