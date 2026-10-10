# Firmware I/O implementation units

Twelve units connect the reset/exception material to owned peripheral services.
Start with the relevant card in `units/`, predict the register trace, and
implement only the selected function in `src/firmware_io.c`.

```sh
cmake -S labs/firmware_io -B build/firmware-practice
cmake --build build/firmware-practice
ctest --test-dir build/firmware-practice -R firmware_io.uart --output-on-failure
```

The initial scaffold compiles and fails every implementation gate. Explicit
`-DCMASTERY_ASSESSOR_ROOT=/absolute/path/to/assessor/firmware_io` selects private
reference code and adds deliberate mutation controls. Keep that material
concealed while recording an independent attempt.

The register adapter is an executable hardware model with readable operation
traces. It deliberately separates the driver's state machine from the board's
actual memory map. When `arm-none-eabi-gcc` is installed,
`firmware_io.cortex_compile` also compiles the selected implementation for
Cortex-M4 and Cortex-M7 and rejects external runtime symbols.

The parent projects still require inspected F439/H745 register maps, startup
and interrupt code, linker placement, clock and pin configuration, cache/DMA
maintenance where applicable, and physical traces. These tests establish no
claim about measured baud accuracy, electrical bus recovery, actual DMA
quiescence, Ethernet operation, or watchdog timing on a board.

Before promoting a component, preserve the adapter's source snapshot,
execution assumptions and trace contract. Reuse its implementation in the
integration family; board adaptations must rerun contract tests and add their
own target evidence.
