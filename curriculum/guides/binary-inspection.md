# Inspect the binary you built

Start in a checkout or an extracted learner archive with Python 3, a C17 compiler, GNU binutils and hexdump. Each exercise opens a separate workspace. For example:

```sh
python3 -m cmastery init PR-T07-01 /tmp/c17-stages --exercise X-T07-001
cd /tmp/c17-stages
python3 inspect_lab.py prepare --cc gcc
```

Read `ACTIVE.md` and the workspace `CONTRACTS.md`. Write your prediction before opening the tools, then save your observations and explanation under `answers/`. The check binds numerical observations to the artifacts you built. Use a separate scratch copy for each changed condition.

| Exercise | Project | What you establish |
|---|---|---|
| X-T07-001 | PR-T07-01 | Follow a macro, an external call and its definition through preprocessing, compilation, assembly and linking. |
| X-T08-001 | PR-T08-01 | Translate a symbol address to a file location. Confirm instruction bytes independently with readelf, objdump and hexdump. |
| X-T09-001 | PR-T09-02 | Resolve a cross-object relocation, measure a PIE load bias in GDB and account for zero-filled memory. |
| X-T44-002 | PR-T44-04 | Compare GDB stops under two optimizations. Recover and annotate the same computation in a stripped ELF using Ghidra CodeBrowser. |
| X-T27-001 | PR-T27-02 | Follow the Cortex-M4 reset vector and freestanding startup through a live GDB session on QEMU MPS2 AN386. |
| X-T43-002 | PR-T43-09 | Replay raw PMU pairs offline and reason through a wrapped sample and tail publication. |
| X-T43-003 | PR-T43-09 | Bind a fresh native observation of your own implementation to the exact measured executable, CPU and event domain. |

The hosted exercises target Linux x86-64 with the System V AMD64 ABI. Cortex-M4 requires arm-none-eabi-gcc/binutils, qemu-system-arm and gdb-multiarch. Use the repository's `bash tools/install_inspection_tools.sh` for the pinned Linux x86-64 Ghidra and JDK, then follow the CodeBrowser steps in your workspace. X-T43-002 ships a small offline dossier. X-T43-003 requires a host that permits per-thread instruction and cycle events, and records a denied or unavailable event explicitly.

Each card carries its own rubric and a delayed transfer. Build output, completed checks and worked author measurements each establish different facts. Independent assessment records the learner's own explanation and transfer.
