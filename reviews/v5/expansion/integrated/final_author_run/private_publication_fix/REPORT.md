# FIX-PRIVATE-PUBLICATION

Parent obligation: `G-V5-DOMAIN-TOOLS`. This is an author implementation
defect found by actual candidate reproduction, not an external blocker.

The candidate `candidate_20261019T030129449614Z` stopped before rebuilding:
`package.write_zip` rejected a private PDF in the HTML publication. Source,
learner and assessor ZIP creation and extracted source-byte verification had
already finished; those steps did not establish safe HTML publication.

`R-LCS-ST` retained a repository-private capture as its bibliography URL.
`publish_links.learner_source` delegated to a learner filter that assumes the
packager has already removed private files. The publisher did not establish
that prerequisite. Its fresh source tree therefore admitted the capture, and
Sphinx made both a static copy and a hashed download copy. This was not a stale
output accident. No release was accepted, promoted or published.

The correction:

- Denies private capture and environment/cache path components at publication
  routing, including decoded paths and aliases resolving to those components.
  An excluded citation does not require its private payload to exist. Its
  notice does not claim that the payload ships in the full source bundle.
- Adds an independent whole-output payload check, including unlinked binary
  assets; checking article text and live links alone was insufficient.
- Projects one exact public bibliography location from existing corroborating
  metadata with the identical recorded capture digest. Original registration,
  inspection, resource identity and source registers remain unchanged. The
  projection makes no claim about current remote bytes or newly read passages.
- Runs the location qualification guard in the established full build.

The focused checks in `test_runs/result.json` include actual Sphinx rendering,
preserved public downloads, absent private captures, percent-encoded paths,
private aliases, and unlinked forbidden payloads. Windows symlink-privilege
skips are explicit; Ubuntu executes the corresponding path controls.
`predecessor_rejection_v2.json` records the exact old implementations rejected
by semantic assertions. The first router-control attempt trapped a lazy policy
import rather than the copy operation; its raw result is retained and is not
used as copying evidence. The corrected attempt preloads that policy and
requires the `private copy` assertion specifically.

`retained_receipts/manifest.json` preserves 279 prior receipts and raw files
before rerunning producers. `preimages.json` pins the prior implementations.
No historical acceptance or learner record is rewritten.

Full regeneration must remove both old PDF copies. A new full build,
source-current foundation and campaign execution, integrated validation, and
actual candidate reproduction remain required. Focused passes alone do not
close this defect. Actual later results belong in the final run ledger;
physical observations, external review and independent learner assessments
remain separate.
