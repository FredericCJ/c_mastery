# Diagnostic author validation: T28.12

The original three ordinary criteria and three delayed criteria remain byte-for-byte
unchanged. The reference package implements the exact bounded datagram, real local
UDP adapter, existing FCursor/ED compositions, stream framing, USB packet/request
model and dedicated Linux gadget/usbfs experiment. A finite pass is not absolute
mastery, physical USB/Ethernet conformance, external review or a learner attempt.

Executed in WSL/Linux on the recorded x86-64 host, GCC15.2, Clang21.1.8,
Arm GCC14.2, QEMU10.2.1 and source-pinned selected Linux7.0.0-38 guest:

| Exact invocation | Observed result and scope |
| --- | --- |
| `python3 tools/run_diagnostic_depth.py --skip-guests --report reviews/v5/expansion/legacy_advanced_diagnostic/attempt01_validation.json` | 783 passed focused development checks; superseded for current closure by the full campaign. |
| `python3 tools/run_diagnostic_depth.py --report reviews/v5/expansion/legacy_advanced_diagnostic/current_validation.json` | Exit0; 794 checks. Six native compiler/optimization/sanitizer profiles; compiled semantic mutants and stubs; all65536 sequences independently checked using Python struct, all payload lengths and all stream splits. |
| Same full campaign, target cells | 24 big-endian PPC64 ELFv2 images and24 Cortex-M4 images, each executed with declared expected pass/rejection. Direct linker/object imports/final closure, maps and disassembly recorded. Original helper sources are explicit. No target libc, libgcc, compiler-rt, libatomic or HAL linked. |
| Same full campaign, Linux guest cells | Four disposable, diskless, networkless two-CPU QEMU runs: positive actual USB traffic/cancellation; incorrect endpoint-size mutant; replay-admission mutant; unfinished gadget binding refusal. Each has15 build/tool/ABI/import/guest commands and before/after source identities. No physical USB or host module loading. |
| `python3 tools/check_diagnostic_depth.py --self-test --output reviews/v5/expansion/legacy_advanced_diagnostic/support_validation.json` | Exit0;19 rejected evidence mutations, including missing provider, uncompiled mutant, missing guest/target/public workflow, stale/added input and forbidden runtime. |
| `python3 tools/summarize_diagnostic_costs.py` | Exit0;8 positive target image profiles. Linked function bytes and compiler local frames; no whole-stack/WCET inference. |
| `python3 tools/check_diagnostic_exports.py` | Exit0;9 real CLI/exported-builder commands. Untouched core/USB/socket scaffolds reject; explicitly supplied reference implementations pass. Both exported guest variants compile; runtime refusal/traffic is checked separately. |
| `python3 tools/check_diagnostic_package.py --self-test --output reviews/v5/expansion/legacy_advanced_diagnostic/completion_dispositions_validation.json` | Exit0;11 task/delay/evidence-dimension/mapping controls rejected. |
| `python3 research/v5/expansion/legacy_advanced_diagnostic/verify_source_package.py` | Exit0; apt checksum, extracted source archive and5 inspected Linux files agree with the selected distribution source package. This is not an independent binary reproduction. |

Every command/exit status/raw log/hash is in the linked JSON reports. The full
campaign's four copied guest reports also retain absolute external tool/source/
image/module paths. Its seven evidence-classifier claims and ten rejection cases
are synthetic metadata tests; they cannot authenticate a physical trace.

Earlier original-guest failures remain under `/home/frede/cmastery-diagnostic-tools/`
(`guest-pilot*`, `module-pilot`): missing reset callback, missing module dependency,
autoconfiguration packet-size mismatch and initial test assumptions were corrected,
not reclassified as passing. Kernel faults occurred only inside the original guest.

`source_access.json` retains the unresolved zero-address specification conflict,
inaccessible current USB ECN package and exact board-routing dependency. The T39
MAC/PHY target route and T38 CAN mechanism remain required work, alongside actual
board observations, independent learner/delayed evidence and external review.
