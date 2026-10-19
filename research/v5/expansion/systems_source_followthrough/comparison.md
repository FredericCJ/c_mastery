# Runtime and format source boundaries

The original three source-parent records remain preserved. Their concrete
assignments are captured in `assignment_scope.json`; each source below has
an exact version and passage boundary. The five earlier source children in
`source_reconciliation` remain bounded historical readings, not new full reads.

For **LSYS-ABI-GABI**, the retrieved AMD64 draft0.99.6 now supplies the formerly
failed historical call/entry passages. The retained ELF header, section,
string, symbol, relocation and segment sources cover the actual bounded byte
decoder. The fresh4.3-draft dynamic chapter distinguishes immediate dependency
search from transitive loading, optional lazy function binding from mandatory
early work, and initialization order from array order. Its new tags and draft
label do not establish equivalence with the older gABI or all processor ABIs.

For **LSYS-RUNTIME**, the previous explicit glibc startup/TLS, compiler-rt
division/ABI, Newlib-nano output-hook and DWARF/CFI passages match the selected
original tasks. Fresh musl bodies show the separate static and dynamic TLS
owners, later-module storage reservation for existing threads, and vector
publication after initialization. Relocation precedes copying final TLS
templates; a constructor may require services unavailable in early startup.
The fresh glibc mapper, relocation owner and lookup owner distinguish mapping,
zero-fill, protection changes, symbol matching and dependency retention.
The lookup's called matcher and machine-specific relocation macros are still
explicitly outside these fresh readings. A selected-helper contract does not
mean inspecting all of libgcc and compiler-rt, and a Newlib output proposal
does not claim every retarget hook of every embedded library.

For **LSYS-FORMATS**, the original Intel RevA format is now read completely
in a pinned11-page1998 reprint. Linear and segmented arithmetic differ, and
the two entry forms retain different processor metadata. The original small
HEX decoder was a deliberate generated-input subset; its refusal of wraps
and other types was not a claim about the full format. The required new
companion supplies each record mechanism and a changed programmer-window
policy, retaining that distinction. Existing GDB build-ID/debuglink readings
and actual mismatch controls cover the assigned companion format. A CRC or
matching name is neither authenticity nor address compatibility across builds.

This scoped resolution follows the actual assignments, including their
requirement to mark uninspected bodies. It does not silently replace an
assigned implementation with a title, remove a mechanism, or claim a complete
audit of the world's loaders, debug formats or compiler support libraries.
