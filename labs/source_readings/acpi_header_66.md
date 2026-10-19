# ACPI6.6 source successor for T53.01

Read the official [ACPI6.6 chapter5](https://uefi.org/specs/ACPI/6.6/05_ACPI_Software_Programming_Model.html#root-system-description-pointer-rsdp-structure), sections5.2,5.2.5 and5.2.6, tables5.3–5.4. The2026-10-18 web view made these passages readable; subsequent RSDT/XSDT requests still returned403. Earlier access failures remain historical.

The inspected specification defines little-endian numeric fields, a trailing-space RSDP signature, a checksum over its first20 bytes and another over the entire extended structure. Revision0 lacks the extended fields; revision2 and above admit them. The common table header includes total length and a checksum over that length.

Compare these facts with the actual `sa_discover`/`table` implementation in the T53.01 exercise. It accepts only revision0 or2 and requires extended length36. Those are explicit teaching-profile restrictions, not general forward-compatibility rules. It uses byte decoding and caller-supplied mapped spans rather than dereferencing physical integers. Successful checksum verification does not prove mapping safety or record validity.

For AX-PR-T53-01-T53.01.b, explain refusal of a correctly checksummed revision3 structure and revision2 structure extended to40 bytes. Derive why two checksum conditions differ. Assess using the existing publication/mapping rubric; an answer claiming general ACPI conformance fails. Private comparison checks exercise the unchanged implementation. Root-table-body semantics remain a named source gap; no physical discovery or complete-standard inspection is inferred.
