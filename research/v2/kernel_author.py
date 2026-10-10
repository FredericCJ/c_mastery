"""Author the original T48 cards and bounded fixture catalog; no library payloads."""

import json

from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]

def write(path,value):

    p=ROOT/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(value,encoding='utf-8')

def dump(path,value):write(path,json.dumps(value,ensure_ascii=False,indent=2)+'\n')

cards=[

('Boot contracts and the long-mode transition',8,'boot',

 'GRUB supplies a 32-bit protected-mode entry, not a ready C environment. Preserve EAX/EBX, make the initial stack and page tables, load a GDT, enable PAE and long mode, then enter the 64-bit code segment. Our four-GiB supervisor identity map uses 2048 two-MiB entries; it includes the fixed PC MMIO window without exposing it to user mode.',

 'Write the Multiboot2 header, original transition assembly and link layout. Inspect entry address, header checksum and map before stepping the transition. Audit undefined symbols and distinguish bootloader service from learner-owned entry code.',

 'Wrong header checksum, lost EBX, malformed CR3 alignment, absent far jump, wrong C stack alignment and accidentally linked hosted startup.',

 'Boot the selected QEMU guest to the first serial marker and retain disassembly, register trace and ELF/map identities. Explain every supplied and owned operation.',

 'Move the kernel physical load address while preserving the boot/header and mapping relations.',

 ['Derive the machine-state and memory contract between GRUB, transition assembly and C.','Build and execute an original protected-to-long-mode transition with a correctly linked freestanding image.','Diagnose a deliberately broken entry/mapping assumption from the first supported machine-state divergence.']),

('Bounded boot information and memory reservations',6,'boot',

 'A boot tag carries a byte size; the following tag starts at its eight-byte-rounded end. Check the raw extent before rounding or reading fields. Preserve the module, boot structure and kernel physical ranges before giving any page to a process. Reserved overlapping map entries win over usable entries in this teaching allocator.',

 'Decode tags by explicit field widths, accepting larger compatible memory-map entry strides. Require one bounded module and one map. Generate truncated headers, invalid strides and overflowing physical ranges before using actual GRUB input.',

 'Size below eight, rounded tag beyond total size, overflowing base plus length, missing terminator, duplicate required tag and reserving only kernel .text.',

 'Pass kernel_boot and retain an actual boot reservation ledger. A synthetic tag at offset 8 with size 21 must advance to offset 32.',

 'Add an unknown tag and a larger memory-map stride without losing the required entries.',

 ['Validate complete boot-tag and memory-map spans before interpreting fields.','Reserve boot-owned memory and give reserved overlaps precedence over available ranges.','Reject truncated or overflowing records while preserving the prior output contract.']),

('Serial diagnosis and normalized exception frames',8,'guest',

 'A trap enters asynchronously, so ordinary function-call preservation is insufficient. Normalize the hardware error/no-error cases, save all general registers and retain the interrupted CS/RIP/flags/stack. The kernel uses a separate double-fault stack and does not execute floating-point/SIMD instructions.',

 'Implement bounded serial output, GDT/TSS/IDT setup and original assembly save/restore. Draw the 15 saved general registers, vector/error pair and five-word hardware frame before writing the C frame declaration.',

 'Wrong pushed-word order, missing error placeholder, insufficient stack alignment at a C call, invalid TSS descriptor and returning with the wrong operand size.',

 'Retain a real saved-frame trace and successful return. In the declared frame there are 22 eight-byte words; the software declaration must agree with assembly.',

 'Inject a different user exception and terminate that process while preserving peers.',

 ['Separate architectural and software-saved exception state.','Implement a normalized trap entry/return and bounded first-cause diagnostics.','Distinguish a recoverable user fault from a fatal kernel-mode exception.']),

('Physical frames and explicit page-table ownership',12,'frames',

 'A frame belongs to exactly one allocation state: reserved, free or allocated. Alignment is necessary but not sufficient for release. Address translation is a separate structure: every process owns its user mappings while supervisor mappings remain inaccessible from ring 3.',

 'Build the bitmap/state allocator from boot memory, then create per-process tables. Zero newly exposed frames. Roll back a partly fulfilled allocation and delay reclaiming an exited process until another address space is active.',

 'Reserved-page release, double free, allocation of boot data, partial-allocation leak, stale mapping and exposure of nonzero reclaimed bytes.',

 'Exhaust a finite hosted map, show exactly 256 pages from [3 MiB,4 MiB), and execute zero-filled user allocation and process teardown in the guest.',

 'Tighten available RAM and demonstrate an atomic allocation refusal.',

 ['Derive and implement frame ownership and exhaustion invariants.','Construct explicit process mappings with zero-fill and bounded allocation rollback.','Explain when page-table and frame storage may be reclaimed after a context switch.']),

('Ring-3 isolation and guarded user storage',10,'ranges',

 'A user-capable upper table does not grant access through a supervisor leaf. The selected process window is two MiB; static image, heap and guarded 128-KiB stack have separate regions. Kernel identity mappings remain supervisor-only. Writable user data is non-executable and executable text is not writable.',

 'Construct the first privilege-return frame and enforce permissions at every mapping level. Run two instances that write different values at the same virtual address. Add a separate process that attempts to write the kernel load address.',

 'User bit propagated to a supervisor region, writable executable segment, wrapping extent, a cross-page buffer entering a guard and incorrect initial privilege flags.',

 'Show actual peer progress and a protection page fault at 0x100000 with present/write/user bits set, while both independent virtual data values survive preemption.',

 'Try instruction fetch from user data and classify the fault using the actual saved state.',

 ['Construct separate address spaces with explicit user/supervisor and write/execute policy.','Demonstrate process isolation with both positive and negative machine-level observations.','Explain guard-page detection and the limits of a finite memory-safety campaign.']),

('Timer preemption with a nonpreemptible kernel',10,'guest',

 'The timer regains control even when a user program never yields. Saving context and choosing the next runnable process are different mechanisms. The first scheduler is round-robin, one CPU and one thread per process; interrupts remain disabled inside the kernel. A long synchronous device operation therefore adds scheduler latency.',

 'Configure the pinned PC timer/PIC, issue end-of-interrupt, retain each process frame, switch CR3 and kernel-stack ownership, then return to the next user. Separate explicit yield from timer-caused switches.',

 'No EOI, switching to a dead process, using another process kernel stack, dropping a register, confusing syscall frequency with cooperative yield and claiming hard real-time behavior from QEMU ticks.',

 'Two CPU-consuming workers that never call yield complete after observed timer ticks; their private memory values survive. Record timer-driven switches and nonpreemptible sections.',

 'Change the quantum and add a bounded slow syscall; measure the different delay mechanisms.',

 ['Separate scheduling policy from context-save and address-space-switch mechanisms.','Implement timer-driven round-robin user preemption with valid stack ownership.','Measure and explain latency introduced by a nonpreemptible kernel.']),

('Static ELF64 and a read-only boot archive',12,'elf',

 'The loader consumes program headers, not section names. File bytes are copied into a checked virtual span and the remaining memory bytes start at zero. CMKA1 maps bounded unique names to non-overlapping image spans; it is a boot container, not a general filesystem.',

 'Decode ELF64 little-endian x86-64 ET_EXEC images with at most eight load segments, fixed 4096-byte alignment and no interpreter, dynamic linking or TLS. Validate all segments before mapping. Package init, workers, fault control and shell.',

 'Truncated header or payload, filesz greater than memsz, overlapping page mappings, entry outside executable storage, writable executable segment, duplicate archive names and payload/table overlap.',

 'Pass hosted truncation/extent cases and boot real independently linked ELF programs. Preserve zero-fill and rejected-image evidence separately.',

 'Add a segment whose memory tail crosses a page boundary without introducing a writable code page.',

 ['Validate the bounded ELF program-header contract before committing mappings.','Implement a checked named-span boot archive and zero-filled static image loader.','Distinguish the educational executable profile from general ELF and filesystem support.']),

('Bounded syscalls and revocable process handles',10,'handles',

 'The syscall number selects a service; user data never selects an arbitrary kernel entry point. CMK uses int 0x80 with RAX number/result and three argument registers. Validate arithmetic and every page crossed before copying. A handle combines a slot and generation and becomes invalid after close.',

 'Implement console, exit, yield, ticks/PID, page allocation, archive and block calls using the public header. Make the disk handle exclusive to init. Enforce read/write direction and reject high bits rather than silently truncating caller values.',

 'Unknown call, foreign/stale handle, pointer into kernel memory, huge/wrapping byte count, read output into text and handle use after close.',

 'Actual user programs receive errors for kernel pointers, oversized allocation, unknown syscall and a closed handle; hosted generation checks reject stale tokens.',

 'Reuse a handle slot with a new generation and retain the old caller token as a rejection test.',

 ['Define an explicit syscall ABI and checked copy-in/copy-out contract.','Implement process-local handle ownership, closure and generation rejection.','Demonstrate that untrusted argument rejection preserves kernel and peer state.']),

('Modern PCI virtio discovery and split queues',10,'guest',

 'A device capability describes a register window, not permission to dereference arbitrary offsets. Discover the modern block device and required capabilities, negotiate VERSION_1 and FLUSH, then publish a split queue. One outstanding request makes descriptor and bounce-buffer ownership visible.',

 'Inspect PCI capabilities with bounded traversal, reject unsupported transport/feature/extent cases, initialize an eight-descriptor queue and derive its DMA addresses from the declared identity mapping. Publish descriptors before advancing availability.',

 'Treating a PCI access capability as a MMIO window, wrong feature bank, absent required feature, wrapped notification offset, premature buffer reuse and a completion naming the wrong descriptor.',

 'Retain actual negotiated status, queue addresses, descriptor identities and successful guest requests. A 16-bit index wraps from 65535 to 0 while the corresponding eight-slot position wraps from 7 to 0.',

 'Use a failing or non-completing backend and explain why DMA storage remains reserved after timeout.',

 ['Decode the selected PCI capability and feature negotiation contracts.','Implement split-queue publication and completion ownership with bounded descriptors.','Explain transport-specific ordering and why volatile alone does not establish the complete DMA protocol.']),

('Block completion, flush and persistence boundaries',10,'guest',

 'The kernel block service moves exactly one 4096-byte page or reports failure. A virtio sector is a separate 512-byte unit. Successful completion of a write does not by itself close the declared durable-storage boundary; the store must request and observe flush.',

 'Implement bounded read/write/flush requests with a static DMA bounce page. Validate capacity before multiplying a page index by eight. Poison the device on timeout and retain in-flight storage. Reboot with the same ordinary raw image.',

 'Page beyond capacity, writable destination in kernel space, wrong sector units, missing flush feature, error status interpreted as completion and retry after losing DMA ownership.',

 'Write a patterned page, flush, read it, then boot a new guest and compare all bytes. Keep backend cache configuration and exact image identity; do not claim a physical power-loss test.',

 'Suppress the flush in a deliberately faulty adapter and show which evidence no longer supports durability.',

 ['Separate request completion, buffer release and persistent-state claims.','Implement explicit bounded page/sector conversion and flush propagation.','Qualify reboot observations against the actual emulated backend failure model.']),

('Init, bounded console shell and teardown',8,'archive',

 'Init and the shell are ordinary isolated ELF programs. The small shell owns a 64-byte command buffer, reads nonblocking console bytes and yields when no byte is available. The command set is help, echo and exit; overflow follows an explicit reset-and-report policy.',

 'Package named init/shell images in the archive, implement a bounded line parser and exercise input through the real serial channel. Exit revokes capabilities and releases user frames after scheduling away.',

 'Unterminated command, overlong input, duplicate image name, command-buffer reuse across an outstanding operation and freeing the currently active kernel stack.',

 'An automated interactive guest receives help and exit and produces the expected bounded response; hosted archive tests reject malformed tables.',

 'Add one read-only diagnostic command without adding raw kernel memory access.',

 ['Build init and a bounded shell against the custom ABI rather than hosted libc.','Test interactive input and process teardown through actual guest execution.','Defend the boundary between a boot archive, shell and general process/filesystem services.']),

('One portable VM core inside a user process',10,'guest',

 'Porting means changing adapters while retaining the VM relation and bytecode. The kernel never interprets VM object references or accepts native addresses from bytecode. Its user process owns Cvm and program storage, supplies a bounded numeric callback and preserves separate instruction and host-call budgets.',

 'Link the original T46 core into a freestanding ELF, audit compiler helpers and user stack usage, then execute a hand-encoded CVM1 program. Keep the hosted compiler outside this guest milestone.',

 'Duplicated interpreter, silently linked libc, missing stack budget, arbitrary FFI pointer, failed callback returned as successful VM execution and an exhausted host budget ignored.',

 'Actual ring-3 execution computes 42 through a bounded callback and independently returns CVM_HOST_BUDGET with zero callback allowance. Record the shared source identity.',

 'Load a new bounded bytecode fixture through the boot-archive API and reject its malformed variant.',

 ['Port the unchanged portable VM through explicit user-space adapters.','Reconcile VM, compiler-helper and process resource contracts.','Demonstrate runtime error and host-call budget behavior inside the original OS.']),

('VM, durable store and kernel reboot integration',12,'guest',

 'The store owns B+tree pages, WAL and recovery; the kernel owns the block transfer/flush service. An exclusive adapter maps the 67-page store image to raw pages starting at 16. VM bytecode may request named store services but cannot choose a device page or kernel pointer.',

 'Link T46/T47 cores in init with caller-owned static workspace. Execute bytecode that commits value 42, deliberately lose the callback success reply, restart the guest on the same disk, recover and read the value through bytecode.',

 'Double-charged copied implementation, write without flush, bytecode raw-device access, unknown transaction outcome treated as definitely absent, wrong store-slice arithmetic and stack exhaustion from nested workspace.',

 'Preserve first-boot CVM_HOST_ERROR after the known committed callback, second-boot VM GET42, WAL recovery/store-check evidence and actual syscall/virtio image identities. This is a lost-reply experiment, separate from the store simulator crash matrix.',

 'Change the callback value and cut execution at a selected storage fault-model boundary; defend acknowledged versus indeterminate outcomes.',

 ['Compose runtime, transaction and block interfaces without bypassing ownership layers.','Execute bytecode-to-durable-storage and reboot recovery on the original OS.','Explain lost-response ambiguity without equating a reported error to absence of commit.']),

('Kernel diagnosis, negative controls and delayed transfer',12,'guest',

 'A green image is one experiment. Diagnose entry failures, bad page permissions, malformed images, lost context and queue ownership at their first violated boundary. Keep host and guest results separate so a correct model cannot conceal broken assembly or an unexecuted device path.',

 'Run the dual-compiler hosted matrix and real guest campaign, then introduce one controlled mutation at a time in a separate learner copy. Use GDB saved state and serial evidence to minimize the first causal divergence. Rebuild from a new directory.',

 'Synthetic log substituted for guest output, source/image mismatch, timeout reported as pass, changed expected marker hiding a defect and a known fault reported as blind diagnosis.',

 'Reject learner stubs and corrupted ELF/handle variants; pass repaired host and guest gates. Complete a reviewer-held perturbation after seven days and annotate unresolved physical/SMP/general-OS claims.',

 'Defend and implement one changed resource limit or fault policy without consulting the old solution.',

 ['Use independent host and machine-level evidence to diagnose an original kernel defect.','Preserve matching source/image/tool identities and reject inadequate negative controls.','Transfer the mechanism to a delayed changed-condition assessment with explicit limits.'])

]

