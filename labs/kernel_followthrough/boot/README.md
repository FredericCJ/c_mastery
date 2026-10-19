# From a bootloader entry to a C call

This successor addresses PR-T48-01 only. It builds `boot.S`, the assembly entry
recorder, and one C17 probe. It does not need the allocator, IDT, scheduler,
archive, virtio driver, VM or storage engine. The original learner files remain
incomplete. Select the assessor directory explicitly only after recording your
own prediction and first failure.

At the i386 Multiboot2 boundary EAX carries the magic and EBX the physical boot
information address. GRUB supplies flat segments and protected mode with paging
off; it does not supply a C stack or a usable GDT for reloading segments.
`cli` retains the masked-interrupt premise and `cld` establishes the C direction
flag convention. Preserve both arguments before reusing their registers. The
saved information pointer must still denote reserved boot data when a later
allocator begins; this boot-only probe never allocates.

Follow the actual assembler in this order:

1. Save EAX/EBX and establish writable stack storage. The saved argument words
   are data, not a return address or part of the stack-alignment calculation.
2. Point PML4[0] at the PDPT. Point its first four entries at four page directories.
   Their 2048 entries describe consecutive 2-MiB supervisor mappings. The last
   maps `[0xffe00000,0x100000000)`. Zeroed BSS supplies all unused entries and
   high halves. Retain the ELF load/zero-fill premise in the dossier.
3. Set CR4.PAE, load CR3 with the aligned physical PML4, enable EFER.LME and NXE,
   load the owned GDT, then set CR0.PG and WP while preserving PE. The next
   instruction must remain mapped at its current address.
4. Transfer far to selector8. Enabling long-mode paging does not itself select
   the 64-bit code descriptor. The far jump makes CS.L=1 and CS.D=0; a near jump
   leaves the previous compatibility-mode descriptor cached.
5. Load data selectors only after the owned GDT is installed. Clear FS/GS,
   install the 64-bit stack, place retained arguments in EDI/ESI, and call C.
   Immediately before `call`, RSP must be a multiple of16; its pushed return
   address makes entry RSP congruent to8. The recorder captures that value
   before any compiler prologue changes it.

Missing prerequisites have different first invalid boundaries. Without PAE,
the CR0 transition is inconsistent. An unmapped instruction or stack fails on
its next access. A stale GDT invalidates the far transfer or selector reload.
Omitting EBX preservation can reach C with a false argument. Omitting the far
transfer leaves the subsequent bytes decoded under the wrong CS attributes.
Misalignment violates the calling convention even when a particular scalar
build happens to print a marker.

The historical assembler places two four-byte saved arguments after its final
alignment and before a16-KiB stack. This leaves its stack top eight bytes off the
required pre-call alignment. The successor adds an alignment directive after
the arguments and masks the checksum expression to its32-bit encoding for
Clang’s integrated assembler. Historical sources and receipts retain their original identity.

Run the campaign in a fresh owned directory:

```sh
python3 tools/run_kernel_boot_followthrough.py \
  --work reviews/v5/expansion/legacy_kernel_followthrough/my_boot_attempt
```

The campaign builds GCC and Clang at O0/O2, retains ELF headers, section offsets,
link maps, symbol tables, disassembly and undefined-symbol output, and boots each
image under the pinned QEMU profile. It checks the Multiboot header in the first
32768 file bytes, eight-byte alignment, length24 and checksum modulo2^32.
QEMU's debug-exit port uses a32-bit write. A serial pass followed by a timeout
does not pass the execution gate. GDB captures the preserved entry arguments
and CS attributes across the far-transfer boundary; its intentional kill can
report a remote disconnect, which is checked separately from the state record.

Changed conditions are concrete. Relink at6MiB and recompute every absolute
symbol and reserved physical interval: page tables, GDT, stack and the C entry
all move, while the identity-map equation remains physical=virtual below4GiB.
Do not place any 32-bit boot reference beyond4GiB. Move read-only data ahead of
the Multiboot section and retain the discovery/alignment/checksum checks.
Finally remove the post-argument alignment and explain the resulting captured
entry RSP. These executed author examples prepare the original delayed task;
they do not constitute the required seven-day independent learner observation.

Primary scopes inspected: [Multiboot2 sections3.1,3.3,3.6](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html),
[Intel SDM3A253668-060US sections9.8.5 and6.14](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf),
and [AMD64 psABI draft0.99.6 sections3.2.1–3.2.3](https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf).
The latter two are historical pinned editions; this is not a current-errata,
vector/FPU, physical-machine or general boot-protocol conformance claim.
