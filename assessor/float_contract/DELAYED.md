# Withheld seven-day assessment: PR-T04-04

Administer no earlier than seven days after the corresponding initial attempt.
Use the retained `AX-PR-T04-04-T04.xx.x-DELAYED` identities; do not record author
reference execution as a learner attempt. Supply the public interface/contract
and the learner's earlier artifact, withholding this analysis and reference code.
Require prediction and admission/error bounds before exposing raw results.

| Parent | Changed task and supplied premises | Required evidence and discriminating judgment |
|---|---|---|
| T04.09.a | Run the narrow long-double64 build with no long-double libc calls. | Actual precision equality, ABI caveat, a revised exact/MPFR oracle; reject nominal-width authority. |
| T04.09.b | Provide the documented SSE FTZ control and runtime normal operands DBL_MIN,0.3. | Save/restore all original control/status bits, observe gradual versus zero, distinguish input DAZ and x87; classification must not invalidate restoration check. |
| T04.09.c | Change the rational model to radix10 and denominator3. | Derive recurring remainder, prove nontermination and contrast denominator10. |
| T04.10.a | Supply2048 alternating pairs with2^-50 perturbation. | Prewritten naive bound and exact dyadic argument, raw three-method results, explanation of when either proof fails. |
| T04.10.b | Change to downward rounding and then wider x87 evaluation. | Revised product-rounding predictions and generated operations; distinguish matching values from matching mechanisms. |
| T04.10.c | The caller uses zero sign to choose a directed boundary. | Demonstrate equal numerical/ULP zero yet opposite required sign, implement the additional policy without changing finite ULP distance. |
| T04.11.a | Caller has FE_DIVBYZERO already set and downward rounding; inject hold/mode/restore failures. | Preserve state when restore works, no partial publication, explicit containment error when it cannot work. |
| T04.11.b | Supply the GCC x87 flags and require a volatile-product comparison. | Actual object inspection and runtime values, semantic store/return rounding, no unexecuted wider-target claim. |
| T04.11.c | NaN represents missing data in the calling application. | Local counterexample to fast filter, strict admission repair, equivalent finite-only timing contract and residual signed-zero policy. |
| T04.12.a | Use float then radix10 precision3 and7 models. | Algorithmically regenerate both-side spacing, endpoints and midpoint decisions, label actual versus modeled evidence. |
| T04.12.b | At53-bit oracle precision the exact answer is1+2^-53. | Directed enclosure refuses unique output,128-bit enclosure resolves it; explain input conversion and final ties-even rounding. |
| T04.12.c | Change sum to dot of `[1e16,1,-1e16]` with three unit weights. | Derive a fresh product/addition bound; preserve tolerance1/4 and distinguish algorithmic guarantee from desired accuracy. |

Score each criterion independently:0 incorrect;1 reproduces observations but
cannot derive;2 correct mechanism, boundaries and evidence limits;3 independently
repairs/transfers with explicit changed premises and a discriminating counterexample.
Capture start date, original artifact hashes, supplied premises, commands, written
prediction, result, assessor rationale and unresolved weaknesses. Human assessor
identity and elapsed delay remain empty until a real assessment occurs. All
programs are original local teaching targets; no physical access or external
service is authorized. The author campaign implements these variants now, while
their independent delayed evidence remains pending.
