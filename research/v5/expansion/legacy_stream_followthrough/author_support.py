"""Bind exact original stream criteria/variants after their source-bound campaign."""
import copy
import hashlib
import json
from pathlib import Path
from attach import attach, IDS, PUBLIC, SUPPORT, VARIANT_SUPPORT

ROOT = Path(__file__).resolve().parents[4]
BASE = Path(__file__).resolve().parent
REPORT = 'reviews/v5/expansion/legacy_stream_followthrough/variants_validation.json'
CONTRACT = 'labs/api_depth/stream/VARIANTS.md'


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8', newline='\n')


def main():
    report = json.loads((ROOT / REPORT).read_text(encoding='utf-8'))
    if not report['passed'] or len(report['profiles']) != 6 or sum(len(p['outcomes']) for p in report['profiles']) != 96:
        raise ValueError('Incomplete stream variant campaign')
    for path, digest in report['inspected_paths'].items():
        if hashlib.sha256((ROOT / path).read_bytes()).hexdigest() != digest:
            raise ValueError('Stale variant source ' + path)
    resource = dict(id='R-LSTREAM-COURSE', title='Original copied-frame stream producer and changed-condition contracts',
        authors=['C Mastery authors'], url=CONTRACT, kind='course', edition='Original C17 teaching API',
        domains=['T19'], locators=['stream.h and complete assessor/api_depth/stream.c: staged frame, generation and sf_pump callback progress',
            'VARIANTS.md sections A-C: independent status/count, borrowed scatter/gather spans and two-frame ownership/reset',
            'variants_test.c result_case/vector_case/queue_case and independent decoded oracle'],
        access='Actual complete bounded course sources inspected', role='oracle', assignable=True,
        reason='Supply the exact original three delayed variants with an actual concrete producer import.',
        prerequisites=['Byte spans, bounded progress and live-owner identity'],
        inspection=dict(date='2026-10-17', method='Read actual producer implementation/header, original AX tasks and all new bounded contracts/tests.',
            scope='Original course mechanism and exact three changed conditions; not a new external standards inspection.',
            limits='No independent reviewer, delayed learner, operating-system reset or physical target claim.', evidence_urls=[CONTRACT]))
    write(BASE / 'resources.json', {'schema_version': 1, 'resources': [resource]})
    write(BASE / 'research_gate.json', {'status': 'passage-inspected', 'domains': ['T19'],
        'inspection_kind': 'Author inspection of original course source, not independent acceptance',
        'research_input_sha256': {'research/v5/expansion/legacy_stream_followthrough/resources.json':
                                 hashlib.sha256((BASE / 'resources.json').read_bytes()).hexdigest()}})
    canonical = ROOT / 'curriculum/v5/expansion/legacy_assessments/PR-T19-04.json'
    record = attach(json.loads(canonical.read_text(encoding='utf-8')))
    write(canonical, record)
    scopes = {
        'a': 'The exact2,0,1,error primary trace and every0..5 positive-progress-plus-error prefix execute. Stable error wins even on the final byte; retry/wait budget and invalid callback controls preserve the declared cursor.',
        'b': 'Original [3,1,2,3,6] framing and a single cursor over trimmed nonempty header/payload/checksum spans execute with455 payload/quota pairs, boundary errors and an independent decoder.',
        'c': 'Two actual copied producer frames execute full admission preservation, FIFO callbacks, stale generation cancellation, partial-frame transport taint/reset and finite retirement. Reset is an explicit owner assertion exercised with a new synthetic sink.'}
    updates = []
    for identifier in IDS:
        criterion = next(c for c in record['criteria'] if c['id'] == identifier)
        row = {'id': identifier, 'original_source': canonical.relative_to(ROOT).as_posix(),
               'original_objective': criterion['objective'], 'project': record['project'],
               'dimensions': {
                   'teaching': {'state': 'complete-for-declared-contract', 'evidence': [CONTRACT, 'labs/api_depth/stream/CONTRACTS.md']},
                   'implementation': {'state': 'complete-for-declared-reference-and-changed-condition', 'evidence': ['assessor/api_depth/stream.c', 'assessor/api_depth/stream_variants.c', 'labs/api_depth/stream/variants_test.c']},
                   'author_validation': {'state': 'passed-with-stated-scope', 'evidence': [REPORT]},
                   'external_review': {'state': 'not-run-for-this-successor', 'evidence': []},
                   'independent_learner': {'state': 'not-assessed', 'evidence': []},
                   'physical_observation': {'state': 'not-run-no-target-claim', 'evidence': []}},
               'closure_rationale': scopes[identifier[-1]], 'producer_support': SUPPORT,
               'variant_ids': [v['id'] for v in criterion['variants']],
               'required_environment': 'Original hosted C17 components with explicit completed io.stream import; serialized API and synthetic callbacks.',
               'external_blockers': ['Independent delayed learner work and external review remain unperformed.'],
               'evidence_limits': ['No whole T19 closure, thread-safety proof, foreign OS, physical observation or strict freestanding audit.',
                                   'The supplied callback result type and transport reset model are original course contracts, not OS syscall guarantees.']}
        updates.append(row)
        for variant in criterion['variants']:
            child = copy.deepcopy(row)
            child.update(id=variant['id'], parent_id=identifier, original_variant_id=variant['id'],
                         original_variant_task=variant['task'], minimum_days=variant['minimum_days'],
                         variant_ids=[variant['id']], variant_readiness='Actual bounded author implementation and tests; learner delay/independence remains unassessed.')
            updates.append(child)
    pins = [REPORT, SUPPORT, canonical.relative_to(ROOT).as_posix(), CONTRACT,
            'research/v5/expansion/legacy_stream_followthrough/resources.json',
            'research/v5/expansion/legacy_stream_followthrough/attach.py',
            'research/v5/expansion/legacy_stream_followthrough/author_support.py']
    write(ROOT / VARIANT_SUPPORT, {'schema_version': 1, 'required': True, 'updates': updates,
        'source_sha256': {path: hashlib.sha256((ROOT / path).read_bytes()).hexdigest() for path in pins},
        'scope': 'Exact T19.08 a/b/c and their original delayed variants; other PR-T19-04 objectives retain their own obligations.',
        'supersedes_pending_binding': ['research/v5/modern/stream_producer.json: original three delayed variants',
                                     'reviews/v5/expansion/legacy_stream_followthrough/support_validation.json: earlier partial binding observation'],
        'remaining_author_work': [], 'remaining_external_work': ['Independent delayed learner attempts and external review.']})
    path = ROOT / 'labs/api_depth/family.json'
    family = json.loads(path.read_text(encoding='utf-8'))
    campaigns = family.setdefault('supplemental_campaigns', [])
    campaigns[:] = [c for c in campaigns if c.get('id') != 'stream-variants']
    campaigns.append({'id': 'stream-variants', 'producer': 'tools/run_stream_variants.py', 'report': REPORT,
                      'contract': CONTRACT, 'scope': 'Exact T19.08 parent and delayed criteria; actual explicit io.stream import;96 compiled/executed reference/control outcomes.'})
    write(path, family)
    print(json.dumps({'updates': len(updates), 'criteria': len(IDS), 'variants': 3}))


if __name__ == '__main__':
    main()
