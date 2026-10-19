# Final interpretation: failed builders and publication

Complete the original `labs/higher_order/src/final.c`, keeping the public carrier
types and full higher-order contract. This companion addresses the exact changed
builder in `X-T69-012-DELAYED`. It supplies tests, not a second implementation.

Use two genuinely different dictionaries: the evaluation lambda stores a borrowed
body/context and does not run it until application; the rendering lambda invokes
its builder immediately with symbolic x and stores text. Make each builder write
a valid result and then return HO_FAILED. Before execution, predict call counts,
unchanged immediate output and retained context effects. Evaluation lambda
construction succeeds without a body call; evaluation application fails after one
call. Rendering lambda construction itself fails after one call. Equal denotation
under pure successful builders does not equate these operational schedules.

Next let the rendering builder return a valid 127-character body successfully.
The body value fits its own carrier, but surrounding lambda syntax does not fit
the function carrier. Lambda construction returns HO_FULL, preserves the caller's
entire function output and retains the builder count. Separately apply a valid
127-character function rendering to a one-character argument: the application
syntax also exceeds its output capacity and preserves its entire output.

Keep contexts live and stable for every evaluation function use. Callback effects
are not rolled back, no result publishes on failure, and no retained rendering
callback is invoked during later rendering application. There is no general
effect-type or lifetime checker in the C interface. Invalid internal lengths,
forged function pointers and nonlocal exits are outside this cooperative contract.

Compile `probe.c` with the original header and explicitly selected `final.c`,
using the same C17 flags as the free-effect companion. The campaign records six
native compiler profiles, separate learner rejection, isolated public exports,
and three compiled publication mutants per compiler. Its private oracle compares
the different schedules. Existing higher-order costs supply actual code/frame and
compiler observations; physical, external review and seven-day independent
learner performance remain separate.
