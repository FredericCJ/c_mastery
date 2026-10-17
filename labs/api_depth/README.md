# API contract depth laboratories

Use `python -m cmastery init PR-T18-01 DEST --family api_depth` for append, or `PR-T19-01` for callbacks. The source is deliberately unfinished. Use the exact [contracts](CONTRACTS.md) and required legacy assessment supplement for the parent project. The family adds laboratory evidence without rewriting accepted unit identities or hours.

Build with `cmake -S . -B build`, `cmake --build build`, then `ctest --test-dir build --output-on-failure`. The host test program may use the C library; the implementation may use only the declared headers, fixed storage, ordinary C operations and caller callbacks. Audit callback dependencies separately.

The separate assessor implementation is opt-in through `CMASTERY_ASSESSOR_ROOT`; it is not a learner dependency. The author campaign producer is `tools/run_api_depth.py --report reviews/v5/expansion/integrated/api_depth_campaign.json`.
