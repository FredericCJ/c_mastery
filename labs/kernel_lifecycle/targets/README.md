# Disposable-VM kernel adapter

Required projects PR-T57-01–08. Use a snapshot guest, no host shared directory,
bridged network, credentials or other data. Record guest kernel release/config,
headers, compiler, lockdep/KASAN availability and source hashes. Never load this
module in the workstation kernel. This task supplies source; it does not load it.

Implement owner.c from its scaffold. The reference lives under assessor and is
not copied by learner init. Build in the guest with its matching kernel tree:
`make -C /lib/modules/$(uname -r)/build M=$PWD modules`. Build-only success is not
load/unload evidence. For reference investigation copy the separate assessor
owner.c into a fresh disposable directory with this Makefile, label it assisted.

The original target exposes /dev/cm_b06_owner, mode0600, at most32 synthetic
bytes per write. Open/release own references; work owns a reference from before
queueing until callback completion. Exit deregisters entry before flushing work
and dropping the initial reference. On allocation/registration failure unwind
only resources acquired. Inject a user-copy fault at a protected-page boundary;
copy_from_user returns uncopied bytes and may sleep. Decide whether this record
protocol rejects partial input or publishes a prefix, and test the choice.

Required observations: hold descriptor while requesting unload; close and retry;
race writes with close in guest; inject failures at allocation/registration;
run100 unload cycles and inspect counters and lockdep/KASAN output. Add a lock
context variant and an RCU lookup/removal variant; supply actual lookup protection
and grace-period evidence. This reference is not an RCU or hard-IRQ implementation.
Do not infer privilege escalation from a crash. Harmless success is an owned
synthetic buffer, exact release count and quiescent unload.

## Implemented continuation

The required [concrete continuation](../guest/README.md) supplies the additional source,
fixtures, failure controls, objective criteria and bounded author execution.
Its evidence does not replace independent learner or physical obligations.
