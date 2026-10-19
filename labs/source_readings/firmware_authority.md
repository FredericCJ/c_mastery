# Firmware tables and the boundary of source authority

This required source comparison accompanies PR-T53-01 and its existing
`sa_parse`/`sa_discover` assessment. R-ACPI66-X2-SUCCESSOR records the actually
read ACPI6.6 section5.2.12.12 and table5.34. The type9 record is16 bytes, with
the x2APIC ID, flags and processor UID at offsets4,8,12. The reserved field is
specified as zero. Its flags use the Local APIC meanings. The compatibility
note discusses old OS expectations; it is not authority to truncate a32-bit
identifier to8 bits. Exact root/header passages remain an access gap in this
follow-through; the existing ACPICA declarations remain implementation evidence.

Compare these facts with the existing decoder contract. Distinguish CPU UID,
APIC ID and dense application index. Accepting a type9 layout does not enable
x2APIC hardware or prove that an arbitrary value can be used as an IPI target.
The teaching profile's aligned nonzero32-bit LAPIC base and refusal of address
override records are explicit limitations. Checksummed bytes still need checked
record lengths and duplicate admission. A specification's requirement on a
firmware producer is not automatically the decoder's chosen rejection policy:
state how reserved fields are treated and justify that policy separately.

Worked counterexample: two enabled records can have distinct UIDs but the same
APIC ID. A UID-only uniqueness check therefore admits an ambiguous hardware
destination. The existing exact criterion requires both identity spaces checked
before the final output commit. An unknown well-sized record must not change
the meaning of the already parsed identities. No test result here grants
permission to write to a physical controller.

Apply the same authority distinction to the required board ports. A vendor
device header can establish a named register field while leaving the board's
connector route, oscillator, analog threshold or reset retention unestablished.
Keep those exact dependencies in the source-gap record; do not fill them with
a plausible address or another board's pin assignment. Existing emulated
experiments and original source-supported helpers remain valid only in their
declared environments. Physical measurements and independent assessment stay
pending even after the remaining documents become readable.
