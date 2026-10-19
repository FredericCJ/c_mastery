# T53.02 assessor argument

The state model treats generation as an identity, timeout as loss of timely
observation, and quiescence as a separate port premise. A stale ACK cannot
advance a new attempt; a duplicate cannot count twice; UINT32_MAX cannot wrap.
Observe that generation exhaustion does not prevent release of an already
quiescent attempt. Serial/parallel admission must reject every stack/page cross
overlap, not merely duplicate CPU IDs. The plain C controller is externally
serialized; no atomic progress property follows from its fixed arrays.

The actual target makes the port premise concrete. AP C verifies mode, CR3,
identity and entry stack, installs its real local tables, and publishes fields
before ACK. ACK still does not authorize freeing the active C bootstrap frame.
`ss_park` switches RSP to the permanent per-CPU stack and calls `ss_parked`, which
publishes quiescence there and never returns. All return addresses leading to
the bootstrap frame are abandoned. The permanent stacks/tables stay live for
the entire VM. Therefore serial code/mailbox reuse follows an actual last-access
handoff, not a busy-bit poll or a claim that timeout stopped an AP.

The delayed target waits for the BSP's timeout gate before completing. The BSP
must first demonstrate both release refusal and new-admission refusal, then
allow completion and record LATE. Missing-ACK and previous-generation target
controls exit through precise semantic markers; an outer timeout or exception
does not count as mutation detection. Sparse QEMU APIC IDs0,1,2,4 reject the
assumption index==identity. One CPU is the empty AP boundary only.

Each `.rodata.ss_ap` image is assembled for its actual low page; relocation
checks and16/32/64 disassembly are separate. PAE precedes CR3 in the new adapter,
then EFER, paging and the long-code far jump. NX is required because the selected
transition enables NXE; QMP preflight refuses a model without it before image
execution. The raw host preflight is not a proof of all CPUID/firmware/errata
properties. The retained PIT delays and spin counts are emulator observations,
not physical INIT/SIPI timing compliance.

The selected original policy sends one SIPI after INIT assert/deassert and
contains failure instead of retrying; compare this explicitly with the inspected
Intel typical two-SIPI procedure. Do not imply execution of a second-SIPI path
that is not present. Actual parallel launch waits on each private acknowledgement
and quiescence record; the order of collecting them is not the AP execution order.

Assess original leaves/variants using the exact level2/3 distinctions in the
public contract. The independent15,544-word oracle omits arbitrary histories and
cannot prove a truthful hardware attestation. QEMU completion does not establish
physical timing, current vendor errata or independent learner mastery.
