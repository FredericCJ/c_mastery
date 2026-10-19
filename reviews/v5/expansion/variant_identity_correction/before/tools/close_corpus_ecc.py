"""Bind exact ECC rubric and delayed variants to current author evidence."""
import copy
import json
import pathlib
import sys

from common import ROOT, sha
sys.path.insert(0, str(ROOT))
from cmastery.exercises import parse_brief
from check_corpus_ecc import validate

HERE = ROOT / 'research/v5/expansion/corpus_ecc'
REPORTS = [
    'reviews/v5/expansion/corpus_ecc/current_validation.json',
    'reviews/v5/expansion/corpus_ecc/support_validation.json',
    'reviews/v5/expansion/corpus_ecc/export_validation.json',
]


def main():
    reports = [json.loads((ROOT / p).read_text(encoding='utf8')) for p in REPORTS]
    campaign, guard, export = reports
    assert not validate(campaign), 'current campaign fails scoped guard'
    assert guard['passed'] and len(guard['negative_controls']) == 13
    assert guard['report_sha256'] == sha(ROOT / REPORTS[0])
    assert export['passed'] and export['source_stable']
    assert len(export['executions']) == 11 and all(r['passed'] for r in export['executions'])
    assert not export['private_sources_copied']
    for path, digest in export['input_sha256'].items():
        assert sha(ROOT / path) == digest, ('stale export', path)
    brief = ROOT / 'curriculum/v5/exercises/X-T15-008.md'
    meta, _ = parse_brief(brief)
    artifacts = ['labs/corpus_ecc/CONTRACTS.md', 'labs/corpus_ecc/ecc.h',
                 'assessor/corpus_ecc/ANALYSIS.md', 'assessor/corpus_ecc/ecc.c',
                 'labs/corpus_ecc/tests.c', 'assessor/corpus_ecc/oracle.py']
    scopes = {
        'X-T15-008': ('All five rubric mechanisms and both changed-condition variants.', ['corpus_ecc.8', 'corpus_ecc.16', 'corpus_ecc.erasures', 'corpus_ecc.burst']),
        'X-T15-008-R1': ('Independent parity-equation codebook, injective systematic representation and minimum-distance argument for both widths.', ['corpus_ecc.8', 'corpus_ecc.16']),
        'X-T15-008-R2': ('Every payload with zero, one and two bit flips: 592 small and 280576 changed cases per native profile; invalid inputs and unchanged refusal output.', ['corpus_ecc.8', 'corpus_ecc.16']),
        'X-T15-008-R3': ('Unique bounded erasure/error enumeration, exact 2t+s<4 admission and independent nearest-codeword enumeration for every small received word. Larger erasure coverage is selected, with the separate distance proof stated.', ['corpus_ecc.erasures']),
        'X-T15-008-R4': ('Four-lane and rotating-phase maps, all burst starts and lengths1..4, every phase, exact work/storage/rate analysis under declared guarantees.', ['corpus_ecc.burst']),
        'X-T15-008-R5': ('Three-flip miscorrection, four-flip undetected codeword, chosen valid-word forgery and wrong-phase identity counterexamples in original synthetic arrays.', ['corpus_ecc.8', 'corpus_ecc.burst']),
        'X-T15-008:V1': ('Changed16/11 decoder is actually implemented and exhaustively checked for up to two flips; erasure/error admission and selected larger erasure cases are also executed.', ['corpus_ecc.16', 'corpus_ecc.erasures']),
        'X-T15-008:V2': ('Rotating phase is actually implemented; every phase and four-bit burst is checked, and incorrect phase demonstrates valid decoding with wrong logical ownership.', ['corpus_ecc.burst']),
    }
    assert {r['id'] for r in meta['rubric']} <= scopes.keys()
    rows = []
    for ident, (scope, anchors) in scopes.items():
        rows.append(dict(id=ident, parent_exercise=meta['id'], parent_project=meta['project'],
                         dimensions={
                             'teaching': dict(state='complete-for-declared-bounded-contract', evidence=artifacts[:3]),
                             'implementation': dict(state='complete-for-declared-reference-and-changed-conditions', evidence=artifacts),
                             'author_validation': dict(state='passed-source-bound-native-oracle-and-target-audit', evidence=REPORTS),
                             'external_review': dict(state='not-run', evidence=[]),
                             'independent_learner': dict(state='not-assessed', evidence=[]),
                             'physical_observation': dict(state='not-observed;finite-channel-model-only', evidence=[]),
                         }, artifacts=artifacts, closure_rationale=scope,
                         author_validation=dict(report=REPORTS[0], checks=anchors, export=REPORTS[2]),
                         minimum_days=7 if ':V' in ident else None, remaining_author_work=[],
                         evidence_limits=copy.deepcopy(campaign['limitations']) + [
                             'Four Cortex-M4 object/link audits were performed; no Cortex-M or physical noisy-channel execution is claimed.',
                             'The delayed variants have author implementations and tests, not independent delayed learner evidence.',
                             'A finite test suite does not prove arbitrary mastery or authenticity.']))
    paths = set(campaign['inspected_paths']) | set(export['input_sha256']) | set(REPORTS)
    paths |= {p.relative_to(ROOT).as_posix() for p in HERE.iterdir() if p.is_file() and p.name in ['resources.json', 'taxonomy_extensions.json', 'research_gate.json']}
    paths |= {'tools/author_corpus_ecc.py', 'tools/close_corpus_ecc.py', brief.relative_to(ROOT).as_posix()}
    result = dict(schema_version=1, scope='New required Hamming-code competency refinement; bounded author support only.',
                  source_sha256={p: sha(ROOT / p) for p in sorted(paths)}, current_reports=REPORTS, updates=rows)
    out = HERE / 'completion_dispositions.json'
    out.write_text(json.dumps(result, indent=2) + '\n', encoding='utf8')
    print(json.dumps(dict(passed=True, updates=len(rows), path=out.relative_to(ROOT).as_posix(), sha256=sha(out))))


if __name__ == '__main__':
    main()
