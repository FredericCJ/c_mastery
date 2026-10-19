# Exact remaining PR-T36-02 work

Inspected the full existing six-criterion supplement, parent project, fixture,
SPEC and lower-bound C/assembly/probe code. The originals already contain
real SysV/Thumb lower kernels and probes; retain them. Missing author evidence
was the exact changed Microsoft convention, callback-added leaf, changed
poisons/page layouts, size_t record stride, and explicit validation-versus-
search costs. This successor supplies those mechanisms and executes the
original kernels rather than replacing them with another C model.

Actual source inspection: Microsoft x64 calling-convention defaults, parameter
passing, unwindability for a leaf, scalar return and saved registers; AAPCS32
2025Q1 core registers, stack constraints, call/linker rules and base argument
allocation; cached AMD64 Draft0.99.6 sections3.2.1–3.2.3 printed pages15–21.
The direct Arm PDF was inaccessible and the web SysV PDF timed out. Their
actual accessible raw Arm RST and local source PDF are separately pinned;
failed requests are not counted as inspected pages. No whole-document reading
or current Windows execution is claimed.

The actual original interval fixture exposes lower/upper, high-bit ordering,
one-past results, extent admission and sortedness cost. Existing ABI mutation
controls are retained; distinct new callback and wrong-stride controls plus
protected-page observations exercise missing mechanisms. Profile C component
closure is checked separately from Linux host and semihost image scaffolding.

No accepted historical identities, receipts, hours or criterion wording are
rewritten. All other T36 projects and all T37 remain independently required.
Source inspection, reference execution, external review, independent learner
assessment and physical observation are separate evidence dimensions. Exact
published C17/C23 text remains a source gap.
