# Assessor: declared users and conditional ceiling arguments

The public14-field contract maps the three original criteria and their exact
DELAYED variants. Evaluate all three independently. Preserve the original
historical body/receipt and report only the successor's actual scope.

The retained CMX core already computes minimum base/held ceilings and iterates
donations transitively. It checks base against a ceiling, but has no list of
authorized users. An undeclared task of priority5 could pass an Aceil2 check;
the wrapper must reject it. The declared priority1 addition similarly requires
a new table with Aceil1. Frozen base priorities avoid silently invalidating
offline table analysis. Refusing dynamic changes is an explicit implemented
policy, not a claim to implement safe live reconfiguration.

`sc_init` copies a temporary core whose registered mutex addresses initially
refer to its local storage. Rebase every registered pointer after publication.
No waiters exist during init, so there are no live task-object pointers to
rebase. After initialization, stable-address storage is a precondition.
The owned model stack intervals are only CMX registration identities; the
emulator does not dispatch these as actual task stacks.

A blocked acquire can acquire ownership in the core before its caller polls.
The wrapper must account for that pending-owned state, not allow an unlock
against an uncollected result or accidentally append the same ownership twice.
Cancellation/timeout ends donation but does not fabricate ownership. Sequence
exhaustion prevents fresh wait enrollment while allowing existing cleanup.
No forced termination or robust owner-death behavior is part of this adapter.

T52.04.a's general ordered-release experiment may remove A while B remains;
priority changes2→5→8. This intentionally differs from T52.04.b's strict
nested acquisition A,B/release B,A. The former is a useful restoration
counterexample but does not meet the cited properly-nested proof premise.
In PI mode a high A waiter and medium B waiter donate2 and5. Cancelling high
removes2 but preserves5; handing B to medium then removes5. In immediate mode
the still-owned A continues imposing2 independently of those donations.

The same three-task workload produces different direct-blocking counts even
when PI and immediate ceiling have the same useful-service trace. Immediate
boost at low's entry keeps high ready behind an equal-effective-priority owner;
PI permits high's attempted acquire and then donates. The experiment explicitly
continues current on a tie. The retained CMX fixed-priority scheduler rotates
ties, so it cannot silently be substituted as the scheduler used in this
argument. This distinction is enforced by a semantic control, not a footnote.

For the HLP single-blocking argument, two lower-priority critical sections
with ceilings at least as urgent as high cannot both be entered before high
under the stated single-CPU continuation/nesting/no-self-suspension premises.
The second low task would have to preempt an owner whose ceiling dominates
its nominal priority. Physical231's proof prints a questionable task subscript;
the valid comparison uses the preempting task's nominal priority. In integer
time, the chosen lower section must begin at least one tick before high's
arrival, giving the book's delta−1 convention. Do not subtract1 for continuous
time or an unmeasured physical duration.

Original PCP additionally rejects entering a free resource if another owner's
system ceiling is not below the caller's nominal priority. The supplied Python
predicate comparison only demonstrates that distinction. It is not a complete
PCP lock, waiter transfer algorithm or proof. The actual implementation here
is immediate ceiling plus inherited CMX donation under explicit wrapper rules.

The two-CPU counterexample is an original event model: low on CPU0 holds A
0..3, then B3..6; high on CPU1 requests A at1 and B at3. It directly blocks
twice,2+3=5. Raising low's priority cannot prevent high on another CPU from
running, nor force low to yield between its release and next acquisition.
The single-CPU proof's critical implication therefore fails. A replacement
requires an SMP resource protocol, migration/spinning/suspension rules and a
new bound; listing one by name or reusing the UP formula fails level3.

Assess resource use, code generation, failure preservation and the full
independent state traces, not only the four named success markers. Finite
testing and author review are not an independent human proof or learner pass.
