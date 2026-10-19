# Assessor analysis: six T05.02 tasks

For a and a-DELAYED, check b's sign before forming `INT_MAX-b` or `INT_MIN-b`.
When b>0, the first bound is representable; when b<0, the second is representable.
Only then is a+b admitted. The output must remain77 after a refused oracle call.
An assignment before the check is a contract violation even if the status is
correct. Old objects use explicit int declarations and admitted default-promoted
argument types. Separate object construction preserves that premise; a name
in a symbol table conveys no parameter signature. The shared header makes the
deliberately conflicting return declaration visible in one translation unit,
so the observed diagnostic has a specific intended cause.

For b and b-DELAYED, distinguish the pre-C23 nondefinition empty declaration
from a definition with an empty list. The supplied float argument promotes to
double before the unprototyped call; the actual callee has a compatible double
parameter. With a double prototype, conversion is directly checked. Under the
selected C23 rule, empty parentheses mean no parameters and the argument is a
constraint failure. The positive prototype control ensures a generic compile
failure is not being mislabeled a changed language rule. An incompatible call
can link successfully and still have undefined behavior; never execute it as
an expected-output specimen. The Clang strict-prototype warning policy is
recorded independently from the language reasoning. Published exact editions
remain inaccessible and neither draft is relabeled.

For c and c-DELAYED, `(a>b)-(a<b)` is limited to -1/0/1 regardless of key
extrema. The correctly typed adapter receives actual record object pointers;
its conversion is distinct from casting and calling a function with the wrong
type. Reversing the bounded result is safe. The insertion invariant is that
the prefix is sorted, equal-key serial order is retained, and remaining records
are not lost. Move only strictly greater records in the selected ordering.
Copying the context before mutation and borrowing it synchronously avoids
retaining a caller pointer; no claim extends to concurrent mutation or queued
callbacks. The qsort check assesses signature integration, not qsort stability.

The independent oracle deliberately differs from insertion sort and C bounds
arithmetic. Check all 39,062 enumerated sort rows and65,618 sum rows, plus the
separate maximum-length and invalid-admission anchors. Each named mutation
must fail after successful configure/build. Address/undefined-sanitizer success
does not prove API premises, compatibility across all translation units or all
integer widths. The author campaign and assessor implementation are not learner
assessment; score D1-D4 separately using the public rubric and fresh variants.
