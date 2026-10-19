# Private T21.10 analysis

10.a: checksum16 accepts the original four-byte payload;17 rejects. The independent
slot oracle uses cut position, not the writer's slot-selection algorithm: before
commit retain generationhistory, after commit selecthistory+1. There are33
history/cut cases. A corrupt newest slot falls back to generation2; duplicate
valid generation conflicts. The checksum mutant must compile and fail.

10.b: require two actual differing build notes, separate debug artifacts, matching
admission and stale4/5 rejection before symbolization. A plausible symbol from
the wrong binary is not success. Compare runtime-minus-bias against nm, then
addr2line against the selected debug file. Copying the binary to another directory
is actual relocation of the executable path; address-layout variation is observed,
not guaranteed on every run. The captured function entry is not falsely labeled
a hardware fault PC.

10.c: decode and recovery return separate statuses. Valid state3 must preserve
recovery state and produce no effects. Repeated cut4 cleans/reconstructs local
resources, but the retained journal ID deduplicates the effect. A valid solution
must state the journal persistence/retention assumptions; resetting all state and
calling the effect again is not idempotent. The resource ledger independently
checks the final live set. No source-bound model result proves an actual power-loss
protocol or board restart safety.

The delayed packet assesses the implemented two-slot cut, relocated PIE and
repeated-reset mechanisms under the original criterion identities. Require a
fresh derivation without the private answer and retain the original minimum seven
days. Independent learner performance and physical evidence remain pending.
