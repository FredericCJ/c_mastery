# CMK2 source inspection

Inspected 2026-10-10. Records for registry merge are in `kernel_smp_resources.json`.

- [ACPI6.6 chapter5](https://uefi.org/specs/ACPI/6.6/05_ACPI_Software_Programming_Model.html):   5.2.5.3,5.2.6,5.2.12 5.2.12.2, including MADT format and processor-local APIC tables. These support bounded table parsing and the decision to mask the legacy PIC during APIC operation.
- [Intel SDM3A253668-060US, September2016](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf):  4.10.4.1 (printed4-41 4-42),  4.10.5 (4-45 4-46),  8.4.4.1 (8-20 8-21),  10.5.4 (10-16 10-17),  10.6.1 (10-18 10-20). Actual text was inspected, including the CR3 invalidation conditions, low-memory startup-vector construction, and APIC timer/ICR fields. This supplements the earlier v2 interrupt-frame inspection; it does not relabel the old edition as current.
- [Virtio1.2](https://docs.oasis-open.org/virtio/virtio/v1.2/virtio-v1.2.html):   4.1.5.1.2 4.1.5.1.3 and4.1.5.3 4.1.5.5. Actual passages establish vector assignment/readback and queue completion interrupt handling. The implementation uses one MSI-X table entry and one split queue; general PCI routing and all virtio features are outside the exercise.

The fixed emulator profile is QEMU `pc-i440fx-10.2`, `qemu64`, MTTCG,1/2/4CPUs,128MiB. Passing its device model is execution evidence for that profile. It does not establish hardware WCET, cache timing, production OS completeness or physical-board support.
