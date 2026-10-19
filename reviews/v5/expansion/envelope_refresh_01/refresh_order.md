# Scoped evidence refresh order

This directory records metadata/export refreshes. It is not a replacement C
campaign or a global inventory update.

1. Preserve prior support, guards, exports and affected producer bytes. The
   `owned_manifest.json` history records 25 selected packages. Supervision has
   its separate history and manifest. Original C campaign hashes are checked
   before and after each selected package refresh.
2. Run each established actual exporter with the current canonical CLI. The
   selected project brief and all its declared public assets are checked and
   pinned by `tools/envelope_export_inputs.py`. It does not pin unrelated
   exercise-catalog rows. Its seven synthetic controls are separate from actual
   package exports.
3. Run the existing mechanism-specific guard, existing close producer and
   package guard, in that order. `tools/refresh_owned_envelopes.py` orchestrates
   this sequence for the exact 25 packages under WSL. Its preparation step is
   deliberately one-use because history must not be overwritten.
4. Supervision uses its established `author_support.py`, then
   `tools/check_constrained_supervision.py --report NEW.json`. The guard now
   executes actual canonical CLI initialization and retains its original
   campaign/source checks. No physical F439 evidence is inferred.
5. Once all relevant provider envelopes pass, refresh direct consumers:
   `tools/build_systems_x_followthrough_support.py` then
   `tools/check_systems_x_followthrough.py --self-test --report NEW.json`;
   `tools/reconcile_device_parent_gaps.py --write --self-test`;
   `tools/reconcile_interrupt_depth.py --write --self-test --report NEW.json`.
   Capture the device producer's structured stdout in a versioned receipt.
6. Root merges current exact provider rows. Then preserve and rerun
   `tools/audit_inventory_aliases.py`, `tools/build_inventory_aliases.py` and
   `tools/check_inventory_aliases.py --self-test --report NEW.json`, and the
   equivalent audit/build/check named-parent producers. Their intake preserves
   exact original identities; selected child rows must match the current
   canonical implementation register. The resulting support does not pin that
   mutable register.
7. Root signals final provider metadata freeze before
   `tools/build_residual_followthrough_support.py` and
   `tools/check_residual_followthrough.py --self-test --report NEW.json`.
   Preserve the preceding residual support and evidence map first.
8. Run `tools/audit_current_envelopes.py --output NEW.json` again. This read-only
   direct-pin audit excludes explicit history and other owners' active source
   work. It complements, rather than replaces, transitive mechanism guards.

The older checkpoint umbrella producer is
`research/v5/expansion/legacy_completion/support.py` without `--attach-records`.
It must run after checkpoint cards and retained campaign receipts are current;
it reports pending source validation explicitly and cannot silently promote a
stale receipt. The modern errors/ownership reconciliation uses
`tools/reconcile_errors_ownership_readiness.py`, checking its exact 20 exercises,
80 rubric rows and 20 delayed identities without new C execution.
