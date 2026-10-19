# PR-T03-02 withheld technical analysis

The public contract and the retained nine exact rubrics apply. Reference outputs establish finite author observations only. The core imports actual foundation functions; this is not a second implementation passed off as integration.

## T03.04.a

`matrix` converts to pointer to its first row, an `int[3]` object. Admit row<2 before forming its row address and col<3 before its int element. Values at the two valid original coordinates are1 and6. A separately allocated representation needs two actual pointer objects plus independent live row extents; allocation order has no logical-order meaning. Missing one release on failure or retaining a row pointer after free fails the changed task.

## T03.04.b

Offset5 is a useful mathematical linearization but `matrix[0]` has the first three-int row as its arithmetic domain. Do not execute `matrix[0]+5`. The whole-object character view derives from `&matrix`, not merely from the first-row pointer; it can inspect `sizeof matrix` bytes without treating them as six ints reached by the original row pointer. Member/endianness/padding observations remain distinct from a wire representation guarantee.

## T03.04.c

The table accessor first reads a real `int*` object; a nested array contains ints at that location. A cast cannot create the absent pointer objects. The common `OcMatrix` dispatch stores a real context pointer and a compatible callback, which applies the representation's own checks. Withhold a shortened row and test that the outer rectangle alone cannot authorize its final column. The negative translation demonstrates the ordinary uncast mismatch, not every possible warning after explicit casts.

## T03.05.a

Foundation `fd_take` checks position<=length, then count<=length-position before addition. Null with zero length is valid only with position0 and no null arithmetic. `fd_subview` checks offset and length against remaining space and computes needed values before publishing fields; exact parent aliasing is supported. The compiled foundation mutant changes position on rejection, so the failure-preservation check must reject it.6,435 independent enumerated intervals also test the same actual imported producer.

## T03.05.b

End is an admitted pointer formation/equality state, never a dereference. Empty interior ranges likewise provide no element despite their nonnull base. Reverse traversal subtracts count from the already validated integer position and forms only a nonnegative in-array/one-past result. An implementation which decrements first and repairs later is invalid even when its final value looks right. The reverse mutant destroys position on failure; select count3 at position2 to distinguish it.

## T03.05.c

Both subtraction operands must designate the same actual array or its end, and their difference must fit `ptrdiff_t`; `size_t` can represent capacities that exceed the positive difference range. The integer magnitude guard precedes actual subtraction. Logical fragments `{2,0,3}` give both `(0,2)` and `(2,0)` offset2; comparing local indices reverses/equivocates cases and the compiled mutant fails. The overflow metadata example is safe because it forms no corresponding physical pointers. Relational rules also include stated aggregate/member cases; do not overgeneralize this component's narrower same-array contract into the entire ISO rule.

## T03.06.a

Both constant-expression0 and `(void*)0` initialize null pointers. A stored variable value0 is not an integer constant expression; its implicit pointer assignment violates a constraint. An explicit integer-to-pointer cast is a different, implementation-defined case. `memset` zero bytes is not a portable null construction. Nullable wire indices encode an application identity within a particular live owner, not a saved address. The all-ones sentinel is outside valid indices even at count`UINT32_MAX`. Valid foreign element pointers may be compared for equality, but dangling/one-past/forged candidates are excluded; equal representations cannot establish provenance.

## T03.06.b

Object-to-void-to-original equality is an ISO rule. `uintptr_t`, when present, provides the specified valid void-pointer roundtrip; it does not authorize arbitrary integers or preserve an ended object's lifetime. The changed task copies7 into a distinct still-live allocation, records the original integer before free, clears its pointer and prints only integer history plus the fresh pointer. The history is never converted back. Reject explanations that use post-free pointer comparison even when a sanitizer stays silent.

## T03.06.c

The intermediate differently typed function pointer can be converted back; only the restored compatible type is called. ISO's object/void rule does not supply the function-address loader rule. The local Linux experiment uses `dlsym` and explicitly checks `dlerror` after clearing it. GCC strict diagnostics and Clang acceptance are actual different observations; silence is not ISO authorization. The typed-table successor removes the object/function conversion from interface lookup but still depends on trusted native ABI and module execution. The table's complete object extent is a premise, not proven by reading its self-reported size.

Two borrows force two returns before close. Callback invocation cannot reenter/close/mutate the owner. Refusal leaves the handle live; an injected close error retains it for retry, including failed-admission cleanup with no usable table. Neither copied borrow variables nor this plain struct are unforgeable capabilities. `RTLD_LOCAL` is not a sandbox; successful close does not guarantee unmapping, and retained addresses never authorize calls after the owner releases them. CLOSE is a harmless false-success mutant, so it can be rejected without actually unloading a live callback. RETURN omits consumption, exposing the affine accounting requirement.

## Scope and independent transfer

The public target roots all core operations and the actual producer; final images must show no undefined runtime helpers, while the full provider object may contain unused other mechanisms. An injected unresolved allocator demonstrates the link boundary is active. This is not target execution or a startup/board contract. Native/i386 execution changes pointer/data-model observations but does not exhibit every implementation-defined representation.

For independent reassessment, retain the original delayed identity and give ragged rows, adjacent empty fragments and a plugin owner shared by two consumer adapters. Require a fresh proof of the borrower protocol and explain why a malicious table is still excluded. Do not replace the original changed tasks or infer learner mastery from the supplied reference, mutations or finite enumeration.
