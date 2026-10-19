# PR-T02-01 exact assessor analysis

The original15 criteria retain their ordinary and seven-day tasks. Reference
bodies are types.c; experiments.py generates original inspectable isolated TUs.
Public tests are executable interface constraints, not a substitute for the
criterion-specific derivations below. Grade0 absent/wrong,1 repeats supplied
example,2 derives and verifies the ordinary task,3 independently derives the
changed condition with its own counterexample and evidence limits.

## t0201a

`a` is an array of4 pointers to int; `b` points to an array of4 int; `cb` points
to a function taking int and void* and returning int. Legal uses include *a[0],
(*b)[0], and cb(7,context), assuming each referenced object/function is valid.
Assigning a to b is an incompatible-pointer constraint, not a stride correction.
The transfer declaration is `int (*factory(void))[4]`: call first, dereference
the returned pointer, then index a four-int array. The reference returns a static
row; a local automatic row would invalidate the result on return. A successful
call says nothing about arbitrary pointer validity. Require the lifetime argument.

## t0201b

`extern int x;` declares without defining; file-scope `int x;` is tentative;
`typedef int Count;` is an alias declaration with no object storage. A prototype
declares a function, a body defines it. Without another definition in that TU,
the tentative object behaves as zero-initialized at its end. Actual -fno-common
objects show B x versus U x; extern-with-initializer shows D x. Combining the two
defining objects gets a link refusal in the selected toolchain. A -fcommon or
permissive linker behavior would not repair the ISO C one-definition obligation.
Require translation-unit boundaries and distinguish terminology from nm letters.

## t0201c

`typedef int Unary(int); typedef Unary *Pointer; Pointer factory(void);` is
compatible with `int (*factory(void))(int);`. Both call results are pointers to
int(int). The compatible redeclarations and actual return/call test demonstrate
the selected construction; expand the aliases in the reasoning. TdCallable is a
different structure type, so `factory().call(7)` replaces direct invocation.
Trying to call the wrapper itself is rejected. The wrapper still can contain null
or a miscast function pointer; ISO C has not supplied a safe callable abstraction.

## t0202a

Opaque structure: incomplete object, aggregate; completed Pair: complete object,
aggregate; int[4]: complete object, aggregate; void: incomplete object type that
cannot be completed; int(int): function type, not an object or scalar/aggregate.
An opaque pointer is itself a complete scalar object type. sizeof Pair and the
array are legal, sizeof incomplete structure/void/function violates constraints.
The two independently compiled opaque consumers need only pointers and prototypes;
only types.c defines layout and performs member access. Both consumers deliberately
refer to the same static owner0, while owner1 must stay a distinct object. Neither
layout hiding nor pointer constness prevents a forged pointer or data race.

## t0202b

For function type compatibility ignore top-level parameter qualifiers: const int
versus int gives compatible parameter types. Pointee const remains part of the
pointed-to type: int* and const int* parameter types are incompatible. In one TU
the conflicting declarations receive diagnostics. Independent conflicting TUs can
link because symbol resolution may lack that type information; do not execute the
resulting call or infer validity from equal pointer representations. Signed and
unsigned callback parameter types are incompatible too. The cast conversion is
permitted, but calling the wrong function type is undefined under N1570 6.3.2.3p8.
A real numeric adapter must first reject negatives or otherwise specify conversion,
then invoke through the true unsigned signature. ABI coincidence is insufficient.

## t0202c

The typedef speed(t,d) call compiles and computes0 for3/12 even though the intended
units are wrong. Distinct TdMeters/TdSeconds structures reject the swap. Runtime
checks still enforce nonnegative bounded values, positive duration, exact division
and bounded multiplication. The reference tests12/3=4, rejects13/3 without changing
output, and rejects1000*2 before multiplying. A truncation mutant must fail. Unit
composition requires a declared rounding/range model; structures alone cannot
prove physical meanings or prevent explicit reconstruction with the wrong member.

## t0203a

Ordinary identifier scope generally begins after its declarator; therefore the
inner x in `int x=x;` already names the uninitialized inner object. Do not execute
it to discover a guessed inherited value. Labels have function scope, file objects
file scope, block locals block scope and prototype-only parameters expire at the
end of the declarator. A tag first introduced in a prototype has only prototype
scope. A later file-scope tag with equal spelling is distinct, making a supposedly
matching definition conflict. Forward-declaring the tag at file scope repairs
identity. Require the exact entry/exit points, not just namespace labels.

## t0203b

Tags, labels, ordinary identifiers and each structure/union's members occupy the
specified separate namespaces. The positive example simultaneously names a tag,
local, label and two members `name`; each expression context selects its binding.
Typedef names belong to ordinary identifiers, so typedef int name and int name
conflict in one scope. A negative example in a different scope would instead test
shadowing and would not establish this criterion. Require both controls.