resources=[

('R-KER-MULTIBOOT2','Multiboot2 Specification','Free Software Foundation','version 2.0',2016,'standard','https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html',['3.3 I386 machine state','3.6 Boot information; modules and memory map'],'Boot state, tag sizes and reservations; no bootloader implementation is supplied by this reading.'),

('R-KER-INTEL3A-060','Intel 64 and IA-32 Architectures Software Developer Manual, Volume 3A','Intel','253668-060US, September 2016',2016,'manual','https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf',['6.14.1–6.14.5, printed pp. 6-16–6-19'],'Historical exact edition inspected for IA-32e IDT, frame, IRET, TSS/IST behavior; not the current edition or full processor errata.'),

('R-KER-ELF-GABI','System V Application Binary Interface: Program Header','System V ABI authors','gABI public reference',2001,'standard','https://refspecs.linuxfoundation.org/elf/gabi4+/ch5.pheader.html',['Program Header','PT_LOAD','Segment Permissions'],'File/memory extent, zero-fill and permissions; the lab accepts a narrower static ELF64 profile.'),

('R-KER-VIRTIO-1-2','Virtual I/O Device (VIRTIO) Version 1.2','OASIS','Committee Specification 01, 1 July 2022',2022,'standard','https://docs.oasis-open.org/virtio/virtio/v1.2/cs01/virtio-v1.2-cs01.html',['2.7 Split Virtqueues','3.1 Device Initialization','4.1 Virtio Over PCI Bus','5.2 Block Device','6 Reserved Feature Bits'],'Modern PCI/split-queue/FLUSH contract; no packed-queue, MSI-X, IOMMU or general-device claim.'),

('R-KER-QEMU-PC','QEMU i440fx PC system documentation','QEMU project','Documentation inspected 2026-10-10; executable 10.2.1',2026,'manual','https://www.qemu.org/docs/master/system/i386/pc.html',['i440fx PC','Invocation: machine, accelerator, disk and serial options'],'Device-model and invocation scope; master documentation is not a claim about every QEMU release.'),

('R-KER-OSTEP-LDE','Operating Systems: Three Easy Pieces — Mechanism: Limited Direct Execution','Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau','Version 1.10',2023,'book','https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-mechanisms.pdf',['6.2 Restricted Operations','6.3 Switching Between Processes'],'Conceptual privilege, trap and timer mechanisms; x86-specific ABI comes from the chosen architecture manual.'),

('R-KER-OSTEP-PAGING','Operating Systems: Three Easy Pieces — Paging: Introduction','Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau','Version 1.10',2023,'book','https://pages.cs.wisc.edu/~remzi/OSTEP/vm-paging.pdf',['18.1 A Simple Example And Overview','18.2 Where Are Page Tables Stored?'],'Frames and per-process translation examples; not a complete x86 page-table implementation.')]

