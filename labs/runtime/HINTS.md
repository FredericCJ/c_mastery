# Staged runtime hints

Open one level only after saving a prediction and smallest failing case. These prompts guide the mechanism and contain no implementation.

| Stage | Level 1: representation question | Level 2: invariant | Level 3: experiment |
|---|---|---|---|
| Values/wire | Which bytes have stable meaning across targets? | Decode fields before publishing the program. | Change one magic/length byte; retain the old destination. |
| Verifier | Is this offset an instruction start? | Every control-flow join has one relative stack height. | Jump to byte 2 of an INT, then create a mismatched join. |
| Dispatch | Which tag and range does this opcode require? | Never execute overflowing C signed arithmetic. | Pair INT32_MAX+1 with INT32_MAX+0. |
| Calls | Which state belongs to an invocation? | Each frame restores its caller's operand base. | Trace two calls to the same function with different arguments. |
| Compiler | Which binding does this name denote? | Initializer and body have different let environments. | Shadow an outer name, then use it after the inner binding exits. |
| Branches/loops | What value does every expression leave? | Both if paths join at equal height; while body net result is discarded. | Trace zero, one and five iterations, then use an integer condition. |
| Arena | Is unreachable the same as free in this phase? | Allocation never reuses an occupied arena slot. | Make seventeen unrooted pairs in a sixteen-slot arena. |
| Collection | Can an allocation occur while a reference lives only in C? | All live roots are visible before sweep. | Fill the heap and construct a parent around a temporary child. |
| Budgets | Which ledger pays for this operation? | Guest, collector and host counts have separate units. | Exhaust one while checking that unrelated ledgers stay unchanged. |
| Host/store | Does callback failure establish rollback? | A failed commit can have uncertain persistence. | Inject commit failure and require recovery before another write. |
| Ports | Is compile success target execution? | Core source and bytecode identity survive the adapter change. | Compare Linux output, emulated Thumb transcript and actual own-kernel result. |
| Defense | Which observation could reject your explanation? | Each claim has a bounded artifact and an independent challenge. | Change a capacity after a week and predict the new boundary before running. |
