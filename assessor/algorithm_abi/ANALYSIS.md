# Private analysis: PR-T36-02

This is author technical analysis, not independent review or a learner result.
The original six criterion objects and delayed V1 identities remain unchanged.

## T36.03.a and its Microsoft transfer

The original x86 and Thumb sources independently encode the same half-open
invariant. Empty reaches the return label without reading the pointer. All
positive return values including n are indices, not automatically valid load
indices. Native tests compare actual `algo_lower` from the frozen original
C source as well as a linear oracle. The Python oracle independently checks
partition certificates and `bisect_left` results. Thumb actually executes
the original kernel under MPS2 and uses linear expected values.

Microsoft leaf: RCX=array, EDX=count, R8D=key; R9D=hi, EAX=lo, R10D=mid.
The three home stores at8/16/24(%rsp) use caller-provided slots beyond the
return address. No RSP or nonvolatile state changes; no nested call means
no nonleaf unwind model is being smuggled into the leaf. Compiler-generated
`ms_abi` calls exercise the convention in Linux ELF. This does not test a
Windows loader, structured exceptions or all aggregate/vector conventions.

## T36.03.b and callback transfer

In the baseline SysV probe, RDI/ESI/EDX carry the three arguments, R11 the
selected target, and stack slots0/8/16 preserve output pointer, target and
stack identity. RBX/RBP/R12–R15 are seeded. Six pushes and40bytes make an
88byte adjustment from entry; nested entry RSP mod16=8. Its extra8 before
the indirect call restores caller alignment. R8/R9/R10/ECX are deliberately
scratch; R11 is live only until the target call. After return, R10 is reused
for the recovered output pointer, EAX result is stored before EAX becomes
the probe verdict. Saved seeds and RSP are checked, then caller state restored.

Thumb probe pushes36bytes (r4–r11,lr) and reserves20, total56, preserving
8byte alignment. It stores output/target/SP, seeds all declared preserved
integer registers, then the nested function pushes8bytes (r3,lr). r12 carries
the target and r3 is poisoned. BLX replaces LR, so each nonleaf saves its
own return path. r9 is preserved under this explicit platform choice; AAPCS
does not universally assign it the same role for every platform.

New SysV callback kernel: RBX=array, R13=hi, R14=lo, R15=key, RBP=callback,
R12=mid, context at0(%rsp). Six pushes plus24bytes make72bytes; entry8 mod16
becomes0 before CALL. RDI=context, RSI=array, EDX=mid are reloaded each time.
Only EAX callback result is trusted after return; the branch uses preserved
key/midpoint. All six saved registers and stack are restored.

New Thumb callback: r4=array,r5=hi,r6=lo,r7=key,r8=callback,r9=context,r10=mid.
Push36+reserve12=48, so fifth argument at originalSP is read at newSP+48.
r0=context,r1=array,r2=mid feed BLXr8; r0 is the only returned value. r12/r1–r3
are poisoned and cannot store live search state. Saved LR returns through
the final pop. The intentionally flawed variant returns poisoned scratch
instead of preserved lo. Its wrong value is observed without an invalid load.

## T36.03.c and changed probes

The ABI mutant zeros EBX/r4. A returned index may still be correct; the
baseline probe separately fails preservation and restores its own caller.
The extra-load mutant reads a[n] at the return label. Empty input therefore
also faults, whereas a correct empty call performs no read. Two independent
children per layout distinguish empty and final-element inputs; each of two
layouts ends immediately before a private PROT_NONE page. The harmless success
condition for this flawed target is the expected contained SIGSEGV. It is no
evidence of arbitrary execution or privilege gain. The corrected source
returns normally for all four children. C sanitizers do not instrument these
assembly loads; the deliberate fault campaign is unsanitized and isolated.

Generated changed probes replace saved-register seeds and corresponding
comparisons together, plus caller-scratch poison; no preserved register check
is removed. Callback contexts also change poison values. A wrong scale8 load
is tested on a logical two-element array backed by three real words, so the
counterexample is a wrong index, not an accidental fault masking its cause.

## T36.04.a and upper bound

For key7, lower-bound certificate is [0,5) mid2→[0,2), mid1→[0,1),
mid0→[1,1); return1. Upper bound makes equal keys move right: [0,5) mid2→
[3,5), mid4→[3,4), mid3→[3,3); return3. On a nonempty interval mid lies
inside it; hi=mid removes mid and its right region, while lo=mid+1 removes
mid and its left region. Both decrease natural-number hi−lo. Partition
predicates switch from < / >= to <= / > in the upper variant.

## T36.04.b and eight-byte records

With n<=1024, mid<n makes 4*mid at most4092. The result n is never loaded.
The record variant preserves `size_t` bounds and midpoint, rejects n above
SIZE_MAX/8 before accessing data, then uses scale8. Native size_t is64 and
Thumb32; the code does not truncate a valid64bit n to a uint32. `n<=capacity`
is a caller promise about a real allocation, not proof that storage exists.
The enormous n test is admission-only and preserves output. Both target
widths actually execute the rejected maximum-size_t case. Empty permitsNULL.

## T36.04.c and equal-contract costs

One element0x80000000 and key0 is enough: unsigned comparison puts the result
at0; a signed branch treats the element as negative and incorrectly returns1.
No smaller nonempty counterexample exists. The signed mutant executes that
specific discriminator. The five-element fixture costs4 sortedness checks
and3 search decisions. The C certificate and original C oracle each perform
their own work for verification, not as part of assembly kernel timing.

For k queries over an immutable n-element array, validate once at n−1 checks,
then charge each logarithmic search; compare an untrusted one-shot API with
validation included on both sides. Mutating or freeing the array invalidates
the trust; a Boolean flag is no authentication or lifetime proof. Counts are
source-level comparisons, not cycles. No universal speedup is asserted.

## Assessment and source limits

For every leaf, require its concrete derivation and changed assumptions in
addition to passing tests. Withhold this analysis and implementations from
exports. Preserve failed runs and exact tool/source/log hashes. Native
GCC/Clang optimization and sanitizer profiles, Arm0/2 images, generated-code
inspection, page guards, intended mutants and compiled-stub failures are
author evidence only. Physical board/timing and independent delayed learner
performance remain unassessed. ABI drafts/vendor pages are separately named
from the unavailable final published C17/C23 standards.