reg=[]

for rid,title,authors,edition,year,kind,url,locators,scope in resources:

 reg.append(dict(id=rid,title=title,authors=[authors],edition=edition,year=year,kind=kind,url=url,access='Public primary author/publisher source',domains=['T48'],role='oracle' if kind!='book' else 'explanation',reason=scope,prerequisites=['C ownership','binary layouts','x86-64 assembly'],locators=locators,assignable=True,inspection=dict(date='2026-10-10',method='Bounded official/author page or PDF passages inspected',scope=scope,limits='Only the named passages were inspected; actual execution is recorded separately.',evidence_urls=[url],local_document_id=None,local_sha256=None)))

dump('research/v2/kernel_resources.json',{'resources':reg})

dump('research/v2/kernel_research_gate.json',{'accepted':True,'scope':'T48 bounded source/design gate, not independent final acceptance','date':'2026-10-10','sources':[x['id'] for x in reg],'local_search':{'library':'F:/Knowledge','findings':['Operating Systems — Principles and Practice, Volume I, second edition, a3fe108a (lead only)','Understanding the LINUX KERNEL, cb9a7486 (lead only)','System V AMD64 ABI draft 0.99.6, 7f7d6665 (lead only)'],'graph':'F:/Projects/Writing/corpora/SWE','decision':'Use inspected primary standards plus author-hosted OSTEP chapters; library filename discoveries alone do not authorize chapter claims.'},'reflection':['Keep the boot ABI distinct from hosted C startup.','Prefer a precisely bounded modern virtio transport over a copied production driver.','Preserve runtime/store implementation ownership; only adapters and integration are new.','An older exactly inspected Intel passage is labeled historical, not silently presented as revision 093.'],'limitations':['No physical x86 hardware run','No SMP or POSIX/general ELF conformance','QEMU reboot is not physical power-cut certification']})

