# Cacheable cold-start analysis

The original cache adapter already implements the per-owner maintenance order.
Its premise was not discharged by the disabled-cache lifecycle image. The new
linker sections and startup loops establish actual exclusive line placement;
the cold entry establishes the mapping and cache state before peer admission.
PM0253's normal nonshareable WBWA encoding is intentional. The S attribute is
not a C synchronization primitive and must not replace the actual HSEM protocol.

All old region descriptors are disabled while MPU_CTRL is0; otherwise a higher
numbered retained region could override region7 after enable. The admitted
CCSIDR forms have four ways and32-byte lines. Their physical set/way word is
way*2^30+set*32, with set<=511. Neither arithmetic term overflows and the bit
fields are disjoint. Invalidating before cache enable prevents uninitialized
tag state; writing the line afterward deliberately creates dirty state in the
declared model. Only the full clean followed by DSB authorizes CPU2 release.

The type `struct fw_peer` exists in a dedicated target section. C's object representation
is not a claim about automatic physical coherence. M7 accesses mutable fields
only under HSEM and maintenance; M4 has its own noncacheable view. A stale
generation must be rejected, not overwritten. The CPU2 reset path may not clear
the line even though each ELF contains its own symbol describing the shared
object. NOLOAD alone does not prevent arbitrary startup memset: the bounded
startup ranges are also inspected.

Callback writes are not transactional hardware updates. Invalid geometry changes
the selector; unsupported option state may leave SYSCFG enabled. Those are
explicit failure semantics. The no-reentry/exclusive cold-run premise prevents
concurrent profile changes. Actual asynchronous cache-maintenance BusFaults can
still occur; the model cannot recover them. A physical failure requires the
declared fail-stop/reset protocol and retained evidence.

Controls map to distinct claims: early release exposes old RAM; using the M4
map on M7 violates cacheability even if ordinary values look correct; skipping
invalidation violates startup cache authority; skipping clean exposes dirty
cache/RAM divergence; peer reinitialization destroys old ownership; admitting9
MPU regions contradicts the selected8-or16-region contract. Each control
must compile and reach its exact semantic assertion in all native profiles.
Independent set/way enumeration is structurally separate from the C loop.
Native and MPS2 model execution do not demonstrate actual STM32 coherency.

The original protocol closes admission once readiness is committed under HSEM,
but a trusted later failure can revoke readiness. Actual flash commit and DMA
quiescence are separate providers. The selected workload holds no DMA lease;
reject a submission claiming that CPU health alone proves a powered device
stopped. The physical changed-condition obligation remains open until genuine
dedicated-board traces are supplied, and every independent rubric remains
unassessed by author campaigns.
