# Exact PR-T35-06 implementation audit

Read the original project, fixture and all six criterion/changed-task objects before
authoring. Preserve those objects and baseline binding byte-for-byte in the additive
attachment. Existing digest/version fixture supplied a useful oracle but did not
implement actual signature/trust rotation, reproducible malicious-source comparison
or complete command/descriptor/clearing composition. The successor supplies those
mechanisms under the parent's existing profile A provider allowance.

Actually inspected primary passages: OpenSSL3.5 EVP_SIGNATURE-ED25519 description
(pure one-shot whole-message semantics); EVP_DigestVerifyInit Ed25519 NULL-digest and
one-shot verification/return meanings; EVP_PKEY_new raw public/private keys, public-key
derivation and free ownership; EVP_DigestInit EVP_Q_digest description/return values
(6404–6406,6605–6607 in accessible HTML); reproducible-builds definition of specified
inputs, environment, instructions and bit-identical output. Direct EVP_Q_digest URL
failed; the canonical EVP_DigestInit page was read instead. No provider source/security
audit or final published C17/C23 inspection is claimed.

Actual local imports inspected: command public contract and private service state
machine, worker descriptor policy and explicit secret clearing primitive. The runner
pins them without modifying frozen providers. Review matrix and source-specific
assembly/section analysis are separate assessor material. Initial sanitized negative
harness leak is retained as attempt history and corrected; no leak suppression.

All targets are original local programs with synthetic data and known teaching keys.
Actual Ed25519/provider behavior is hosted execution; logical commit is not flash
installation. Author validation, external review, delayed independent learner and
physical observation remain separate. T36/T37 remain required independent scope.
