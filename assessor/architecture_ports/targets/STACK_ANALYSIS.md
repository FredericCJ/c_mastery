# AP5 assessor analysis

The candidate is `(top-requested)&~7` after rejecting requested>128. The selected
top and low are8-aligned and differ by128, so every admitted request rounds into
the interval. Saving r4-r7/lr consumes20 bytes, then four padding bytes retain
8-alignment on the caller stack. r4 retains that SP while the dedicated stack is
active; the alignment leaf preserves r1 and all nonvolatile registers. The probe
restores r4's SP before its epilogue. It never uses the last128 bytes for return
storage because the leaf's return PC remains in LR.

All0..128 states plus UINT_MAX distinguish arithmetic admission from post-hoc
guard detection. The alignment4 mutant refuses request1 after seeing a bad
call boundary; this semantic failure is meaningful even though no bus fault
occurs. The exclusive-low-bound mutant rejects128. The guard only witnesses
these writes: it cannot prove absence of a write restored before observation.
For AP5-DELAYED a valid solution must derive exception hardware stacking and
alignment from the selected architecture and actual enabled context features;
masking ordinary interrupts does not remove NMI/fault frame obligations. Do not
award the delayed result for copying the author reference.
