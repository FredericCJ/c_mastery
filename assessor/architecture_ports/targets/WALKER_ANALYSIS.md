# AP6 assessor analysis

For level i in2,1,0, index uses VA bits12+9i..20+9i. A leaf's lower9i PPN bits
must be zero, and the successful address combines its aligned physical base
with the VA's lower12+9i bits. This independently explains all three page sizes.
Canonical high bits equal sign bit38; checking only upper bits zero wrongly
rejects the high canonical region, while dropping them aliases holes.

The implementation descends at most twice and rejects a third non-leaf, so even
cycles cannot hang. Missing table access is distinct from invalid encoding.
Permission cannot rescue malformed W-without-R or reserved bits. Supervisor
SUM permits selected data access to U pages, never supervisor execution from
those pages. A/D handling deliberately faults instead of updating storage.
The public truth-table oracle and base-plus-offset checks do not call the model.

Actual RV64 guest tables use PPN-shift10 descriptors; AArch64 descriptors combine
aligned addresses with types/attributes instead. The selected AArch64 code uses
TCR_EL1=0x803519 (T0SZ25,4KiB lower-region regime), MAIR_EL1=0x4ff and root/middle/
leaf entries, with TTBR0 replacement and barriers/TLBI. This is code observation,
not full-register-specification review. Preserve the full Arm manual gap.

AP6-DELAYED must revisit the entire PTE snapshot before publishing a repaired
A/D state: setting bits through a stale descriptor can modify a reused mapping.
A serialized model can prove its own revalidation rule; it cannot establish
multi-hart hardware atomicity, remote shootdown completion or C atomic adequacy
for a platform's page-table update requirements.
