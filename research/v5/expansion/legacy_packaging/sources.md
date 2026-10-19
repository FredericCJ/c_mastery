# T22 bounded source and mechanism audit

Actual T22 module, four project briefs, four supplemental criterion/task sets,
four separate assessor analyses and all four supplied synthetic fixtures were
read. All30 ordinary and30 exact delayed tasks were extracted and inspected.
Unit mappings and step/check text were inspected, with a focused untruncated
read of U-T22-04; no fresh historical acceptance is asserted. The old synthetic
role/trace/allocator/release fixtures do not execute a package or foreign client.

The actual earlier `ApiBuffer`/`api_append` and registry implementation source
was read. The successor imports that source unchanged; append, not the registry,
is the packaging mechanism dependency. Its source hash is pinned in every run.
All new public/private files were authored and locally reviewed by the same
author; that is not independent review.

Primary passages freshly read on2026-10-17:

- Doxygen live manual, Special comment blocks/C-like comments, Putting
  documentation after members and Documentation at other places. Actual member
  versus parameter syntax and file documentation requirement informed the two
  rendered configurations; no assumption was made that every comment is parsed.
- CMake live Importing and Exporting Guide, Exporting Targets, package config
  creation and Creating Relocatable Packages. The fetched page identifies4.5.0-rc1;
  actual executable version is recorded separately. Installed relative paths and
  transitive target dependencies are tested with a real moved prefix.
- Python live ctypes manual (served as3.15 documentation), Return types and
  callback lifetime note. Actual Python runtime version is separate. Explicit
  declarations, retained wrappers and the default-int issue are tested without
  invoking a deliberately mismatched raw-pointer call.
- Git live git-bisect manual, Bisect skip/run: exact0/1..127/125 protocol and
  adjacent-skip ambiguity. The original local history executes a semantic
  predicate; no curriculum repository history is changed.
- Hanson's author-hosted Chapter3 sample, section3.2 printed34–39: helper
  delegation, atom table structure, validation/search/allocation/publication.
  This is a bounded source-reading comparison, not copied target code or a claim
  to the complete book. Historical implementation choices are not modern safety
  guarantees; fixed-size conversion storage and unbounded allocation require
  their stated assumptions.
- Actual local package notices for `/usr/share/doc/cmake/copyright` (top-level
  and differently scoped component records), `/usr/share/doc/libc6/copyright`
  (main and exceptional scope records), and privately extracted Doxygen1.15.0
  copyright (main, bundled JavaScript and Expat text). GPL2 section0/preamble
  and LGPL2.1 preamble were read as document identity/scope context, not a full
  distribution-law opinion. Exact notice hashes are retained in the campaign.

The repository-wide filename scan found a MathJax-specific Apache notice but no
top-level license file; no grant for the new target is inferred from that
unrelated asset. Distribution permission remains unresolved. Doxygen/compiler/
runtime notices do not authorize arbitrary reuse of every bundled artifact.

Exact published ISO C17 is not newly inspected. Modern C10.1 is not freshly read
in this successor; earlier source evidence remains separate. Primary tool
documentation is live and its served version can differ from the installed tool.
No whole upstream distribution or complete license corpus is claimed inspected.

Pilot findings retained in build logs: a compiler option containing an unquoted
space broke the path-with-spaces build; quoting each complete prefix-map option
repaired it. Doxygen rejected a renamed input's explicit old file name and tried
an unavailable optional graph tool; a filename-independent file marker and
explicit graph-disabled configuration repaired the actual selected contract.
These failures were not counted as successful validations. The final report
must bind stable sources, all actual output hashes and expected negative results.
