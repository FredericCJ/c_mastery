# T63 stale events, authority and destruction continuations

These required C/D/E/F/G/I/J exercises retain PR-T63-01–08 and the B06 shared
contract. Prerequisites are the learner bounded arena, ownership ledger, trusted
dispatcher and object-lifetime explanation. Build C17 on the named hosted ABI;
the CFI comparison additionally uses Clang LTO instrumentation. No library may
supply the event-generation, log-encoding or destruction mechanism being taught.

## Original targets and concrete tasks

Implement `security_extension.h` from its stubs. A live object has a stable
control address, generation, principal and synthetic value. An event carries
generation/principal/value. Retire clears live; recreate increments generation
without wrap and sets the new owner/value. Deliver rejects stale generation,
wrong principal or nonlive state without mutation. The supplied stale event
would otherwise change the newly created object's value19 to77. This is a
bounded integrity violation, not arbitrary execution. Reaching UINT32_MAX retires
that identity permanently unless a separately trusted epoch scheme is introduced.

Binary logging accepts arbitrary bytes, including NUL, percent, newline and ESC.
Encode every byte as four uppercase `\xHH` characters and append one NUL. Admit
`n <= (capacity-1)/4` before multiplication; reject unchanged. The exact public
fixture `%`,00,0A,1B,FF yields `\x25\x00\x0A\x1B\xFF`. This is an explicit
log wire format, not generic HTML/SQL/shell sanitization. Time is O(n), output
4n+1, no allocation and no format-string interpretation.

Owner destruction accepts partially acquired first/second resources. Set closed
and detach both pointers before callbacks, then destroy second followed by first.
Callbacks may reenter destroy but may not free/mutate the control object; reentry
must reject and every acquired resource must be released exactly once. The
control object outlives callbacks. No C type system rule enforces that lifetime.

`dispatch_target.c` is a complete original intentionally flawed learner target.
The only attacker choice is one of its named modes. Three trusted callbacks have
the same function type. In same-type mode, selection of the privileged callback
changes sentinel91 to77 even under CFI; a type-valid target is not authorization.
The repaired mode rejects operation1 without a grant and retains91. The mismatch
mode selects one wrong-type function and runs only with CFI enabled, expecting a
type-check diagnostic before the incompatible call. Never run that mode without
instrumentation or equate its termination with exploit code execution.

The required indexed continuation uses a separate three-entry `const` table and
an attacker-corrupted selection0/1/2 while the original trusted request remains0.
The public target's `index N` mode demonstrates query91->92, admin91->77, and
clear91->0 under both plain compilation and CFI. Bounds reject3 before loading
the table; no out-of-bounds or arbitrary-address call is needed to demonstrate
the authorization failure. `-z relro -z now` and the process's own mapping record
establish that this actual table is not writable after relocation. ISO C const
alone does not promise an operating-system read-only mapping. Read-only storage
protects table contents, not an attacker-controlled in-range index.

Implement `dispatch_index_authorized` in the separate learner stub: require
request<3, selected==request, and trusted grant for operations1/2. The callback
type and table bounds are separate checks. `index-repaired` must allow request0
and reject changed1/2 unchanged at91. The public stub intentionally accepts and
must expose77; the assessor policy is separate. D4 now requires all three effects,
both compiler modes, observed read-only mapping, repaired checks and the bounded
index3 refusal. No table-writing experiment or weakening of host protections is
authorized. A valid same-type privileged callback is the deliberately harmless
local success condition; it is not demonstrated code injection or escalation.

## Evidence and assessment

D1: event transition proof, unchanged failures and finite identity policy; D2:
binary encoding/extent proof and exact output; D3: partial-construction/destructor
trace with reentry; D4: actual CFI build, same-type counterexample, repaired
authority result and incompatible-call diagnostic. Run
`python3 tools/run_b06_security_extensions.py`. Require three reference profiles,
two mechanism mutants and compiled stubs rejected by named assertions, plus all
four CFI build/observation checks. Retain the existing ASan and %n demonstrations
as separate targets; they establish different properties.

SD1 maps T63.03.a/b/c and T63.04.a/b/c to D1; SD2 maps T63.05.a/b/c to D4; SD3
maps T63.06.a/b/c to D2; SD4 maps T63.07.a/b/c to D3. Level0 overclaims CFI,
sanitizers or generation equality; level1 only returns expected ordinary values;
level2 derives ownership/authority bounds and demonstrates each repair; level3
solves a delayed seven-day variant with reentrant callbacks or a new queue.
Hints: distinguish control-storage lifetime from generation; close before calling
user code; then separate permitted callback type from permitted operation.
Assessor analysis is `assessor/memory_defense/targets/ANALYSIS.md`.

Only these original local processes and synthetic values are targets. Use no
external files, addresses, services, credentials or host hardening changes.
Changed conditions include concurrent reclamation, a different log consumer and
an optimizer/unequal-length timing audit. The current serialized/event and CFI
results do not establish those claims; equal source iteration counts do not prove
constant time. Author receipt is `security_extension_validation.json`; learner,
external assessment and physical measurements remain separate.

Inspected reference bindings: R-B06-FOLLOWUP-CFI, R-B06-FOLLOWUP-C11-TYPES.
Exact passages and inspection limits are in
`research/v5/expansion/b06/followup_resources.json`; source titles alone are not
inspection evidence. Retain those locator bounds when changing assumptions.
