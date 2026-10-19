# Assessor: pathname identity and policy boundaries

The first live handle and its later hard-linked handle refer to the same original
inode even after both names are removed; the recreated `old` is a different
inode while the original is still live. Inode reuse after all references end is
outside this equality argument. A renamed directory descriptor remains an anchor
when `cwd` and the old pathname refer elsewhere. Absolute `openat` ignores dirfd.

The child/pipe handshake supplies an explicit counterexample schedule, not a
probability estimate. `fstatat` checked the original symlink resolution. After
the acknowledgement, the intentionally flawed second lookup reaches `OTHER`.
Final `O_NOFOLLOW` rejects the final symlink but follows `middle` while looking
up `middle/value`. The complete Linux policy rejects both, and rejects upward
and absolute escape. No retry silently removes a resolve flag. `O_NONBLOCK`
allows the FIFO to be rejected by the regular-file check without waiting.
The acquired descriptor is closed on every post-open rejection.

The hard-link counterexample is essential: one in-tree name refers to the other
synthetic inode without any symlink traversal. It satisfies the declared policy.
A learner claiming content trust or immutable bytes from resolution flags fails
even if every ordinary test passes. No mount/remote-filesystem proof is supplied.

The metadata API copies named fields, not a native struct wire image. The link's
length is the pathname stored in it; following it changes the inspected object.
Linux reports allocated blocks in512-byte units, unrelated to preferred I/O size.
The two actual filesystem-type records and requested/returned nanoseconds support
relational comparison, not a claim that every requested precision is retained or
that all filesystems allocate a sparse file identically. Concurrent stat fields
can reflect different times; the harness deliberately avoids concurrent changes.

The no-default-ACL child's0666 and027 produce0640, with the parent mask untouched.
The declared three-entry default0755 and requested0666 instead yield0644 even
when the model's umask is077. This is a restricted model, not a Linux ACL engine.
Search-only lookup does not grant file read/write. Parent write+search permits
entry replacement in the declared no-sticky/no-privilege model independently of
file write. Observe and label injectedEACCES, never misreport it as a host denial.

Mutation obligations:1 final-only repair misses intermediate links;2 unprotected
open misses final replacement;3 always-follow loses link identity;4 byte-size
mislabelled as allocated-block count;5 ignores default ACL;6 uses file-write for
replacement;7 reopens the replaced name instead of retaining original handle;
8 creates another file instead of a hard link;9 substitutes current directory
for the retained dirfd;10 treats an absolute path as relative. All must compile
before semantic rejection is evidence. The public syscall wrapper only wraps
the course reader's openat2 entry; injected errors test fallback policy, not host
kernel availability. Sanitizer crashes are not accepted mutation results.

Inspect the changed-condition explanations separately for all six original
criteria. The author run does not establish a learner waited seven days, worked
unaided, or received external review. Real-world authorization needs additional
credential, ACL, sticky-directory, immutable/read-only and concurrency contracts.
