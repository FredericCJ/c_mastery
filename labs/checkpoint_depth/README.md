# Checkpoint composition laboratory

The twelve original PR-P1-01 through PR-P6-02 assessment contracts remain the
source of required reasoning, rubrics and delayed variants. This additive family
supplies concrete reference boundaries and unsolved learner sources:

- P1: byte arena, typed handle pool, FIFO, rollback and borrow quiescence.
- P2: partial-write adapters, causal errors, pool-to-writer ownership and custom Linux entry.
- P3: exact versioned CRC wire, bounded recovery, backpressure and independent oracle.
- P4: SPSC acquisition, P3 framing, timestamps, DMA/TC lifetime and original target startup.
- P5: epoll generations, copied workers, shutdown predicates and asynchronous quiescence.
- P6: exact/floating FIR, transaction policies, ISA dispatch and measured equivalent kernels.

Implement only the selected public contract in `src/`; the initial sources are
intentionally unsolved. CMake builds them and semantic assertions reject them.
An explicit CMASTERY_ASSESSOR_ROOT selects separate reference implementations.
No assessor implementation is copied into a learner workspace. All executable
inputs are original synthetic local data. Python oracles and measurement tools
are host verification dependencies, never freestanding target dependencies.

Start by predicting ownership and the exact supplied trace, then state the
invariant at every commit/refusal. Next implement one transition and check the
small boundary cases. Only then run fault schedules, changed assumptions and
the appropriate sanitizer/target campaign. The separate assessor analysis
explains expected outcomes; do not read it before a recorded independent attempt.

`tools/run_checkpoint_depth.py --report reviews/v5/expansion/legacy_completion/checkpoint_depth_campaign.json`
runs six GCC/Clang profiles plus both thread-sanitized profiles. Independent
producers `run_checkpoint_entry.py`, `run_checkpoint_numeric.py` and
`run_checkpoint_firmware.py` record actual target/measurement scope. Their
reports never substitute for delayed learner assessment or physical board work.

`transfer/CONTRACTS.md` elaborates the original named delayed variants with
allocated storage, modern component adapters, version evolution, authenticated
framing, durable receipts and half-duplex turnaround. Public transfer stubs remain
unsolved. `tools/run_checkpoint_transfer.py` selects private author references;
it never installs those references into learner workspaces. Its six profiles
include separate source-bound imports of ownership, errors, receipts and storage.
`run_checkpoint_dependencies.py` audits LTO, stack protection and callback effects;
`verify_checkpoint_timing.py` computes the declared synthetic response/backlog model.
The learner build instructions and precise allowed dependencies are in the
transfer contract. Author source/experiment readiness is recorded separately from
learner completion in the criterion support register.

For optional tools absent from PATH, extract Ubuntu packages into ignored
`build/checkpoint-tools` using `apt-get download` and `dpkg-deb -x`; no global
installation is needed. The entry producer accepts explicit `--strace` and
`--strace-library-dir`; the numeric producer accepts an explicit `--qemu` user
emulator. Record each actual tool version/hash in its report. Firmware execution
uses the dedicated QEMU MPS2-AN386 guest only; it never flashes F439 hardware.

The authenticated transfer uses system OpenSSL3 runtime plus development headers.
When headers are absent, extract `libssl-dev` locally using the same package
procedure, then pass `--openssl-include-root build/checkpoint-tools/openssl-dev/usr/include`
(an absolute Linux path when launching through WSL). No host security setting is
changed. The actual primitive version and header hashes are recorded in its report.
