# FW-S1 assessor analysis

Only EVP success1 is acceptance. Ed25519 uses a NULL digest and one-shot
verification; passing a prehashed digest without selecting a different specified
scheme changes the signed contract. The adapter initializes a rejected result
and assigns the verified key only after success, then releases both OpenSSL
objects. The bridge compares that identity rather than accepting a bare truthy
verdict. Corruption of the signed key-ID byte is a message change even when both
keys are present. A newer image signed by revoked key1 still rejects, while the
independently selected key2 policy admits version8.

The first mutant admits cryptographic failure; the second reports the other key.
Both fail concrete assertions. Provider allocation failure is fail-closed in the
code; the current campaign does not independently inject every provider allocator
failure. Require that limitation in a learner dossier instead of claiming exhaustive
provider testing. Production hardware roots, immutable storage and rollback
counters are different mechanisms, not properties supplied by this host adapter.
