# B06 common exercise and evidence contract

This contract applies to every PR-T57–PR-T64 project and its LAB unit. Each
project supplies the mechanism-specific inputs, API, invariant, boundary and
changed-condition assessment; these obligations supplement its public tests.

## Prerequisites and permitted dependencies

Use the accepted C17 implementation baseline. C11 draft clauses supply precisely
identified semantic reasoning; a draft is not a substitute claim that published
C17 was inspected. C23 comparison work must identify the actual changed clause
and compiler support; unchanged ABI or OS behavior is not an ISO C guarantee.
Before entry demonstrate checked extent arithmetic, object lifetime, byte
decoding, resource ownership and reproducible build capture with the declared
prerequisite artifacts. Reading a module does not accept an artifact.

The supplied portable cores use only C17 types and explicitly declared host
math operations. Implement the mechanism from these primitives: no allocator,
compiler, container, verification engine or protocol library may replace the
mechanism being taught. Host-side Python/CMake/CTest are verification tools.
Kernel headers, system calls and ISA assembly belong to separately named
adapters. A hosted state model does not demonstrate kernel scheduling, device
ordering, cache coherency or flash persistence. For each freestanding adapter
compile with -ffreestanding, inspect relocation and undefined-symbol tables,
linker maps and disassembly for compiler helpers; report every dependency and
provide or remove it. Merely disabling builtins does not prove runtime closure.

## Execution and containment

Run host tests in a disposable working directory using synthetic fixtures.
Kernel modules run only in a disposable VM with a snapshot, no host shared
directories, no bridged networking and no production data; do not insert them
into the workstation kernel. Firmware work uses the supplied model/emulator
first. Physical transfer is restricted to dedicated NUCLEO-F439ZI or
NUCLEO-H745ZI-Q development boards and requires a separate authorized session;
this authoring task does not flash devices or change security settings.
Security cases target only the original teaching arena/dispatcher. The attacker
controls the explicitly supplied bytes, handles or event schedule, not real
credentials. Harmless success is the named sentinel/property transition.
Crash, undefined behavior and sanitizer detection are different observations;
none alone demonstrates code execution or privilege escalation.

## Deliverables and verification

Retain (D1) a prediction before execution, pre/postconditions, representation,
invariants, ownership transitions, failure atomicity and cost derivation;
(D2) learner source and build identity; (D3) complete commands, raw outputs,
tool versions and independently derived expected results; (D4) a minimized
counterexample, repair and regression; (D5) a changed-condition implementation
and comparison; (D6) an evidence-limit statement. Use supplied fixture data,
then independently construct at least one input that distinguishes your repair
from the named plausible wrong implementation. Public finite checks cannot
establish absence of all defects. Sanitizers observe executions; bounded proof
depends on assumptions/unwinding; measurements depend on workload and machine.

## Assessment rubric and hints

Every objective has its own criterion in the project and authored unit. Score
0 for no assessable artifact or a contradiction of its contract; 1 for a
passing demonstration without a causal explanation; 2 for an original
implementation/analysis with a correct invariant, boundary argument and
reproducible evidence; 3 for an unaided delayed changed-condition solution
that identifies which previous proof or measurements no longer apply.
Require all objectives at level2 before technical completion and level3 on the
specified transfer task after at least seven days before independent-transfer
completion. Record whether the attempt was self-reported or externally
reviewed. No fabricated reviewer independence, mastery certificate or absolute
mastery claim is permitted. Hints are graduated in each brief; consulting the
assessor solution changes the attempt to assisted practice.

## Validation states

Canonical writing means authored; a compiling scaffold means executable; a
source-bound observed run means executed. These do not imply learner acceptance,
external assessment, VM execution or physical validation. Each family records
remaining target evidence and each campaign reports exact commands. A model
pass cannot satisfy a missing mandatory adapter investigation. New descriptive
hours are separate from accepted baseline hours and impose no learner-time cap.


## 1: Bounded lexer with exact progress

Implement cc_token over a nonterminated byte span. Skip ASCII space/newline/tab; recognize decimal0..65535(kind1), x(2), ((3), )(4), +(5), *(6), and end(0). Return1 on token/end,0 on invalid or overflow. On failure preserve offset/kind/value. On success set value0 except constants. Do not call strtol or a lexer library.

Derive the pre-multiply bound and exact consumed extent, including an unterminated span. Explain why end-of-input is a successful token distinct from rejection.

## 2: Precedence parser and bounded syntax construction

Implement cc_parse from the declared expression grammar, generating postfix CONST/ARG/ADD/MUL followed by RET. Multiplication binds tighter than addition; parentheses override. Limit nesting to16 and IR capacity to caller storage. On failure used remains unchanged; partially written IR scratch is unspecified and must never be executed. Reject every unconsumed nonspace byte.

Prove each parser level consumes its grammar production or rejects, and derive linear token work with bounded recursion. Explain why no partially constructed IR is publishable.

## 3: Subset type and arithmetic semantics

Implement cc_binary(op,a,b,out) for op3 addition and4 multiplication modulo2^32, using uint64_t intermediates and explicit conversion. Reject other opcodes without changing output. Identify the host assumptions required by uint32_t/uint64_t and compare with the C unsigned-return wrapper.

Derive modulo behavior independently of host signed overflow; identify absent signed conversions, floating-point rules and pointer semantics rather than pretending they are covered.

## 4: Typed intermediate representation and interpreter

Implement cc_run over verified CCIns with a local64-entry uint32_t stack and one x argument. CONST and ARG push; ADD/MUL consume two and push one; RET must be last with exactly one value. Reject malformed streams and preserve output. Use cc_binary for semantic arithmetic, not native signed expressions.

Associate every opcode with input/output type and stack effect; prove validated execution cannot underflow/overflow its bounded stack and returns the expression denotation.

## 5: IR verification and resource bounds

Implement cc_validate for1..128 instructions, at most64 live stack values, constants<=65535, opcodes1..5, no underflow, and exactly one final RET with height1. Return peak depth only on success. Reject any operation after RET and any missing RET.

Derive prefix-height and terminal-height conditions and peak stack bytes separately for the two backends. Explain the verifier assumptions that remain outside its scope.

## 6: Original x86-64 code emission

Implement cc_x86 emitting NUL-terminated GNU assembly for cm_expr(unsigned x) from valid IR using edi input, eax result, rcx temporary and64-bit push/pop storage. Use bounded staging so too-small output capacity leaves caller bytes unchanged. No LLVM, compiler library or template compiler may replace emission.

Map each IR stack transition to emitted instructions, prove balanced machine stack and identify all clobbered registers. The textual anchor is weaker than assembling and executing the result.

## 7: Original Thumb code emission

Implement cc_thumb emitting Thumb-2 GNU assembly using r3 to retain x, r0/r1 temporaries, word pushes/pops and bx lr. Constants fit movw. No nested calls or FP operations are supported. Stage output transactionally and preserve the exact same unsigned32 semantics as x86.

Prove the private operand stack remains word-aligned and returns to its public8-byte boundary. Identify the Thumb-state return and why r3 would not survive a newly added call.

## 8: Differential compiler validation and translation limits

Implement cc_compile_eval(source,n,x,out) composing parser, verifier and interpreter with bounded128-instruction storage. Preserve output on any failure. Generate valid expression trees independently, render source and mathematical denotation, then compare interpreter and both emitted targets. Minimize a mismatch while retaining its semantic predicate.

Separate front-end correctness, IR semantics, backend correctness and target environment. Define the supported source subset before interpreting an oracle disagreement.