# C Programming Mastery — second edition

Build small C components, explain their behavior and reuse them in Linux systems,
Cortex-M firmware, communication stacks and a small operating system.

Start with {ref}`guide-second-edition` and {ref}`guide-entry`, then follow the
{ref}`guide-project-route`. The active curriculum contains **{{COUNTS}}** and
**{{PROJECTS}} projects**. Previous objective/project identities have explicit
migration records. Domain numbers organize coverage; dependencies determine order.

The route brings diagnosis and memory experiments forward, develops algorithms
from invariants through x86-64 and Thumb assembly, then connects measured components
to complete systems. T46–T48 add a bytecode runtime, persistent store and kernel;
the final guest recovers a VM-written value after reboot.

| Study | Build | Check |
|---|---|---|
| {ref}`guide-reading` | {ref}`guide-component-contracts` | {ref}`guide-assessment` |
| {ref}`guide-standards` | {ref}`guide-profiles` | [Objective map](coverage/index.md) |
| {ref}`guide-workload` | {ref}`guide-communication-stacks` | {ref}`guide-physical-evidence` |

Use {ref}`guide-performance-debugging` for cost models, profiling, causal debugging
and memory-failure experiments. Each project provides contracts, bounded readings,
inputs, hints, failures and evidence requirements.

The portable baseline is C17 with parallel C23 investigations. The targets are
Linux x86-64 and Cortex-M4 Thumb. QEMU supplies a distinct emulated route.
Full completion requires physical observations for every named interface.

See {ref}`guide-laboratory` for the runner and
[research and source selection](research/index.md) for bounded inspection records.
Automated results establish their recorded scope; they do not certify learner
mastery or unperformed hardware experiments.
