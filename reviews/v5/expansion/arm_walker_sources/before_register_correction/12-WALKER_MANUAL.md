# Selected Arm walker comparison: DDI0487H.a

This additive reading companion resolves the inaccessible Arm manual passages
identified by `G-B06-T58-07-TRANSFER`. Read it with the original
[`WALKER.md`](../architecture_ports/targets/WALKER.md), whose bounded Sv39 model,
learner tasks and execution evidence remain unchanged. The complete Arm manual
was obtained; **only the listed passages were inspected**. This is not a claim
to have reviewed all 11,530 pages or qualified a physical processor.

The primary document is *Arm Architecture Reference Manual for A-profile
architecture*, ARM DDI0487H.a, ID020222, issued 4 February 2022. Its title and
release history are PDF pages 1–3. The private capture, page identities and
inspection findings are recorded in
`research/v5/expansion/arm_walker_sources/inspection.json`. Page numbers below
are both PDF page numbers and the numeric part of the printed page label.
The document describes many optional features; its presence does not establish
that the selected emulator implements them.

## Derive the selected address path

D5.2.7, tables D5-19 and D5-20 (4815–4816), and D5.2.8, table D5-24 (4823),
give the four-kilobyte geometry. With `T0SZ=25`, the lower address region has
39 input bits. The initial level is **Arm level 1**. A full initial table has
512 eight-byte entries and requires 4096-byte alignment. The indices are
VA[38:30], VA[29:21], and VA[20:12]; VA[11:0] supplies the byte offset.
Consequently VA `0x80000000` selects indices **2, 0, 0**. The original guest
sets root[2] to a middle table and middle[0] to a leaf table. The final page
descriptor selects the page holding 17 or 29.

Do not transfer the level names mechanically. The retained Sv39 walker calls
these levels 2, 1 and 0. The selected Arm regime calls them 1, 2 and 3.
Arm level-1 and level-2 block descriptors map 1 GiB and 2 MiB; its level-3
page maps 4 KiB. Sv39 distinguishes a leaf using R/W/X and checks its PPN
alignment. Arm uses the descriptor type bits and level-specific address
fields. D5.3.1–D5.3.2 (4837–4841) give these distinctions, including the
reserved level-3 `01` encoding. A table descriptor is not a leaf permission
record. The selected Arm tables have no extra hierarchical restrictions;
arbitrary tables would also require the hierarchical permission rules.

The Sv39 canonical-address check sign-extends bit 38. The Arm guest selects
the **lower** 39-bit region using TTBR0; it does not implement an equivalent
upper-region test by reusing that sign-extension predicate. A changed task
must derive both region selection and the configured size from TCR rather
than copying the Sv39 test.

## Decode the actual controls

D13.2.131 (5946, 5953–5958), D13.2.97 (5791–5793), and D13.2.144
(6014–6015) explain the actual constants in the retained `aa64.S` guest:

| Control | Selected value | Meaning and boundary |
|---|---|---|
| TCR.T0SZ | 25 | Lower region is 2^39 bytes; initial level 1. |
| TCR.TG0 | `00` | 4 KiB granule, assuming the selected implementation supports it. |
| TCR.IRGN0 / ORGN0 | `01` / `01` | Walks use inner/outer write-back, read/write-allocate attributes. |
| TCR.SH0 | `11` | Walk memory is Inner Shareable. |
| TCR.EPD0 / EPD1 | 0 / 1 | Lower walks enabled; an upper-region TLB miss faults without a walk. |
| TCR.A1 / AS | 0 / 0 | ASID comes from TTBR0 and uses the eight-bit configuration. |
| TCR.IPS | `000` | Selected intermediate physical address size is 32 bits. |
| TCR.HA / HD | 0 / 0 | Hardware Access-flag/dirty updates disabled. |
| TCR.TBI0 / TBI1 | 0 / 0 | No top-byte-ignore selection. |
| TCR.DS | 0 | The selected non-LPA2 descriptor layout applies. |
| TTBR0 | Aligned root address | The guest supplies ASID 0 and CnP 0. |
| MAIR.Attr0 | `ff` | Normal inner/outer write-back, non-transient, read/write-allocate. |
| MAIR.Attr1 | `04` | Device-nGnRE. |

These fields decode `TCR_EL1=0x803519` and `MAIR_EL1=0x4ff`. They describe
table walks and mappings, not proof that data caches are enabled or that a
physical interconnect implements the assumed memory map. The guest uses the
emulator's selected boot context and only sets SCTLR.M in its inherited
SCTLR value.

