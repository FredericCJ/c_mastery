# Withheld PHY management analysis

The public contract binds the original three T39.05 criteria and delayed IDs.
This analysis is assessor-only and is not exported to a learner workspace.

## T39.05.a

At168MHz the selected divider is102, code4. PHY0/register2 contributes0x80,
CR contributes0x10 and MB1 contributes1, giving0x91. MDC is approximately
1.6470588MHz, independent of the continuous50MHz two-bit RMII reference.
10MHz lies outside the declared divider profile; selecting code2 by rounding
is a contract violation even if the numeric MDC rate appears conservative.
The real adapter accesses two four-byte-spaced words. The wrong stride mutant
tries MIIDR0x40028018 and fails the original address model. The selected
32-bit access audit is not a false claim that RM0090 prohibits smaller accesses.

Board tables identify the same selected routes across the inspected revisions,
but current Rev11 does not name the fitted PHY in §7.11. Require actual board
and PHY identity; a document comparison cannot supply it. MDIO IDs are writable
in the inspected historical PHY revision and provide identification diagnostics,
not adversarial authentication. A C snapshot likewise carries no cryptographic
or language-enforced authority.

## T39.05.b

The actual producer imports the retained read/query implementation. Reset8000
must be written alone; waiting for MAC MB0 only acknowledges the management
transfer. BMCR bit15 must clear before issuing1200, and bit9 must then clear
before the current query is admitted. The teaching model deliberately leaves
these fields pending over multiple completed transactions, so collapsing the
acknowledgements is falsified. IDs0007/C13A match the masked family. BSR20 then24
means past-down/current-up; special1018 selects done100/full. All-ones identity,
busy, frozen-clock poll exhaustion and missing current link are distinct results.

The synthetic successful boot has13 transactions:1 reset write,3 reset reads,
1 restart write,3 restart reads and5 query reads. Each transaction has6 fallible
callbacks under its one-busy-read schedule, giving78 injected failure positions.
The independent oracle derives this rather than asking the implementation for
its count. Output snapshots remain unchanged on all failures. The extra-poll
mutant fails the exact bus-read bound even though both paths eventually time out.
Do not claim these finite injections exhaust all physical faults or callback
nontermination. No state machine can guarantee a time bound if its callbacks
themselves fail to return.

## T39.05.c

Special1008 resolves100/half and is rejected despite advertised full capability.
1014 resolves10/full and is admitted. Readiness is an observation: a latch-low
history, current link loss, mode change or management error retires the old
incarnation. Epoch exhaustion clears readiness permanently rather than wraps.
The EPOCH/LATCH/READY mutants separately falsify initial identity, transient
history and failure revocation. Clearing the owner's public bytes or forging a
snapshot is outside the trusted single-owner API contract; discuss that exact
limit when comparing with a language-level affine resource or capability.

The actual retained EDTx owns copied bytes in phase3. After PHY link loss it
still owns them; an unconfirmed abort moves to phase4, not idle. Only an
independently confirmed stop permits reuse, and a completion carrying the old
token cannot release the new slot. This is a real composition of two original
implemented controls but still a bounded model of device ownership. A normal
descriptor ring and physical stop acknowledgement are separate requirements,
which must not be marked complete by this demonstration.

## Assessment and evidence

Require derivations, unchanged original and delayed-task wording, independent
predictions, a first-failure explanation and repaired mutation regressions.
The C17 native and actual Thumb model matrices, object audit, linker closure
and retained stack calculation support only their recorded execution scopes.
The16KiB harness reserve accommodates synthetic register/Tx model objects;
it does not measure a production driver watermark or WCET. Reject arguments
that passing sanitizer or oracle runs prove all semantics, independence or
physical timing. External review, independent delayed learner performance and
actual board observation remain unperformed.
