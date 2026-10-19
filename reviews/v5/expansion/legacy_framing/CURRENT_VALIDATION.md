# T24.05/.06 current author evidence

The exact six ordinary criteria and six changed-condition reference variants are implemented in `labs/hosted_systems/stream_depth/` and separate `assessor/hosted_systems/stream_depth/` material. Original independent seven-day assessment remains unattempted.

Executed in WSL Ubuntu with GCC15.2 and Clang21.1.8, Linux local AF_UNIX sockets, hosted C17: `python3 tools/run_streams_followthrough.py --report reviews/v5/expansion/legacy_framing/current_validation.json` exited0 with104 commands and12 criterion observations. Each compiler ran debug, O2/NDEBUG and ASan/UBSan profiles. The eight compiled semantic mutants and three compiled learner scaffold controls exited1 at intended assertions. All2,611,210 chunk partitions,65,536 two-byte lengths,2,313 global-budget cells and175 edges from25 independent sequence states passed in each profile. Hosted nm/size inspected actual objects; no strict-freestanding closure is claimed.

`tools/check_streams_followthrough.py --report reviews/v5/expansion/legacy_framing/current_validation.json` exited0;21 deliberate report violations were rejected. `tools/close_streams_followthrough.py --report reviews/v5/expansion/legacy_framing/current_validation.json` preserved all six original criterion bodies and delayed tasks and checked pure attachment idempotence. `tools/check_streams_exports.py` exited0 with one actual PR-T24-03 export, public file identity and assessor exclusion verified.

The earlier102-command pilot is retained in `pilot_validation.json`. A later added regression verifies a replay returns its original result after intervening effects; the new current-result mutant is separately rejected. Raw current logs, commands, source identities and38 retained build artifacts are recorded by the report. The actual positive-short prefill syscall count was zero; deterministic short-return callback evidence remains distinct from actual EAGAIN and one-byte syscall progress.

During initial native development an ancillary test incorrectly required all recvmsg output flags to be zero. It observed0x40000000 when MSG_CMSG_CLOEXEC was requested; the inspected man-pages6.19 recvmsg passage explicitly documents that output flag. The test now checks the relevant truncation bits and descriptor properties. An optional strace diagnostic was unavailable; direct test output and primary documentation resolved the issue. These early interactive failures have no fabricated campaign receipt.

External review, independent learner/delayed attempts and physical observations remain absent. Local sockets do not establish Internet delivery, durable exactly-once effects or other-OS behavior. No current C17/C23 published standard inspection is inferred.

Current report SHA256: 4004d88a2935629d653d5d1b65de813c31e7853d33d8599c788b2d56ac116c9b