subtopics=[];projects=[];family=[]

for i,(title,hours,kind,mechanism,steps,failures,accept,transfer,objectives) in enumerate(cards,1):

 tid=f'T48.{i:02d}';pid=f'PR-T48-{i:02d}';projects.append(pid)

 subtopics.append({'id':tid,'title':title,'elective':False,'objectives':[{'id':tid+'.'+letter,'text':text} for letter,text in zip('abc',objectives)]})

 tests=['kernel_guest'] if kind=='guest' else ['kernel_'+kind]

 if i==1:tests=['kernel_boot_smoke']

 if i==12:tests=['kernel_runtime']

 if i==13:tests=['kernel_integration']

 if i==7:tests.append('kernel_archive')

 if i in [5,8,11]:tests.append('kernel_guest')
 deferred_tests=[]
 if 3<=i<=10 and 'kernel_guest' in tests:
  tests.remove('kernel_guest');deferred_tests=['kernel_guest']

 family.append({'id':pid,'tests':tests,'deferred_tests':deferred_tests,'capabilities':['linux-c17']+(['qemu-x86-64-grub'] if any(t in tests for t in ['kernel_boot_smoke','kernel_guest','kernel_runtime','kernel_integration']) else []),'scope':'Selected tests are immediate anchors; an empty set requires the project-local manual guest trace. deferred_tests are later cumulative regression only. Cross-family variables required for projects 12–13.'})

 meta={'id':pid,'title':title,'module':'T48','topics':[tid],'status':'core','hours':hours,'profile':'F','fixtures':[f'fixtures/kernel/{pid}.json'],'edition':2}

 if i==12:meta['requires_projects']=['PR-T48-11','PR-T46-12']

 if i==13:meta['requires_projects']=['PR-T48-12','PR-T47-12']

 body='```json\n'+json.dumps(meta,indent=2)+'\n```\n\n# '+pid+': '+title+'\n\n## Contract\n\n'+mechanism+'\n\nThis '+str(hours)+'-hour milestone includes implementation and verification. Reading, assessment and delayed retrieval are in the module budget. Reuse accepted prerequisites; implement the studied mechanism in the learner workspace.\n\n## Inputs and expected results\n\nUse `'+meta['fixtures'][0]+'` for the declared finite calculation and `labs/kernel` for actual C/guest controls. Static arithmetic does not close the executable gate. The relevant CTest names are '+', '.join('`'+x+'`' for x in tests)+'. For early projects 03–10, a full kernel_guest run is a later cumulative regression: the immediate component dossier must include the project-local trace specified below; the full run cannot be claimed until its later dependencies exist. The first project uses only kernel_boot_smoke and stops after the actual C-entry serial marker. Cross-family projects explicitly enable the original runtime/store source paths; a standalone kernel pass is insufficient.\n\n## Steps\n\n'+steps+'\n\nKeep the contract, pre-run prediction, exact image/build identity, raw observation and first invalid transition. Reveal hints in `labs/kernel/HINTS.md` in stages. The default stubs compile but fail; selecting an assessor solution changes the evidence category.\n\n## Failure cases\n\n'+failures+'\n\n## Acceptance and evidence\n\n'+accept+'\n\n'+ '\n'.join('- **'+tid+'.'+letter+'**: '+text for letter,text in zip('abc',objectives))+'\n\nMap each objective to a concrete file/observation, retain negative controls and state unresolved claims. Refer to {ref}`guide-assessment` for rubric and causal repair.\n\n## Hints and recovery\n\nStart with a hosted checked boundary whenever possible, then cross the actual machine boundary. A publication or model verifier is not a substitute for the named boot, privilege, queue or reboot observation. Reduce one failed boundary before editing unrelated mechanisms.\n\n## Transfer challenge\n\n'+transfer+' Attempt after at least seven days, with a reviewer-held variant when available.\n'

 write('curriculum/projects/'+pid+'.md',body)

