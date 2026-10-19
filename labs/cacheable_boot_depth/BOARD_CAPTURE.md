# Required dedicated H745 capture, not yet observed

Use only a dedicated unswapped H745xI development board after a separately
authorized hardware session. Preserve the exact board/silicon revision, power
configuration, applicable errata review, existing BCM4/BOOT addresses, both
ELF hashes and complete flash placement. The runner never writes option bytes
or flashes hardware. An unsupported provisioning state must stop the exercise.

1. Inspect both maps: `.peer` is exactly0x30040000..0x3004001f, NOLOAD and
excluded from both startup BSS/data loops. Record generation and both private
stack extents. Verify no DMA/interrupt/debugger writer or other firmware owns
that line, HSEM0 or an alias. Keep debugger memory auto-refresh off during the
timed ownership interval; a probe read may not observe dirty M7 cache bytes.
2. Capture global cold reset with CPU2 held. Before release inspect MPU
region7, CCR.DC, CSSELR/CCSIDR and actual clock enables. Observe M7 invalidating
every admitted cache line, initializing its dirty peer line and completing the
clean/DSB before RCC_GCR.BOOT_C2. Use hardware trace or a separately instrumented
image; breakpoints and extra stores change timing and must be reported.
3. Capture M4 startup without any clear/write to `.peer`. Read its noncached
MPU map, current generation and code/stack identity. Then capture HSEM acquire,
M7 invalidate, reservation/health transition, M7 clean and HSEM release on a
common external timebase. Require both cb_status values0x600d0000/0x600d0001,
current generation, health3, no outstanding reservations and readiness1.
4. Use the supplied original controls on a separate disposable teaching image:
early CPU2 release and omitted clean must be analyzed for stale RAM exposure.
No deterministic failure is promised for every physical cache state. Report
actual successful/failed observations, cache line residency and barriers; a
passing unbroken run cannot demonstrate necessity.
5. Repeat changed generation and cache geometry only where the exact hardware
supports it. Different geometry is always executable in the original model;
do not manufacture a different physical CCSIDR value. Compare the existing
noncacheable image under the same logical workload and report code size,
ownership latency and memory traffic only with a documented observation method.
6. For a DMA extension retain its transfer reservation after timeout until the
actual provider establishes quiescence. This boot workload initiates no DMA;
its completed CPU handoff must never be relabeled a physical DMA drain test.

Retain raw traces plus synchronized annotations, source/build identities and
every failed run. Cache enable, reset and software-readiness observations do
not certify silicon errata, arbitrary asynchronous faults, durable flash commit
or independent learner mastery. Stop rather than invent a capture when the
board, probe, trace support or applicable source authority is unavailable.
