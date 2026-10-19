# FW-START assessor analysis

The first irreversible operation is BOOT_C2, not an ordinary memory assignment.
The release model checks contents at the instant of that write. An eventual
correct mailbox cannot repair an already released peer. Readback/barrier after
RCC peripheral enable is distinct from the final DSB before BOOT_C2. Neither a
host trace nor compilation certifies interconnect timing on H745 silicon.

The system starts at HSI64 with divide1 and HSIDIVF0. The latter indicates a
completed change, not general oscillator readiness. Cold admission checks
HSIRDY instead. M4 current boot address0810 denotes08100000; programming a new
option value is not a same-session branch instruction. SYSCFG UR0 BKS1 means
original bank mapping; BCM4 must be0. BOOT pin0 remains an external provisioning
precondition because reading the current flash options does not prove the
sampled boot pin. Neither code attempts a live supply/PLL reconfiguration.

The selected output is a trusted ordinary C value, not an unforgeable capability.
st_release assumes successful cold admission and cryptographic selection. The
caller cannot pass attacker-created BSSelection and then claim this layer
revalidates the signature. M4 independently verifies actual storage so stale
mailbox data cannot substitute for authentication. The exact peer generation,
sequence, vector and complement must all match. Shared storage is uncached and
owned; this hardware protocol is not an ISO C threads synchronization proof.

A fail-stop after peer release retains old committed metadata but does not
revoke execution already started on the peer. Another trial requires system
reset and controlled provisioning. Boot self-check health is narrower than
application readiness; committing a new update on that signal would be an
assessment failure. The selected images consume existing committed records;
no new update is accepted by this boot-only wrapper.

Mutation1 bypasses boot/bank/BCM4 validation; mutation2 releases before shared
publication. The public model rejects each semantically. The learner stub must
compile and fail ST_CHECK, not merely fail to link. The F439 image is only a
startup diagnostic and cannot close a two-core or recovery-update criterion.
For the delayed warm-cache variant, require an owned-region map, dirty-line
policy, barriers and DMA/peer quiescence, not a blanket invalidate instruction.
