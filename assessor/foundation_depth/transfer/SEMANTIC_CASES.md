# Withheld semantic transfer dossier: X-T02-001-DELAYED

Use the parent objective rubric and distribution protocol in ASSIGNMENTS.md.
These are reasoning cases, not an invitation to execute undefined paths. C17 is
the active baseline. The listed N3096 function-call passage is a separately
identified draft comparison; function-prototype rules differ between editions.

1. Start `int x=1,y=2`. `f` executes `return ++x;`; `g` executes
   `return y*=2;`. Classify the ordering and observable result of `f()+g()`.
   Then use a single `int s=1`, with `f` executing `return s++;` and `g`
   executing `return s*=2;`. Enumerate result/final-state pairs and explain
   whether shared state alone creates undefined behavior. Finally replace the
   calls with the argument expressions in `sum(s++,s++)`, where `sum` has a
   visible two-int prototype.
2. With `unsigned short a=65535`, derive the promoted type and mathematical
   value of `a+1`, then assignment back to `a`, under (i)16-bit unsigned short
   and32-bit int, (ii)16-bit unsigned short/unsigned int and16-bit signed int.
   Assume the ordinary binary ranges stated in the parent analysis. Explain
   why the equal stored result does not establish equal evaluation semantics.
3. Supply this unfamiliar header, treating each test as a separate translation
   unit: `typedef struct { const unsigned char *p; size_t n; } PacketView;`
   and `int packet_at(PacketView v,size_t i);`. The implementation's documented
   precondition is a live array of `v.n` bytes and `i<v.n`. Compare
   `packet_at(3,0)` with `packet_at((PacketView){a,2},2)` for a real two-byte
   array `a`. Does a successful translation discharge the second precondition?
   Propose a failure-atomic checked wrapper, including error and output lifetime
   contracts, without claiming it can validate arbitrary pointer provenance.
4. Consider a registry whose epochs have all been retired. A maintenance API
   resets them to1 while callers may retain old serialized handles. Produce a
   complete history showing revival. State a sufficient maintenance protocol
   under which reuse could be admitted, and what prevents an ISO C type from
   proving that protocol automatically.

Assessor analysis (withhold with this file): case1 separate state yields6 with
`x=2,y=4`, whichever function executes first. Shared state permits `(5,4)` and
`(4,3)`, because the function-body executions are indeterminately sequenced;
grouping by `+` does not select their order. The argument modifications in
`sum(s++,s++)` are unsequenced and give undefined behavior, with no portable
golden result. Replacing two function bodies by two argument expressions changes
the sequencing premise. A diagnosis saying “shared variable always means UB”
or “a comma in the argument list sequences arguments” fails this criterion.

Case2: model(i) computes int65536 then converts to unsigned short0; model(ii)
computes unsigned int0 by modular arithmetic, then stores0. Case3's first call
violates a language constraint and needs a diagnostic; the second can translate
but violates an API/index precondition and makes the implementation's actual
array access undefined. A valid wrapper first checks the metadata/index, stages
its result and changes the output only on success. Truthful native storage and
valid output objects remain preconditions. Do not infer either classification
from one compiler's output.

Case4: issue a handle at epoch1/generation1, retire its registry slot through
three binds/unbinds, reset, bind a fresh owner with local generation1, then query
the saved handle. Numeric equality revives it. Reuse needs proof that all old
handles and in-flight operations, including persistent copies, are unreachable,
or a separate nonreused namespace. A caller promise is not an ISO C guarantee.

Graduated hints: distinguish argument evaluation from function-body execution;
write promoted types before values; separate translation constraints from API
preconditions; draw an identity timeline. Award level2 only with each relevant
rule and counterexample; level3 requires a sealed fresh argument under changed
premises. No finite compiler run proves these semantic arguments or unaided work.