## t0203c

The shadow specimen is defined: the outer status stays0 while the inner7 expires.
A -Wshadow diagnostic helps locate the binding but suppressing it does not assign
the outer variable. The callback repair receives a context containing &status;
its store changes that live caller object. This is explicit environment passing,
not an implicit language closure. A pointer to expired caller storage would remain
wrong even with the correct callback signature. The delayed answer must identify
the exact captured storage and duration; for deferred execution use the job contract.

## t0204a

File static: internal linkage/static duration. External object: external linkage/
static duration. Block automatic: no linkage/automatic duration. Block static:
no linkage/static duration. Symbols alone do not expose every optimized automatic
object. Actual O0 objects and repeated f calls5 then6 demonstrate the block-static
state. Left/right TUs both have internal state3, but their returned live addresses
differ and setting left9 leaves right3. Equal spelling does not join entities with
internal linkage in different TUs. Require storage and linkage as separate columns.

## t0204b

An extern with a visible prior internal-linkage declaration inherits it. A hidden
prior declaration or visible no-linkage declaration instead leads to external
linkage; the constructed single-TU internal/external use of the same identifier
has undefined behavior by6.2.2p7. Compiler acceptance/rejection is observed only;
there is no runtime golden output or mandated diagnostic for this case. register
does not promise a physical register and forbids address-taking under C17.
_Thread_local creates distinct thread-duration objects, introduced in C11. The
worker sees3 then9 while main retains7. The static result carrier is read after
successful pthread_join; POSIX4.15.2 supplies synchronization. Thread storage itself
is not synchronization and no pointer to expired TLS is compared after joining.

## t0204c

The independent double(double)/int(int) pair links in the recorded ordinary builds
but remains invalid; it is never executed. A common int(int) header makes the
provider mismatch visible to each compiler; the repaired pair runs. GCC LTO with
-Werror=lto-type-mismatch rejects this concrete mismatch and accepts the repaired
control. Do not claim every cross-unit type or aliasing defect is detected, nor
infer a Clang LTO result from a GCC run. Header discipline improves visibility;
runtime preconditions and whole-program behavior still require separate analysis.

## t0205a

The matrix callback has a complete prototype and an explicit status result. The
ordinary tests distinguish empty no-callback success, invalid callback preservation,
and a bad final element rejected before any callback. Nonempty success calls all
elements row-major and sums44 for the supplied1..8 matrix with bias1. Numeric
failure atomicity before execution does not imply rollback of arbitrary callback
effects. In C17 an empty nondefinition list does not specify parameters, and the
separate int(int) definition used by the pilot is compatible after default
promotions. N3096 6.7.6.3p13 instead treats the empty list as void. Actual C23 mode
and __STDC_VERSION__ are capability observations; the one-argument caller rejects
there, while the no-argument positive control runs. This does not certify all C23.

## t0205b

`int a[4]` adjusts to int*, so sizeof a measures that pointer; `int (*a)[4]`
keeps a pointer-to-row type and sizeof *a measures4*sizeof(int). Their call sites
use a versus &a respectively. Record actual sizeof values without assuming an
ABI for all targets. `int a[static 4]` still adjusts to a pointer but imposes a
minimum-access precondition at every call. Passing a shorter object is outside
the contract even if a particular execution avoids its last element. The valid
four-element control runs; no unsafe short-array execution is required.

## t0205c

The wrapper calls td_step(value,context) using its real signature. Reinterpreting
the function pointer does not reorder machine arguments or make the types
compatible. The private cast specimen is compile-only; any optional diagnostic
policy is separate from the language verdict. The delayed job retains a borrowed
context across submit and later run. Pending cancellation clears the borrow and
acknowledges quiescence; a canceled job never calls it. While RUNNING, reentrant
run/cancel/reset report BUSY, so the owner cannot reclaim context yet. The outer
run clears pointers and publishes DONE only after return. The cancellation mutant
falsely says OK while leaving the job runnable and must fail. This is serial
deferred execution, not OS cancellation or a thread-safe scheduler. A longjmp,
concurrent mutation, wrong callback return bound or forged public state violates
explicit prerequisites that ISO C does not enforce for this representation.

## Independent assessment and limits

After at least7 days provide the original delayed task and public contract only.
Withhold these solutions, generated private specimens and oracle. Require a fresh
prediction, alternative declaration/consumer and counterexample before execution.
Record learner identity, prior/new source hashes, dates, permitted material and
criterion-specific result; author runs are not learner attempts. Alternative
representations are valid only when the public ABI, static ownership model,
admission/failure rules and stated cost bounds hold. No finite campaign proves
absolute mastery, arbitrary pointer validity or conformance across all editions.
