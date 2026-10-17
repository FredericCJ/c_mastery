# RISC-V and AArch64 execution ports laboratory

Core C17 arithmetic models plus RV64 LP64 and AArch64 LP64 assembly adapters. No hosted library in target startup; audit helpers and all undefined symbols. Board-specific interrupt controllers and firmware remain separate.

Portable core controls are independent of required guest/board observations. Ordinary init copies compiling learner stubs, tests and contracts. Explicit `-DCMASTERY_ASSESSOR_ROOT=/absolute/repository/assessor` selects reference sources; never use that flag for learner evidence.

Actual cross-linked image runs, hostile-register context tests, trap return and two-address-space translation observations in both dedicated guests are mandatory; model tests are not a complete OS port.

See the canonical project briefs and `CONTRACTS.md`.
