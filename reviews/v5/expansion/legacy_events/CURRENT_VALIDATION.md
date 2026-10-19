# Current T24.01–04 author validation

The current combined campaign passed356commands and24 ordinary/changed
observations. It supports only12 criteria, leaving22 other T24 objectives as
ordinary author work and preserving the two existing terminal criteria.

Executed on the recorded WSL Linux host with GCC15.2/Clang21.1.8:

```
python3 tools/run_events_followthrough.py --module signals --module ipc --report reviews/v5/expansion/legacy_events/current_validation.json
.venv/Scripts/python.exe -X utf8 tools/check_events_followthrough.py --report reviews/v5/expansion/legacy_events/current_validation.json
.venv/Scripts/python.exe -X utf8 tools/close_events_followthrough.py --report reviews/v5/expansion/legacy_events/current_validation.json
.venv/Scripts/python.exe -X utf8 tools/check_events_exports.py
```

All exited0. `current_validation.json` retains exact argv, cwd, environment,
statuses, raw output, generated inputs, source and retained-object hashes.
Its SHA256 is `4fc32b8e671534da10d086b15481e0aa654ea590ff6a05091d91c7c16c822dbf`.
The authoritative actual digest is in the guard/support records; use those for
machine verification. The27 guard controls rejected omitted observations,
profiles, source/object evidence, selected semantic builds, source changes and
inflated completion claims. Two actual CLI exports preserve the original
criteria/seven-day delay and exclude assessor implementations. `integration.json`
also verifies pure canonical attachment reproduces each current parent from its
original checkout-HEAD version and is idempotent.

New IPC evidence includes six GCC/Clang debug/release/ASan+UBSan profiles,
26-state208-edge independent ownership checks,3 actual512-request transports,
3 controlled disconnect/cancel traces,3 independent execs across2 epochs,
81 independently calculated deadline cells and actual stalled partial response,
five acquisition-failure cuts, accept/cancel cleanup, IPv4/IPv6/private Unix and
Linux abstract names. Eight compiling incorrect/stub implementations fail the
intended checks. The prior signal package is rerun in the same final campaign.

For each optimized compiler/transport the raw cost data include a warmup and
seven rotating-order samples. These ran on a shared author host, not an isolated
benchmark machine; timing and CPU observations establish neither a universal
ranking nor worst-case response bounds. Named shared memory continuity is normal
worker-exit behavior, not reboot or power-loss persistence. Author execution
does not establish external review, learner performance or delayed transfer.

Earlier failed handler implicit-helper and drvfs pathname attempts remain in
their original run/pilot records. Native Linux temporary directories provide
the same pathname-socket interface without altering host settings. All242 pinned
historical receipts remained unchanged at the post-integration check.