dump('research/v2/kernel_taxonomy.json',{'edition':'C mastery overhaul, T48, October 2026','domains':[{'id':'T48','title':'A small isolated x86-64 operating system','subtopics':subtopics}]})

reading=[('R-KER-MULTIBOOT2',0,2),('R-KER-MULTIBOOT2',1,2),('R-KER-INTEL3A-060',0,6),('R-KER-ELF-GABI',0,1),('R-KER-ELF-GABI',1,1),('R-KER-ELF-GABI',2,1),('R-KER-VIRTIO-1-2',0,1),('R-KER-VIRTIO-1-2',1,1),('R-KER-VIRTIO-1-2',2,1),('R-KER-VIRTIO-1-2',3,1),('R-KER-VIRTIO-1-2',4,1),('R-KER-QEMU-PC',0,1),('R-KER-OSTEP-LDE',0,2),('R-KER-OSTEP-LDE',1,2),('R-KER-OSTEP-PAGING',0,1),('R-KER-OSTEP-PAGING',1,1),('R-parnas-modules-1972',0,1)]

lookup={r['id']:r for r in reg}

meta={'id':'T48','title':'A small isolated x86-64 operating system','track':'kernel','status':'core','prerequisites':['T08','T09','T16','T23','T27','T32','T37','T46','T47'],'prelude_objectives':[],'workload':{'reading':26,'implementation':90,'verification':48,'assessment':10,'consolidation':12},'readings':[{'resource_id':r,'locator':lookup[r]['locators'][li] if r in lookup else 'The Criteria','purpose':lookup[r]['reason'] if r in lookup else 'Defend VM/store/kernel interface ownership using the foundational decomposition publication.','hours':h} for r,li,h in reading],'projects':projects,'profile':['C','F'],'equipment':['Linux/WSL GCC and Clang, CMake, Python','QEMU 10.2.1 pc-i440fx-10.2 qemu64 TCG, one CPU, 128 MiB','GRUB 2.14 rescue tools, GNU ld, xorriso','An ordinary disposable disk image in the build directory'],'optional_project_hours':0,'edition':2}

