# T58 guest assessor analysis

The voluntary context stores the original return address and original stack;
continuing through the restore path must not accidentally validate the alternate
stack. Register sentinels make omitted saves observable. Integer-only preservation
does not meet an FP-enabled ABI promise. Inspect every high register in the
assembly and the post-restore SP, not only the final return value.

The two-root mapping experiment defeats a walk that always reads the first root.
Both roots preserve identity mappings needed to execute the observer. Permissions
are tested by actual accesses, not an extracted bit. RV store/load page faults
are causes15/13. AArch64 EL1 data abort has EC0x25 and selected permission versus
translation syndrome bits. The expected FAR/stval and instruction PC make an
unrelated fault fail. ERET/SRET restores the saved status under the exact selected
privilege contract; a service trap and data fault need different PC reasoning.

The checked negative controls alter a preserved register or root-B mapping. They
demonstrate sensitivity to these mechanisms, not comprehensive ISA coverage.
Withheld GPA3 variation makes a superpage misaligned or changes the granule; the
learner must derive the walk and fault level again. GPA2 variation changes
service instruction length/return convention. Reject fixed '+4 everywhere'
reasoning. Hardware timing, multi-hart invalidation and FP context remain separate.
