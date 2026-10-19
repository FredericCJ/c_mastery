# Required physical comparison record

This is an unfilled collection protocol, not physical evidence. Use only the dedicated authorized STM32F439 target.

Before deployment record board serial/revision, MCU marking, probe/firmware, supply, clock tree, Flash wait states, caches, memory placement, interrupt mask/load, startup object and linker-script hashes. Bind exact ELF/map/disassembly and readback image hashes to each plain/LTO build. Record stack watermark method and interrupt/nesting margin. Keep the profile-E image free of libgcov, filesystem and hosted allocation.

Run the same modulo-32 correctness vectors on each image. Capture cycle counter wrap handling and synchronized trigger timestamps for at least 30 paired runs per workload stratum. Record raw counts, clock units, min/median/max and measurement overhead. A sample maximum is not a WCET proof. Preserve failures and temperature/supply conditions. For energy use calibrated voltage/current sampling with shunt value, bandwidth, sample rate, integration interval and baseline subtraction; cycles do not imply joules.

Acceptance requires semantic equality AND independent Flash<=1300, stack<=128, cycle<=180 synthetic exercise gates, replaced by explicit actual application budgets before real acceptance. A candidate with lower measured energy but missed latency remains rejected until the requirement owner changes the latency bound. A board revision, clock, linker placement, compiler, interrupt load or workload change invalidates corresponding timing/resource claims and requires a new capture. QEMU, host and synthetic rows never fill physical columns.

Record fields: image ID; source/compiler/flags; board ID/revision; map; code bytes; stack estimate; observed stack; clock; interrupt state; workload; raw cycles; overhead; uncertainty; optional raw current/voltage; calibrated energy; acceptance decision; operator/date. All physical fields are currently **not observed**.
