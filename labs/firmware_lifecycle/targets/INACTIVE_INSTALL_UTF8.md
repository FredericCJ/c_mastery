# FW-INSTALL: authenticated transport to an inactive H745 slot

Required PR-T59-01/03/04/05 continuation with the original objective and delayed
assessment identities. The B06 common contract and explicit FW-BOOT primitive
profile apply. Families A/B/C/D/E/F/G/H/I/J assess erase authority, program
completion and persistent old-image preservation. Prerequisites are actual
RTFrame admission, current BSSelection, guarded slot reads, FH256-bit programming
and fresh readback. No update/flash library implements the coordinator.

Implement inactive_install.h. A fresh FUInstall owns its state and a2144-byte
copy of the admitted envelope/padding/signature. Source RTFrame must be complete,
current-policy authenticated and immutable during begin. Old BSSelection is a
trusted result of actual stored-pair authentication, not attacker metadata.
Candidate generation must strictly increase and its version must not decrease.
The exact H745 bank1 inactive map is core0 sector1/2, core1 sector3/4, payload
at sector-base+512. Reject a candidate at the currently selected base before any
erase/program write. Bootstraps, journal sectors6/7 and trial sector5 are outside
this operation's erase authority. No whole-bank erase is permitted.

Admit controller quiescence, preserve qualified PSIZE and original lock state,
clear prior completion/error flags, issue selected sector erase, and wait for
new completion or error. Busy never means timeout cancellation. Restore control
only after quiescence; refresh the owned sector and verify every word is virgin,
at most32 reads per poll. Then program exactly67 fresh256-bit words once each:
header+payload at base-32, padding ones, signature at base+2048. Do not infer
completion from visible bits alone. Use the actual fh_start/fh_poll contract for
each word. After all completions, refresh2144 bytes and use the actual guarded
bs_read_image/signature/current-policy path. Require the same authenticated
digest, then publish the output. Pending/error/rejection must not publish it.
The copied policy is a snapshot; concurrent revocation or policy replacement is
excluded by the operation owner and must be a separate synchronization contract.

FH_DONE means an inactive authenticated image has been installed. It is not a
trial launch, application health acknowledgment or durable activation commit.
The old pair and its committed journal remain untouched. The caller may release
the input frame after begin copies it, but retains the FUInstall/callback context
until the terminal result. Partial hardware progress may remain after failure;
restarting requires a new erase, not overwriting nonvirgin ECC words. A poll
budget expiration retains hardware ownership and does not restore controls.

The linked original second-stage probe receives one UART frame, authenticates
the existing committed pair, installs only the requested inactive image and
reports status5 for installation. All code, constants and vectors reside in
owned bank208120000..0815FFFF; private data/stack use DTCM. This placement keeps
instruction/constant reads out of bank1 while bank1 is being erased/programmed.
The immutable M4 bootstrap08100000..0811FFFF is separate. A trusted cold bootstrap
must transfer to this second stage before peer release; normal boot selection,
maintenance trigger, durable trial marker and health-based commit are downstream
integration obligations, not claims of this probe. The runner never flashes it.

D1: derive region/word arithmetic, sector exclusion and the old-pair invariant.
D2: implement staged ownership and exact erase/program/readback completion.
D3: execute all536 individual32-bit program-store cuts and first/middle/final
selected erase-granule cuts; each leaves the exact old images/journal intact
and publishes no result. Test erase error, busy retention, active-slot refusal
and corruption after program completion but before signed storage readback.
D4: run six compiler/sanitizer profiles, reject active-slot and skipped-readback
mutants plus learner stub. D5: cross-link M7 O0/O2 second-stage images, audit
all code/read-only symbols and vector locations in bank2, helper closure, map
bounds and stack records. D6: integrate paired installation with durable trial,
application-health and commit/recovery; keep this unresolved until that separate
coordinator and its cut histories actually exist.

Level0 erases the active slot or calls installation a confirmed boot. Level1
programs good bytes without completion/readback evidence. Level2 satisfies
D1–D5 and names the remaining D6 contract. Level3 independently implements the
original delayed interrupted-install/peer-failure variant after seven days,
including failure with all visible desired bits but no completion acknowledgment.
Hints: erase authority is narrower than the whole memory map; separate hardware
completion from signature validity; do not execute from the bank being written.
Variants change sector geometry or move the writer into same-bank flash and
require a new placement/progress argument. Assessor analysis remains separate.

Security target is original synthetic signed images and the dedicated teaching
slot map. Attacker control is incoming bytes and modeled corruption/reset cuts;
a harmless violation is a mutant erasing the preserved active region or
publishing an image whose stored bytes no longer authenticate. No external target,
real key, option byte or deployed board is modified. Cut histories are digital
model transitions, not physical ECC/erase/power-loss behavior. Exact source and
commands are in inactive_install_validation.json. Link placement is inspected;
actual STM32 transfer, UART, voltage, retention, errata and power cuts remain
unobserved, and independent learner/reviewer evidence remains separate.
