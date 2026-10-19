# PR-T62-07 cross-directory update and abrupt guest termination

Required families B/C/D/F/G/I/J; all T62.07.a/b/c objectives. Use the B06 shared
contract, prerequisite learner file-progress and ownership artifacts, and the
file-versus-directory reasoning from DIRECT_IO.md. The supplied `durable.h`,
stub and probe define a Linux C17 filesystem adapter. POSIX file APIs are permitted;
no database/transaction library may supply its mechanism. Two directory fds refer
to distinct owned directories on the same filesystem. Names are fixed `tmp` in
source and `target` in destination; no untrusted pathname traversal is involved.

Implement `du_replace` with a live immutable0..4096byte payload, valid disjoint
phase output and a non-reentrant optional observation hook. Boundaries are:
1 create source/tmp exclusively and write all bytes, retrying EINTR;
2 fsync its file; 3 rename over destination/target; 4 fsync source directory;
5 fsync destination directory. Return0 only after all operations including close
succeed. Return negative errno otherwise, with phase equal to the last completed
boundary. An injected failure immediately before1..5 returns EIO without doing
that operation. This teaching argument is trusted test control, not external data.
Reject invalid size/failure index before any filesystem mutation. All opened
descriptors close on error. A failed pre-rename attempt can leave owned tmp for
explicit recovery; after rename, do not pretend returning an error rolled back
visibility. The caller owns orphan cleanup and must not retry blindly over tmp.

D1: implement the protocol and state table. Before rename, the old destination
is unchanged; after rename the new bytes are visible but durable commitment
is not acknowledged until both directory syncs succeed. D2: inject all five
failures and compare the actual target and phase. Wrap fsync for observation,
record the file/source/destination inode identities and reject an implementation
that omits the destination sync even if a later recovery happens to show new data.
Cost is O(payload) work and constant adapter storage; filesystem writeback,
journal operations and device latency remain environmental costs.

D3: execute `python3 tools/run_b06_durability.py`. It creates a private64MiB ext4
image, prepares an old4096byte pattern and shuts the preparation guest down
cleanly. For each phase it copies that private image, boots the original update
program and kills only its own QEMU process immediately after the exact phase
marker. A fresh guest mounts the same image and checks recovery: phase5 must
contain the entire new pattern; earlier cuts record old or new, never silently
accept a hybrid or missing target in this selected ext4 experiment. This is an
observed ext4 recovery property under the recorded configuration, not an ISO C,
POSIX or universal Linux crash-atomicity claim. The pattern oracle is i mod251
for old and (i+17) mod251 for new, checked across every byte and EOF.

QEMU runs without networking and with only the newly created image attached.
There is no host mount, reboot, module insertion or physical device access.
Abrupt guest termination leaves host/device caches intact; cache=writeback is
recorded. Physical power interruption, lost host cache and a lying device's
flush semantics remain outside this evidence. Reproduce those only on a dedicated
authorized rig; do not claim this guest experiment is equivalent.

IO10 rubric: a requires D1's visibility/durability and failure proof, b requires
D2's implementation/fault assertions, c requires D3's recovery traces and exact
limits. Level0 equates rename/O_DIRECT with durable commit; level1 only shows
successful reopen; level2 meets all three obligations plus the missing-sync
negative control; level3 independently adapts to cross-filesystem EXDEV or an
unseen post-rename error after at least seven days. EXDEV requires a newly
specified copy/publish protocol, not replacing rename with unchecked copying.
Hints: write the phase/state table first; count distinct directories; distinguish
a visible new file from a committed durable name. Separate technical analysis:
`assessor/linux_io_depth/targets/TRANSFER_ANALYSIS.md`, IO10.

Source-bound author evidence is `durability_validation.json`. External review,
independent delayed learner performance and physical persistence remain separate.
Primary inspected FSYNC and EXT4-JOURNAL source locators are in the B06 followup
register. Preserve all observed failures and limitations when changing backend.
