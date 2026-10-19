# Variadic and hygiene key

T06.04.a: a C17 all-variable wrapper forwards a nonempty sequence including the format, so an empty message is a nonempty argument list containing an empty string token. A wholly empty call expands to a function call lacking its required argument. A named-format form needs an additional argument under the earlier draft constraint; use a separate zero form rather than relying on comma deletion.

T06.04.b: GNU named comma deletion distinguishes omission from a supplied empty argument. EMPTY and NESTED are supplied tokens before expansion and retain the comma in this extension form. C23 __VA_OPT__ considers the hypothetical substituted variable sequence; all three empty variants omit its comma. The actual initializer lengths1/1/1/1/3 corroborate that bounded case. The GNU dangling-comma specimens are preprocessing observations only.

T06.04.c: distinct function bodies update distinct scalar counters, each argument occurs once, and no order is asserted. Disabled replacement is void0, so none of its arguments are evaluated. Duplicating the sink expression via the comma operator is defined but violates the once-only contract and is a valid mutant control.

T06.05.a: the unparenthesized expansion becomes 2*3<4?3:4 and yields4. Parentheses fix grouping but not repetition; the chosen smaller left operand is evaluated again. The function evaluates both arguments once. Generic selection chooses a function designator without evaluating its control; the later Box arguments execute normally. Do not infer machine-code inlining.

T06.05.b: do-while converts multiple operations to a statement accepted in if/else. It also creates the nearest loop, so its continue and break cannot control the caller loop. Both counterexamples increment the following outer statement three times. The intended wrapper excludes caller-directed control transfer.

T06.05.c: the inner initialized temporary shadows the caller variable, so the macro assigns the inner variable and outer9 remains. This avoids undefined self-initialization while demonstrating wrong binding. Typed function parameters avoid textual capture. __LINE__ is insufficient for two expansions on the same line; __COUNTER__ is a separately documented GNU facility, not a portable unique-name or stable ABI promise. Reservation rules remain context-specific.

The oracle independently lists16 selected observations. Sanitizers are bounded diagnostic evidence, not proof. Formatting is a permitted library boundary, not a newly implemented formatter. Every delayed variant requires learner reasoning beyond matching author rows.
