"""Materialize original changed-condition IDs without manufacturing learner work."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path

SCOPE = {
    'T23.02.a': 'Original helper actually terminates itself with SIGTERM; wait classification preserves signal termination rather than normal exit.',
    'T23.02.b': 'Stage-tagged eight-byte little-endian failure records execute with unknown-stage and short-record rejection.',
    'T23.03.b': 'Actual crossed descriptor redirections use a temporary descriptor and preserve both original streams.',
    'T23.03.c': 'An original helper transfers the hidden writer to a grandchild; the public ownership test observes delayed EOF and reaps owned children.',
    'T23.08.a': 'Actual file-backed private mappings compare sharing before and after the child writes, with explicit process synchronization.',
    'T23.08.b': 'Actual MAP_SHARED update and explicit process synchronization are compared with MAP_PRIVATE behavior.',
    'T23.08.c': 'The supplied page-accounting analysis changes page size and huge-page assumptions without changing host policy; no huge-page execution is claimed.',
    'T23.09.a': 'The reference remap prepares replacement storage before publication, changes generation, and rejects the old borrow.',
    'T23.09.c': 'Actual separate-process in-bounds publication uses explicit exclusion and checks synchronization independently of truncation faults.',
    'T23.10.a': 'Two actual initializer processes contend through the supplied locking/publication protocol; cleanup and publication are tested.',
    'T23.10.b': 'Explicit byte decoding executes for both serialized endian layouts; it does not claim an opposite-endian host run.',
    'T23.10.c': 'Fresh-exec/restart publication uses generation/session identity instead of relying on process-ID uniqueness.',
    'T23.05.a': 'Actual hard-link/unlink/open-descriptor observations establish one retained inode identity across the named handles.',
    'T23.05.b': 'Actual directory rename preserves the open directory-handle lookup anchor.',
    'T23.05.c': 'The original intermediate-symlink counterexample defeats final-only no-follow; actual confined lookup rejects it.',
    'T23.06.a': 'Actual owned ext4 and tmpfs files exhibit filesystem-specific metadata/allocation observations; assertions remain relational.',
    'T23.06.b': 'A declared default-ACL permission model executes separately from actual child umask behavior; no host ACL installation is claimed.',
    'T23.06.c': 'The supplied authority model changes the operation to directory-mediated replacement and revises its permission argument; it is not a privileged host denial test.',
    'T23.04.a': 'Actual/injected sink failure after five committed bytes preserves the exact progress cursor and remaining buffer ownership.',
    'T23.04.b': 'Contrasting interrupted-close ownership rules have executable wrapper models; a second operating-system implementation has not been executed.',
    'T23.07.a': 'Actual private i386 off_t32 and large-file ABI builds regenerate the checked extent boundaries from sizeof and type limits.',
    'T23.07.b': 'Actual sparse holes and explicit-zero writes compare identical logical bytes with separately observed allocation.',
    'T23.07.c': 'Overlapping-record synchronization and an intentionally split critical section execute; positioned I/O alone is not treated as an atomic transaction.',
    'T23.11.a': 'The actual three-stage pipeline has an early-exiting sink; the producer/copy failure path closes owned pipes and reaps every owned child.',
    'T23.11.b': 'Owned PTYs execute all four VMIN/VTIME classes with actual read enrollment and broad observational timing bounds.',
    'T23.11.c': 'An actual half-closed local socket observes EOF while still sending the remaining response.',
    'T24.10.a': 'Actual owned-PTY VMIN>0/VTIME>0 read enrollment distinguishes no-first-byte waiting from the observed interbyte timeout.',
    'T24.10.b': 'The same explicitly imported parser object executes reconnect-mid-frame reset across byte array, PTY and local socket transports.',
    'T23.04.c': 'The split effect/receipt transaction mutant executes the missing atomicity case; correct imported storage/receipt code survives all 74 public modeled cut outcomes.',
    'T23.09.b': 'The actual equivalent-contract benchmark repeats sparse random access, correctness/error controls and 192 recorded timing samples across six profiles.',
}


def expand(value, root):
    value = deepcopy(value)
    parents = [row for row in value['updates']
               if row['id'].startswith('AX-') and not row['id'].endswith('-DELAYED')]
    existing = {row['id'] for row in value['updates']}
    for parent in parents:
        objective = parent['original_objective']
        if objective not in SCOPE:
            raise ValueError('Missing specific changed-condition disposition: ' + objective)
        source = root / parent['original_source']
        record = json.loads(source.read_text(encoding='utf-8'))
        criterion = next(c for c in record['criteria'] if c['id'] == parent['id'])
        for variant in criterion['variants']:
            if variant['id'] in existing:
                continue
            child = deepcopy(parent)
            child.update(id=variant['id'], parent_id=parent['id'],
                         original_variant_id=variant['id'], original_variant_task=variant['task'],
                         minimum_days=variant['minimum_days'], variant_ids=[variant['id']],
                         closure_rationale=SCOPE[objective],
                         variant_readiness='Author reference scope only; the separately delayed unaided learner attempt remains unassessed.')
            child['dimensions']['independent_learner'] = {'state': 'not-assessed', 'evidence': []}
            child.setdefault('evidence_limits', []).append(
                'This row binds the original changed-condition task only; parent execution is not a learner attempt or completion of the delay.')
            if objective == 'T23.04.b':
                child['dimensions']['implementation']['state'] = 'partial-wrapper-model-second-os-adapter-unexecuted'
                child['dimensions']['author_validation']['state'] = 'passed-model-only-second-os-observation-pending'
                work = variant['id'] + ': execute the wrapper against a genuinely different interrupted-close OS contract when an authorized environment is available; do not relabel the model as that execution.'
                if work not in value.setdefault('remaining_author_work', []):
                    value['remaining_author_work'].append(work)
            value['updates'].append(child)
            existing.add(child['id'])
    helper = Path(__file__).resolve()
    value.setdefault('source_sha256', {})[helper.relative_to(root.resolve()).as_posix()] = hashlib.sha256(helper.read_bytes()).hexdigest()
    return value
