# Assessor I/O transition analysis

At the scripted EAGAIN return, source position is7, committed count5 and the
buffer's pending suffix is `fg`. Retrying from buffer0 duplicates committed bytes;
advancing the whole buffer after a short write loses bytes. On hard failure,
the same suffix stays owned by the failed cursor and no more callbacks execute.
Do not conflate this state with a source that can be read again. Budgeted EINTR
is a fairness boundary, not EOF or progress. The real pipe case proves the
cursor survives actual readiness transitions as well as callback scheduling.

Unsigned extent checks first enforce selected ABI representability and then the
stable file bound. The zero-length maximum case is valid; maximum+1 exclusive
end is not. Neither conversion nor signed addition is evaluated before its proof.
Native5GiB sparse execution demonstrates the selected ABI, not unexecuted32-bit
support. Linux headers/libraries are necessary for that separate target probe.

The two disjoint workers use pwrite and never share a mutable seek cursor. In the
forced lseek case, the child changes the shared open description to800 after the
parent selected700, so the parent's `WRONG` bytes appear at800. Neither individual
call being valid repairs their composition. An eight-byte record split into two
writes exposes a mixed generation. The process record lock excludes a separate
reader until both halves are published; it is advisory, and every participant
must obey it. Losing the lock through another close on the same inode defeats
the argument. No persistent crash consistency follows from that lock.

Mutation1 restarts a pending suffix;2 forgets unwritten short-output bytes;
3 omits representability;4 omits selected extent;5 permits Linux O_APPEND;
6 substitutes write for pwrite;7 imports retained ownership into Linux close;
8 imports Linux release into the retained policy;9 implements pread with a shared
seek;10 replaces a sparse-hole byte with nonzero data. Each targets a distinct
causal claim and must produce an intended assertion after successful compilation.

The close model returns error independently of ownership. In the Linux case the
number is deliberately reused for a replacement file; a retry could close that
replacement. In the alternative model the first injected interruption does not
release the descriptor, so leaving ownership forever false would leak it. This
is model execution on Linux, not observed HP-UX or POSIX2024-kernel behavior.
The direct2024 standard page was403; keep that reference gap and attribute the
edition comparison only to the inspected Linux manual. Connect has separate
state/completion rules; do not claim this pure model implements socket recovery.
