## ADDED Requirements

### Requirement: Pinned layout enforcement
Every in-scope C file SHALL be laid out exactly as the pinned clang-format
21.1.8 produces with the repository `.clang-format`, except genuine Linux kernel
modules, which SHALL use the kernel-style configuration named in the manifest.

#### Scenario: Reformatted family passes the check
- **WHEN** a family's manifest state is `formatted` or later
- **THEN** `tools/check_c_style.py` reports no layout failure for its files

#### Scenario: A different clang-format version is rejected
- **WHEN** the style check runs with a clang-format whose version is not 21.1.8
- **THEN** it refuses to run instead of reporting a result

### Requirement: Typedef-free aggregate types
Structures, unions and enumerations SHALL be referenced through lowercase
module-prefixed tags, and remaining typedefs SHALL be lowercase, with
function-pointer typedefs ending in `_fn`.

#### Scenario: Aggregate typedef is reported
- **WHEN** an in-scope file declares `typedef struct { ... } Name;`
- **THEN** the style check reports an `aggregate-typedef` finding for that file

### Requirement: Behaviour-preserving transformation
The mechanical rename, formatting and brace-insertion steps SHALL each be proven
equivalent before they are committed: object-code identity for the rename,
identical C token sequences for formatting, and a token difference consisting
only of inserted braces for brace insertion.

#### Scenario: Formatting changes a token
- **WHEN** a formatted file's significant token sequence differs from the original
- **THEN** the formatting step fails for that file and nothing is committed

### Requirement: Recorded exceptions
A rule SHALL be broken only through a manifest entry or an in-source marker
with a reason: `CM-SPECIMEN:` for deliberate defect specimens and `CM-STYLE:`
for `clang-format off` regions.

#### Scenario: Unjustified format-off region
- **WHEN** a file contains `clang-format off` without `CM-STYLE:` on that line
- **THEN** the style check reports an `unjustified-format-off` finding
