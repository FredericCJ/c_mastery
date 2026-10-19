# Firmware target obligations and reset cuts

Use the host field/byte-cut model first. Stage a new journal record into erased
storage while retaining the old committed record. Reset after each seq/value/
check/marker byte. The synthetic marker is valid only when all bytes match;
checksums do not authenticate. Run both old/new selection through the C core,
then separately relax the model to arbitrary torn program units and corruption.
Document counterexamples to the synthetic XOR check, including coordinated field
changes, before adding CRC/authenticated image verification.

Dedicated F439/H745 transfer is mandatory but not executed by this author batch.
Inspect the exact part reference manual, errata, boot options, flash/program
granularity, board revision and cache rules. Supply linker maps for both slots;
implement reset/vector validation, stack range, image extent, version/signature
policy, trial attempt persistence, health confirmation and recovery transport.
For H745 define a joint two-core generation and quiescence handshake. Reset one
core during shared-memory publication; account for cache maintenance, DMA and
ownership. Inject watchdog and power-interruption faults only on dedicated
equipment in a separately authorized run; retain actual traces, never substitute
emulator logs. This task provides no flash command or option-byte change.

## Implemented continuation

The required [concrete continuation](EXTENSIONS.md) supplies the additional source,
fixtures, failure controls, objective criteria and bounded author execution.
Its evidence does not replace independent learner or physical obligations.

Required actual cryptographic-adapter and key-policy composition is specified in [SIGNATURE.md](SIGNATURE.md).