body='```json\n'+json.dumps(meta,indent=2)+'\n```\n\n# T48: A small isolated x86-64 operating system\n\n## Purpose and outcomes\n\nBuild an original bounded kernel that runs isolated user programs, preempts them, provides checked services and persists a VM/store result through its own block driver. The supplied bootloader, emulator and packaging are explicit infrastructure. Fourteen small milestones contribute 138 implementation/verification hours; 26 reading, 10 assessment and 12 consolidation hours make a 186-hour module estimate. Actual time and remediation must be recorded, not hidden behind this estimate.\n\n## Entry check and remediation\n\nBefore entering, derive a callable assembly ABI, inspect an ELF/map, implement checked span parsing, and account for fixed/stack storage. Complete the named foundations; T46/T47 are additionally required before projects 12/13. Earlier boot, interrupt, ownership, algorithms and debugger artifacts are reused by original ID and not charged as new implementations.\n\n## Component progression\n\nBoot and inspect first; validate boot data; establish traps and frames; construct isolated process memory; regain control through a timer; load bounded images; expose checked services; then add a real queue/block driver. The shell, VM and durable store are clients. The OS route is x86-64 with its necessary small 32-bit transition; Thumb remains the separate microcontroller/runtime target.\n\n## Study sequence and teaching\n\n'

