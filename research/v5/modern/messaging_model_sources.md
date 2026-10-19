# Bounded GenMC source inspection

On2026-10-17 the author read the actual GenMC README, CMake configuration and
manual index, usage, features, command-line and API files at commit
9f6c4c0772d0c42b325681581acc6a0f3eb9b5f7. The private checkout and the executable
are pinned by `messaging_models_validation.json`. This is author inspection,
not an independent review of the model checker or its proof papers.

The read passages require finite, data-deterministic clients; identify RC11 as
the default language model; distinguish it from the IMM hardware-model
interpretation; describe assertion/race/memory-error reporting; and list supported
pthread/atomic operations. The fixtures therefore use finite original clients,
explicit RC11, atomic payloads and supported create/join operations. They do not
send the hosted condition-variable hub through an unsupported API stub. No state
space is restricted by a user `assume`, input randomness, scheduler bound or
explicit loop-unroll bound. The source-level clients themselves are finite and
small. A successful result cannot be extended to omitted clients or pointer
reclamation merely because the public API names are similar.

P0690R1 sections2,3.2,3.3 were reopened and read on2026-10-17. Its sequence example
uses release payload stores and an acquire fence after relaxed payload reads. The
course implements its own bounded C17 functions and supplies the C atomic-ordering
argument plus model counterexamples; the C++ proposal is not treated as adopted C
wording. The existing R-MOD-P0690 record remains the canonical identity. Attempts
to reopen the previously inspected Chase-Lev paper URL timed out; that does not
create a new inspection. The Boehm2012 full paper and published ISO C17 text remain
access gaps. Neither the bibliography link nor the reachable proposal's citation
to Boehm counts as reading that paper.

Private provisioning downloaded and extracted Ubuntu packages without installing
them or changing host security settings. GenMC was built with g++15 and LLVM21.1.8.
Two initial configuration/link attempts exposed missing private extracted libraries;
the final link explicitly searched the private library directory. The first upstream
smoke command used the driver's default RelWithDebInfo path, which did not match
the selected build directory; this was a tool-location failure, not a passing test.
The rerun supplied `GenMC=/home/frede/cmastery-modern-tools/genmc/build/bin/genmc`
and passed the real upstream fast-driver suite. Exact final observations are in
`messaging_genmc_toolchain.json`. No modified upstream source or relaxed test was
used. The private tool is verification infrastructure, never a target dependency.

A later same-day inspection reached the author's new host at
https://fzn.fr/readings/ppopp13.pdf and read the introduction's proof scope,
sections2/2.1 and Figure1; a subsequent refetch again timed out. This is a bounded
additional reading of the existing paper, not a claim that inaccessible earlier
requests succeeded. The author-hosted errata at
https://fzn.fr/readings/errata-ppopp207-le.txt was read in full and retrieved with
its content hash. It identifies a redundant ARM barrier, an omitted array reload
after resizing, and the array-publication dependency missing from the earlier
C11 translation. The latter can expose incompletely initialized array contents.
The corrected Figure1 uses the reloaded array and a consume load. Our fixed-array
weak-order model has no dynamic array publication. The separate growable owned
adapter protects publication and retention with a mutex; it does not inherit an
ARM proof or claim that its mutex can be removed by copying the printed fences.
