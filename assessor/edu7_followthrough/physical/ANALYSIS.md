# Withheld EDU7 physical/grant analysis

<a id="t4002a"></a>
## T40.02.a

The supply, signal polarity, physical topology, terminations and driver overlap
are not observable in the host or MPS2 register model. The independent pin/part
ledger follows exact selected TI tables: SOIC D4/R1, /RE2 low, DE3 high, A6/B7,
GND5/VCC8. THVD1400's selected650ns enable and THVD1410's600ns both round up to1us;
receiver-disabled10us/11us figures cannot be used with the unchanged receiver
premise. The two numerical USART banks and copied bytes17/51 prove a bounded
software separation property, not cable isolation. Check the changed adapter's
fresh ledger; require the actual physical packet to pass its physical dimension.

The F439 second port is USART2 at0x40004400, RCC APB1 bit17, PA2/3 AF7 and PA1 DE.
Port0 retains actual USART3 at0x40004800, bit18, PD8/9 AF7 and PD10 DE. Inspect
the final non-model image and actual board conflicts. The AF6 mutant compiles
and is rejected by mapped source execution; no physical-pin execution is claimed.

<a id="t4002b"></a>
## T40.02.b

For42MHz APB1, OVER16 BRR365 gives96*10*365/42000000 seconds, about8.343ms;
BRR729 gives about16.663ms. The latter cannot meet10ms before overhead. The
fit condition is necessary but not sufficient: scheduling gaps can still exhaust
the live deadline. Tests supply a stated90us-per-byte schedule and explicit
chunk-completion observations; their9036us terminal is modeled, not measured.
The fixed frame copy consumes96bytes and12actual lower submissions. There are
only three observed DE writes including initial inactive state; interchunk TC
must not cause a direction edge. Cancel before enable produces zero writes;
cancel after a write retains DE until future TC, even after nominal timeout.

POLL is actually encoded and submitted. Final TC permits DE low and starts
both the local post-DE guard and the unchanged producer's200us earliest-start
interval. Pre-enable is separately max(200us EDU7 policy, TI maximum), not1us.
The independent oracle obtains200+95*90+86+200=9036us; two new mutants reject
short pre-enable and double guard. The early receive observation uses a fresh
monotonic schedule before the corresponding terminal guard step.
First1500 and final9500 pass even though final exceeds2200. The start-deadline
mutant removes that legal reply. Rejected lower offers do not create a grant.
Inspect the exact absolute timestamps and duplicate-token refusal. A different
wrapping timer contract cannot be inferred from unsigned types.

<a id="t4002c"></a>
## T40.02.c

PE/FE/NF/ORE belong to sample validity; local TX phase belongs to ownership. A
faulted echo increments both independent counters and invalidates the partial
parser, but cannot publish peer data. A clean fresh framed reply after the fault
can recover under the retained grant. The error-erasure mutant is rejected by
the two-counter assertion, not merely a crash. A disconnect notice is synthetic
external evidence; it cancels an owned transmission conservatively and invalidates
partial bytes. Neither echo filtering nor no software overlap proves absence of
analog contention. Assess a new trace unaided after seven days; do not infer a
learner result from this author campaign.

All ordinary/changed criterion tasks and accepted baseline objects are preserved.
Author reference code and these answers must stay out of learner exports.