for i,card in enumerate(cards,1):

 title,hours,kind,mechanism,steps,failures,accept,transfer,objs=card

 body+=f'### T48.{i:02d} — {title}\n\n{mechanism}\n\n{steps}\n\nA discriminating failure campaign includes: {failures[0].lower()+failures[1:]} The gate is concrete: {accept}\n\nDevelop and assess the evidence in {{ref}}`PR-T48-{i:02d}`.\n\n'

body+='## Reading guidance\n\nUse the exact primary passages in the reading list for wire/layout/entry claims and the author-hosted OSTEP chapters for explanatory models. Parnas supplies a foundational argument for separating hidden implementation decisions. The searched local library and SWE graph remain lead collections until passages are inspected. The 2016 Intel excerpt is an exactly identified historical source; current errata or a new processor requires renewed inspection.\n\n## Implementation projects\n\n'+ '\n'.join(f'- {{ref}}`PR-T48-{i:02d}`: {card[0]} — {card[1]} h.' for i,card in enumerate(cards,1))+'\n\n## Mastery gate and feedback\n\nPreserve original learner code, contract/ABI map, source/image identities, hosted cases, actual boot/device/peer traces, resource ledger, first-cause repair and delayed changed-condition result. Read `labs/kernel/README.md` for exact commands and unimplemented-stub behavior. The integrated gate must show actual bytecode to store to syscall to virtio, a committed/lost-response distinction, and a fresh boot that recovers the value. Reusing assessor evidence is infrastructure review, not learner mastery.\n\n## Consolidation and transfer\n\nAfter at least seven days, diagnose one unseen page, ELF, handle or queue perturbation and change one resource limit. Explain which claims survive and which need new observations. Reconcile source version, adapter semantics and available capabilities before comparing runs.\n\n## Environment and limits\n\nThe selected kernel is single-core, uses static ELF admission at boot and a nonpreemptible kernel. Synchronous bounded device polling may delay peers. There is no required SMP, general POSIX/filesystem/network stack, JIT, dynamic linker or physical-device conformance. User faults terminate the process; unexpected kernel exceptions stop the diagnostic image. QEMU-backed persistence and supplied crash models remain separately scoped.\n'

