# T27 source and depth reconciliation

All30 original criterion instances and their30 changed-condition tasks were read
from PR-T27-01..05, together with T27 teaching, firmware_io contracts/LAB-FW-02
and the actual selected-startup C/assembly/linker/B06 producer. Existing register
models and cold cross images did not provide the new explicit interval, split
initialization, hooks, generation-cut, ELF rejection and executable startup
variants. They remain valid only for their previously recorded scopes.

The additive package implements numeric interval/image validation; two-bank
copy/zero with no-touch gaps; bounded oscillator/PLL phases and invalidation;
preflight hook dependency graphs; cut-safe retained replacement and8-bit serial
selection; version migration; handoff rejection; original strict M4 images,
nested exceptions, MPU guard, FP ABI comparison, software reset, RAM vectors,
offset bootstrap, and missing/corrupt initialization specimens. Actual vendor
F429 reset code plus its linker pair exposes the documented uninitialized-CCM
gap; the original restricted adapter fixes it without importing a runtime.

The reference is separate from public compiled semantic-failing stubs. Target
verification seeds RAM through the emulator loader independently of the learner
reset code so omitting both the teaching poison and zero loop cannot pass merely
because QEMU initialized RAM to zero. An earlier exploratory guard test checked
MemManage cause bits but ran with PRIMASK set; it could accept an escalated
HardFault. The current target enables that exception and checks IPSR=4, exact
MMFAR and CFSR. A separately compiled masked variant must be rejected. Earlier
pilot receipts are retained; they are not the final current guard claim.

The source register records actual passages, not whole books/manuals. It retains
RM0090 Rev21, PM0214 Rev10 and RM0399 Rev4 identities and distinguishes currently
accessible DS9484 Rev14/UM1974 Rev11/ES0206 Rev24 from earlier historical records.
Two direct ST PDF downloads timed out although primary PDF text was accessible
through the web tool. No local digest is invented for those captures. A guessed
F439 vendor linker URL returned404; the actual F429 pair and separately inspected
F439 reset are explicitly identified. PM0214 p205's inconsistent MPU minimum
prose is retained as a diagnosis case, not silently normalized.

No board identity, physical power/watchdog timing, independent technical review,
learner attempt or seven-day transfer has been manufactured. The physical
capture package, cold/watchdog cross images and acceptance protocol are authored;
physical execution requires a dedicated identified board and instrumentation.
The abstract retained-byte atomicity model cannot establish H745 ECC/cache
persistence. Original criterion, prerequisite, accepted hour and receipt records
remain unchanged. Final current reports and canonical dispositions are produced
only after source freeze and evidence guards, not from this audit prose.

The final loader attempt (attempt06_loader_overlap.json) failed before guest execution: QEMU rejected ROM regions overlapping ELF PT_LOAD zero-fill RAM and independent RAM seeds. The corrected runner keeps the checked ELF for addresses/closure and converts its loadable ROM bytes with objcopy for QEMU `-kernel image.bin`; the separate loader devices then seed RAM without an ELF loader silently zeroing it. This is an author harness correction, not evidence of a guest defect or a passed negative semantic test. The next full campaign must execute all positive and diagnostic paths again.

The next attempt executed GCC-debug guest paths but its report writer stopped at GCC-release stack accounting: GCC named optimized private functions `middle.constprop` and `leaf.constprop`. Raw artifacts remain in build/legacy_advanced_bringup/20261017T141536368915Z. The corrected analysis requires exactly one emitted symbol per named call-chain function and records that binding; missing/ambiguous bindings fail the stack argument. Optimization remains enabled. No complete campaign is inferred from this interrupted run.