**Retained upper-region issue:** this constant has `TG1=00`, a reserved
encoding. D13-5955 specifies an implementation-defined supported granule
choice for that encoding; `T1SZ=0` is not a justified usable upper-region
size either. `EPD1=1` prevents a TTBR1 table walk on a miss in this specimen.
The observed lower-region accesses do not validate these upper settings.
Before enabling TTBR1, choose a supported TG1 encoding (4 KiB is `10`, unlike
TG0), an admissible T1SZ and an aligned table; inspect feature support and
invalidate/synchronize as required. Preserve the original specimen and
receipt; do not silently call its entire TCR configuration portable.

## Explain the leaf permissions and flags

The attribute diagram on D5-4844 and the definitions on 4845, plus table
D5-29 on 4853, decode the low leaf words:

| Word | Type | AP[2:1] | AF | SH | AttrIndx | Tested meaning |
|---|---|---|---|---|---|---|
| `0x703` | L3 page `11` | `00` | 1 | `11` | 0 | EL1 read/write, no EL0 data access. |
| `0x783` | L3 page `11` | `10` | 1 | `11` | 0 | EL1 read-only, no EL0 data access. |
| `0` | Invalid | — | — | — | — | Translation fault; no valid output mapping. |

The valid leaf entries have nG=0, so the final mappings are global rather
than isolated by a distinct ASID. The guest changes the selected root and
invalidates translations; it is not a test of simultaneous ASID-separated
processes. The RAM identity mapping also remains writable. A read-only
alias is not a claim that every alias of the backing page is read-only.

Arm AP controls data access. Instruction access separately involves PXN,
UXN and controls such as SCTLR.WXN (4844–4845, 4860). The guest does not
exercise all combinations. In particular, Sv39's MXR and SUM rules are not
interchangeable with Arm AP, PAN and UAO. D5.4.1–D5.4.3 (4849–4851) distinguish
these mechanisms; no cross-architecture equivalence is assumed.

D5.4.7–D5.4.12 (4860–4862) distinguish software Access-flag management from
optional hardware updating. With HA=0, an otherwise eligible AF=0 descriptor
causes an Access-flag fault rather than the selected guest setting it in
hardware. AF is pre-set in this experiment, so it **does not execute an AF
repair handler**. Arm dirty state uses AP[2] with DBM when its optional
hardware mechanism is enabled; it is not a direct copy of Sv39's independent
D bit. The existing delayed Sv39 repair exercise remains an independently
unassessed task. It does not become an Arm dirty-state implementation through
this reading.

## Visibility and changed conditions

D5.10.1 (4911–4913) permits intermediate translation information to be cached.
Writing a leaf or switching the table base does not alone establish visibility
of the new translation. K10.5.3 (11358–11360) explains table-store completion,
TLB invalidation, completion barriers and instruction-context synchronization.
The retained guest executes barriers and `TLBI VMALLE1`; it then observes 17,
29, a rejected write, and an invalid read in the selected single-PE run.
The source reading explains those bounded observations; it is not a new run.

The lack of an IS/OS suffix matters. D5-4915–4916 gives `VMALL` its stage-1
scope and confines a TLBI without a shareability suffix to entries usable by
the issuing PE. An ISB is local too. Multiprocessor reuse needs the appropriate
broadcast invalidation, issuing-PE completion, and synchronization on other
participants. A stronger DSB does not transform a local TLBI into a broadcast.

D5-4913 lists the changes requiring break-before-make when multiple threads
can use the same tables: selected memory-type/cacheability/output-address,
block-size and global-entry changes. Its sequence first invalidates the old
entry, completes that store, broadcasts invalidation and waits for completion,
then publishes and completes the replacement. The guest's root switch between
preconstructed tables and later valid-to-invalid leaf store are not evidence
of this full concurrent replacement protocol. Reusing either sequence in an
SMP page manager requires a new ownership, ordering and reclamation argument.

For the changed-condition defense, supply a ledger with: the input VA and
three indices; exact TCR/TTBR/MAIR fields; descriptor type, output-address
alignment, permissions and AF policy at each level; the chosen fault versus
successful result; and every visibility operation. Then change one premise:

1. Enable the upper region: repair TG1/T1SZ and derive its address selection.
2. Select a 2 MiB block: derive the level and alignment; reject an incompatible
   descriptor instead of treating it as a 4 KiB page.
3. Clear AF with HA=0: predict an Access-flag fault and design a separately
   validated software repair; do not relabel the old permission-fault test.
4. Add a second PE or hardware table updater: replace the single-owner
   premise with atomic-update, invalidation, completion and lifetime rules.

These are concrete analysis obligations, not newly executed target variants.
The original required implementation and learner variants remain in their
original packet. Stage 2, other granules, LPA/LPA2, tagging, hardware AF/dirty
updates, all fault-priority combinations, cache maintenance under changed
memory types, SMP shootdown, exact processor errata and physical observations
remain outside the selected guest contract. The source-access omission is
resolved; those additional engineering contracts are not silently acquired.
