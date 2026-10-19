# Bounded source inspection, 2026-10-18

Read the actual N1570 PDF pages18-20 (PDF pages35-37), 5.1.2.4p14-18,
and the 7.17.3.1 passage on PDF page295. These define operand/scalar-copy and
transitive dependencies, exclude specified operator operands and kill_dependency,
and restrict the ordering argument to the dependency path. This is the
April12,2011 C11 committee draft, not a final C17 or C23 copy.

Read the current GCC `__atomic` Builtins memory-order introduction,
`__ATOMIC_CONSUME`, and fallback description. The documented implementation uses
acquire for consume and may call helpers without an inline target mechanism.
The page expressly calls its overview illustrative. Actual compiler/target
behavior is tested separately; neither GCC nor this graph exercise establishes
ISO wording for inaccessible editions.

This reuses exact inspected register IDs R-TATOM-N1570 and R-TATOM-GCC without
duplicating their canonical definitions. No bibliography-only entry is credited.
The new missing mechanism was the literal branch-selected index specimen, not
the previously implemented release/consume pointer and fence operations.
