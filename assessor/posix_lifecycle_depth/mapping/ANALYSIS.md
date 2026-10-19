# Restricted mapping and owner-death analysis

Logical bounds must dominate rounded virtual bounds. The one-byte read at
logical end is inside a mapped page but outside the record contract. Generation
validation happens before any pointer formation; remap first acquires a second
same-inode mapping, then unmaps the first and advances the generation. A raw
borrow would be invalid even if virtual address bits happened to be reused.
The two simultaneous mappings force distinct extents without MAP_FIXED.

The codec's independent byte order check uses session0x0102030405060708 and
expects08..01 or01..08; it does not call the codec's decoder as its sole oracle.
SIZE_MAX and all-ones offset cases must be rejected before address arithmetic.
The payload checksum is neither authentication nor a full header-integrity claim.
In the trusted local format, session checks fence stale operations; adversarial
participants that can alter bytes or ignore advisory locks need a different
security contract.

An initializer holds a kernel process lock while its bytes are invalid, partially
constructed, or committed. After each confirmed hook, the parent kills exactly
that owned child and waits. Kernel lock release establishes that this owner no
longer holds the exclusion token; the byte state determines whether there is a
complete record. This avoids resetting an unknown partially initialized pthread
mutex. A subsequent authorized initializer can replace uncommitted bytes, while
a committed winner refuses a second initialization. New sessions are supplied
by the external owner, not inferred from PID reuse or a timeout. Replacing a
committed session requires both expected old identity and strict advancement.

The no-lock reader mutant is discriminated at stage3: the record bytes are
complete but the live initializer still holds exclusive ownership. Stages1/2
alone would fail to distinguish the missing lock because both correct and
unlocked decoders see an uncommitted record. The stale-session mutant is checked
after replacement. Thus the test observes mechanisms, not merely checks a final
checksum. Codec controls accept uncommitted state or force little endian; mapping
controls use physical extent instead of logical extent or omit generation advance.

Residency results are deliberately not hard-coded to the common Linux response.
They are samples and can change under scheduling/memory pressure. Private/shared
tests assess visibility after explicit child completion. Neither one RSS sample
nor a private mapping proves exclusive physical backing.

Fault specimens are separately executed and intentionally unsanitized. The parent
truncation happens only after the mapped helper's ready token and before its
release token. PROT_NONE and beyond-file access then yield classified kernel
signals. Controls remove the protection change or access the surviving page;
the parent must reject the resulting normal exit. This is not sanitizer silence,
exploit execution or physical durability evidence. Core limits change only in
the dedicated helper. All files are inside the runner's temporary working directory.

The remaining .09.b explicit-I/O versus mapping workload comparison must use
equivalent validation/error contracts. This package intentionally does not report
fabricated timing or close that separate criterion. General cancellation,
thread-shared locks, a persistent session allocator and arbitrary storage crash
recovery also require their own designs.
