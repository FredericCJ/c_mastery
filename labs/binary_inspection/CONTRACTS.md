# Inspect the program you built

Work from the exported workspace. `inspect_lab.py prepare --cc gcc` (invoke with python3) prints the directory holding driver/record intermediates, the debug PIE `records`, `optimized`, `stripped`, and a manifest of commands, inputs and hashes. Prepare again with Clang for a second build. Edit a scratch copy for a changed-condition experiment. Preserve your first manifest and transcript before rebuilding.

Create `answers/` and one JSON object per exercise as named in its brief. Write a prediction before looking at the output. Include the commands, observations, units, layer responsible and limitations in an `explanation` string. Encode integers in decimal, hashes and raw bytes in lowercase hexadecimal, paths relative to your workspace. `python3 inspect_lab.py check STAGE --cc gcc` compares the observations to the selected current build. A reviewer evaluates predictions, explanations and transfer separately.

| File in answers/ | Additional fields and units |
|---|---|
| stages.json | object_sha256 of driver.o, output as one line of text, unresolved_call as a symbol, missing_provider_stage as a stage name |
| bytes.json | elf_sha256 of records, function_va and file_offset as integers, first_8_hex as 16 hexadecimal digits in file order |
| linking.json | elf_sha256 of records, relocation_offset in the input section, relocation_type as printed by readelf, bss_type, rw_file_size and rw_memory_size in bytes |
| debugger.json | debug_args_and_return and optimized_args_and_return as [first unsigned argument, second unsigned argument, returned unsigned value], annotation as a path to the exported Ghidra JSON |
| cortex.json | elf_sha256 of target.elf, initial_sp, reset_vector (including Thumb bit), post_startup as [initial_size, record_calls, observed] at result_ready |
| replay.json | report_sha256, pairs as an integer and interpretation as the recomputed gate label. Supply wrapped_sample with physical_header_offset, next_tail and kernel_loss_without_count |
| live.json | report and measured_image as local paths, measured_function as a symbol in that image, report_sha256, pairs and interpretation from the rechecked report |

## CodeBrowser

Use the pinned Ghidra distributed by the repository's `tools/install_inspection_tools.sh` or a compatible local Ghidra with a supported JDK. The installer defaults to `build/inspection-tools` on Linux x86-64. Set `JAVA_HOME` to its `jdk25` directory, prepend `$JAVA_HOME/bin` to PATH and run `ghidra_12.2_PUBLIC/ghidraRun` from the installation directory. Create a local non-shared project, import the stripped ELF as ELF/x86-64, and open it in CodeBrowser. Follow the computation and its callers using the listing, references and decompiler. Rename the recovered function, open its function plate comment, and record your reasoning. In Script Manager add the workspace's `ghidra` directory as a script directory. With the cursor within the recovered function run `ExportInspection.java` and save the JSON inside your answers directory. Keep your Ghidra project locally. The export records a file hash, relative address, name, function comment and actual decompiled text.

## Cortex-M4

Prepare the image with `python3 inspect_lab.py prepare --target cortex`. The target compiles with no hosted runtime. In the generated directory run `qemu-system-arm -M mps2-an386 -cpu cortex-m4 -display none -serial none -monitor none -S -gdb tcp:127.0.0.1:1234 -kernel target.elf`. In a second terminal open `gdb-multiarch target.elf`, run `target remote 127.0.0.1:1234`, inspect SP/PC and the vector bytes, and use an instruction breakpoint at result_ready. Close the emulator when finished. This exercise observes the AN386 model. The F439 curriculum has separate, board-specific physical receipts.

## PMU observations

The two measurement workspaces include `labs/performance_followthrough/native.py` and the offline dossier. From the workspace root run `python3 labs/performance_followthrough/native.py report labs/performance_followthrough/dossiers/replay/layout.json`. Preserve the original bytes and change only a scratch copy. For the live exercise, run `native.py doctor`, then follow the supplied native workflow and `build --help` for the source paths required for your completed PMU and workload. Check your own implementation with `protocol` first. Retain its build manifest, measured ELF and original report. Only fresh LEARNER-provenance runs qualify. A denied event or absent PMU records an unavailable native observation. Offline replay can be completed on that host.
