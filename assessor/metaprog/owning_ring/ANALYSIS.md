# Assessor analysis: T68.01 ownership transfer

The decisive boundary is clone completion, not allocation success. On either
failure point staged storage remains a valid argument to drop; publishing its
pointer or count early breaks the ownership proof. After successful clone, tail
assignment, clearing source ownership and count publication are nonfallible under
the serialized callback contract. An empty pop has no destruction obligation.

FIFO clear is observable even if all allocations are eventually freed. The
expected trace after pop1/push5 is 1,2,3,4,5, with the first destruction performed
by the caller. Reverse clear is a semantic defect, not a leak. The erased version
must move out of real typed storage and empty its scratch before another push;
casting a char array is not an equivalent strict-C storage argument.

For the delayed two-resource owner, initialize both pointers null, acquire first,
then second. Failure of the second leaves only first owned; drop accepts that
partial state and releases first exactly once. The ring itself remains unchanged:
the callback's expanded failure graph requires no early publication in the ring.
Require an actual second-resource failure test before awarding delayed mastery.
The supplied Twin instantiation actually injects all four boundaries before/after
each allocation, then transfers/destroys a successful two-resource owner. That is
author support; it does not establish an unaided learner attempt.

MPO controls1/2 omit partial cleanup/publish count on failure;3 reverses clear;
4 stalls erased head;5 clobbers empty output;6 invokes callbacks on full. Retained
MOD1/2 break tail/head. Each must compile and fail its named ordinary assertion;
signals or sanitizer diagnostics are not an accepted substitute.

The C associated-type assertion only proves the selected declared element type.
It does not prove matching erased operations or resource laws. A normal include
guard suppresses a second definition; declaration-only clients still compile,
then fail at link. A private helper collision is diagnosed before substitution.
Multiple clients use the public header; including implementation definitions in
both creates external-linkage duplicates. Rust associated types constrain method
signatures, while resource behavior still needs its appropriate ownership and
trait contracts. Record actual compiler diagnostics, not predicted success.

The exact learner contract is labs/metaprog/owning_ring/OWNING.md. Its cost review requires
erased caller storage and typed stack staging to be counted. Compare one typed
Blob instance with handwritten Blob under equal callbacks before measuring the
second Words specialization. Tiny local timings are observations, not a universal
speed ranking or learner mastery proof.
