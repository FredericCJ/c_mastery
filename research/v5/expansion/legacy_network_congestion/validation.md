# Congestion author validation

Campaign: 220 checks; passed=True. Exact commands, exit statuses, raw logs, tool/input/output SHA256 identities are in current_validation.json.

Run: `wsl --exec python3 tools/run_network_congestion.py --report reviews/v5/expansion/legacy_network_congestion/current_validation.json`

Six native C17 profiles: GCC/Clang O0/O3 and ASan/UBSan. Each executes8255 independent cases,12 schedules/17 flows,9 compiling semantic mutants and a compiling incomplete stub. Four Cortex-M4 GCC/Clang O0/O3 direct-linked CPU images plus target mutants/stub; each object import is logged, linked imports empty. No external compiler archive is linked.

- arm-none-eabi-gcc-O0 resources: conservative stack 1392 bytes within32768; observed ELF/static frames, not physical high-water measurement.
- arm-none-eabi-gcc-O3 resources: conservative stack 1232 bytes within32768; observed ELF/static frames, not physical high-water measurement.
- clang-O0 resources: conservative stack 1400 bytes within32768; observed ELF/static frames, not physical high-water measurement.
- clang-O3 resources: conservative stack 1232 bytes within32768; observed ELF/static frames, not physical high-water measurement.

Finite-horizon Jain results (100ms):
- Case4: bytes[1408, 512]; exact index225/274.
- Case5: bytes[1408, 512]; exact index225/274.
- Case6: bytes[768, 448]; exact index361/386.
- Case7: bytes[1408, 128]; exact index36/61.
- Case8: bytes[512, 1408]; exact index225/274.

The first recovery-budget implementation reused the resettable duplicate counter. Alternating window updates exposed replenishment; the corrected independent recovery_used counter and compiled mutant9 preserve a regression. Pilot logs were not treated as final campaign acceptance.

Author reference execution; external review and independent/delayed learner evidence absent.
Actual NiParse/SChecksum used by finite original packet model; no full bidirectional TCP or driver claim.
8255 aggregate oracle cases/profile and12 finite schedules are bounded, not universal correctness.
Strict Cortex-M4 core CPU execution is not physical Ethernet or latency observation.
No NewReno/SACK/ECN; conservative1MSS IW and fixed simulator RTO/backoff are explicit model assumptions.
Finite100ms service fairness cannot establish Internet fairness; completed equal workloads trivially give1.
Stack bound uses emitted direct-call frames plus1024 allowance, not measured physical high water.
