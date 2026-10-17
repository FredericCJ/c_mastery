# Error models and executable contracts

Implement the nine T65 contracts in `src/errors.c`. The compiling stubs reject
the semantic gates. `include/errors.h` fixes public types and operation names;
read `CONTRACTS.md` and the selected canonical exercise before coding.

Configure with CMake, build, then run CTest. Python is host-side verification
only. Assessor implementations are selected only by an explicit absolute
`CMASTERY_ASSESSOR_ROOT`; learner initialization never copies them. The assessor
configuration adds two compiled, property-specific mutations per mechanism.

The nonlocal-jump project uses the host C library. Do not import this family as
a strict freestanding runtime. Report the tested environment and retain the
unexecuted independent, delayed and portability obligations.
