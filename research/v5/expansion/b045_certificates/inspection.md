# B04/B05 certificate expansion inspection

Actual source reads establish the three mismatches recorded in inspection.json. The APIs and original finite models are defined in CONTRACTS.md. The tree checker uses integer indices, a global order interval and visited mask; the byte matcher enumerates overlapping windows; the clock snapshots before commit. These are original bounded constructions, not attributed reproductions of an uninspected book. Exact source bytes are pinned separately.

The broader leaf audit remains required. AVL rotations/deletion implementation and assembly hash lookup are retained prerequisites, not supplied by these narrow new APIs. New match count-on-failure semantics are deliberately different from the parent required-count interface and require a documented adapter. Target linking is separate from execution.