write('curriculum/modules/T48.md',body)

dump('labs/kernel/family.json',{'version':1,'family':'kernel','module':'T48','title':'CMK original x86-64 OS','build_system':'cmake','default':'learner stubs compile; behavioral/guest gates remain unimplemented','assessor_root':'assessor/kernel','projects':family,'commands':{'configure':'cmake -S . -B build -G Ninja','build':'cmake --build build','test':'ctest --test-dir build --output-on-failure'},'capabilities':{'host':'Linux C17 GCC/Clang','guest':'QEMU pc-i440fx-10.2 + GNU GRUB + xorriso','physical':'not required and not claimed'},'integration':'Optional explicit runtime/store paths described in README; guest checks build receipt before requiring corresponding markers.'})

cases=[('boot',{'large_page':2097152,'mapped_bytes':4294967296},{'entries':2048}),('tags',{'offset':8,'size':21},{'next':32}),('frame',{'gpr':15,'metadata':2,'hardware':5,'word':8},{'bytes':176}),('pages',{'begin':3145728,'end':4194304,'page':4096},{'count':256}),('fault',{'present':True,'write':True,'user':True},{'code':7}),('schedule',{'alive':[True,False,True],'current':0,'steps':4},{'sequence':[2,0,2,0]}),('elf',{'filesz':1,'memsz':4096},{'zero_bytes':4095}),('handle',{'slot':0,'generation':1},{'token':65537}),('queue',{'index':65535,'slots':8},{'next':0,'before_slot':7,'after_slot':0}),('disk',{'bytes':4194304,'page':4096,'sector':512,'slice_start':16,'slice_pages':67},{'pages':1024,'sectors_per_page':8,'slice_end':83}),('archive',{'header':16,'entries':1,'entry_size':40,'payload':8},{'offset':56,'total':64}),('vm',{'input':41,'host_increment':1,'host_budget':1},{'value':42,'remaining':0}),('commit',{'prior':0,'committed':42,'reply_received':False},{'after_recovery':42,'reply_proves_absence':False}),('scope',{'host':True,'guest':True,'physical':False},{'physical_complete':False})]

for i,(kind,inp,expected) in enumerate(cases,1):dump(f'fixtures/kernel/PR-T48-{i:02d}.json',{'id':f'FX-KERNEL-{i:02d}','kind':kind,'description':cards[i-1][0],'input':inp,'expected':expected,'verification_method':'Independent finite arithmetic/state certificate. Actual C/QEMU execution is separate.'})

print('Authored T48: 14 projects, 42 objectives, 186 h, 14 finite fixtures')

