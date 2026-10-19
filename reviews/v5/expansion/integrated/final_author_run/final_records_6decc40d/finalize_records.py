"""Prepare versioned closeout metadata only after the completed 6decc author run.

No producers, tests, release acceptance, staging or commit are invoked. This
helper was prepared while reproduction was running; root must review and run it
only after the pipeline exits successfully and final evidence is retained.
"""
from pathlib import Path
from datetime import datetime, timezone
import argparse
import hashlib
import json
import os
import re
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
SOURCE = '6decc40df2c438f02f15b5e8ed452ddf57fd026945e97b0fe5c9f4a91cfba35c'
BASE = ROOT / 'reviews/v5/expansion/integrated/final_author_run'
RECEIPTS = BASE.parent
RUN = ROOT / 'build/author_environment_successor_fix/run_20261019T144425724162Z/result.json'
CAND = BASE / 'candidate_20261019T174219076702Z'
RETENTION = BASE / 'final_evidence_6decc40d/retention_map.json'
HISTORY = BASE / 'final_records_6decc40d'
ORIGINALS = {
    'private_publication_fix/integration.json': ('FIX-PRIVATE-PUBLICATION', '6ccefee1950f516e967ae9bcca37eead07d1b2ca44c48be30a67f13889d7d0b4'),
    'author_environment_dependency_fix/finding.json': ('FIX-AUTHOR-ENV-CORRECTION-CAPTURES', 'e9dfa78b938b7ea8dcd2213ff7baea5cb3849bd744aa6839afc62e7572a6119d'),
    'author_environment_successor_fix/finding.json': ('FIX-AUTHOR-ENV-SUCCESSOR-EXPORTS', 'abb5e9bd3067e3d9181cd792f40df37ca25aa0344b06080a77c604a8239cd98c'),
}
CONTINUATION = BASE / 'continuation_current.json'
CONTINUATION_SHA = '258ecc3a3cab1b82b1186f0fd00e0087287d2a43faf42faad4f6ad068c9c7fa0'
INVENTORY = ROOT / 'curriculum/v5/expansion/work_inventory.json'
UPDATES = ROOT / 'curriculum/v5/expansion/implementation_updates.json'
DIMENSIONS = BASE / 'dimension_reconciliation.json'
REMAINING = BASE / 'remaining_implementation_dimensions.json'
UNCHANGED = {
    INVENTORY: '585153d3af5fb7cc5348c56d33f7c4ad3580d2f03dd96464efea73edfff3e44a',
    UPDATES: 'f1862fc475b68457d816dcf1b1e64c10f98f74a9b67b7dc82d30e4f9a800cbe9',
    DIMENSIONS: 'aecfee1da13b7389265bf042b80e32e1091f3e1dc4c0d9961f22dd1ccb118f1c',
    REMAINING: '864815c8f3e27696bfb1109475cbf2e62e6de2c63d81b43388b5de56155c2bca',
}
BOUNDARIES = [
    dict(ids=['AX-PR-T23-02-T23.04.b-DELAYED', 'IW-e2cdc87b21fb7b31'],
         remaining='Authorized QNX SDP8 compiler/sysroot and disposable licensed QNX8 guest; actual target compilation and interrupted-close capture remain unperformed.'),
    dict(ids=['G-DG-BOARD-ROUTE'], remaining='Exact as-built MB1137/PHY schematic, straps, reset/clock and pin routes, with actual board identity.'),
    dict(ids=['G-DG-USB-ADDRESS0'], remaining='Authoritative resolution of conflicting SET_ADDRESS(0) wording; implemented nonzero/reset behavior stays separate.'),
    dict(ids=['G-PL-G474-FULL-PORT'], remaining='Full readable RM0440 and identified physical board/route/clock/wire observations; selected source-supported implementation is retained.'),
    dict(ids=['LRUN-14'], remaining='Authorized readable published C17/C23 thread-support passages for the exact edition comparison; existing lifecycle and signature probes are unaffected.'),
    dict(ids=['XI-GENMC-OPT-IR'], remaining='The retained default-O1 aggregate-memory-intrinsic command fails with signal 4 during promotion; this tool defect remains unresolved. A corrected pinned GenMC/LLVM profile must rerun that exact command and its controls. Separate O1-fno-builtin RC11 queue checks remain bounded evidence.'),
]
DIMENSION_KEYS = {'teaching', 'implementation', 'author_validation', 'external_review', 'independent_learner', 'physical_observation'}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def relative(base, name):
    require(isinstance(name, str) and name and '\\' not in name, 'Noncanonical relative path')
    part = Path(name)
    require(not part.is_absolute() and '..' not in part.parts and part.as_posix() == name, 'Unsafe path')
    path = base / part
    require(path.resolve() == path.absolute() and path.is_relative_to(base), 'Redirected or escaping path')
    return path


def regular(path):
    require(path.is_file() and path.resolve() == path.absolute(), 'Missing/redirected file: ' + str(path))
    return path


def sha(path):
    with regular(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def load(path):
    value = json.loads(regular(path).read_text(encoding='utf8'))
    require(isinstance(value, dict), 'Expected JSON object: ' + str(path))
    return value


def ref(path):
    return dict(path=path.relative_to(ROOT).as_posix(), sha256=sha(path))


def passed(record, label, identity=True, status=True):
    require(record.get('passed') is True, label + ' failed/not final')
    if status:
        require(record.get('status') == 'passed', label + ' has no final passed status')
    if identity:
        require(record.get('input_sha256') == SOURCE, label + ' source differs')


def encoded(value):
    return (json.dumps(value, indent=2, ensure_ascii=False) + '\n').encode('utf8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--write', action='store_true', help='Explicitly write the gated versioned records')
    parser.add_argument('--retention-sha256', required=True, help='Reviewed final retention_map.json SHA256')
    args = parser.parse_args()
    require(args.write, 'No default writes: pass --write after final pipeline success')
    require(Path.cwd().resolve() == ROOT, 'Run from original repository root')
    require(re.fullmatch('[0-9a-f]{64}', args.retention_sha256), 'Malformed retention SHA')

    # All admission and identity checks precede every mkdir/write below.
    runtime = load(RUN)
    passed(runtime, 'Pipeline')
    require(runtime.get('accepted') is False and runtime.get('promoted') is False
            and runtime.get('candidate_archive_inputs_frozen') is False, 'Pipeline still active or release state changed')
    phase_names = ['build', 'release_and_targets', 'validate', 'candidate']
    phases = runtime.get('phase_results', [])
    require([r.get('phase') for r in phases] == phase_names and all(
        r.get('passed') is True and r.get('retained_receipts_unchanged') is True for r in phases), 'Incomplete pipeline phases')
    executions = runtime.get('executions', [])
    require([r.get('phase') for r in executions] == ['build', 'release_and_targets', 'release_and_targets', 'validate', 'candidate']
            and all(type(r.get('exit_code')) is int and r['exit_code'] == 0 for r in executions), 'Incomplete/failed command sequence')
    for row in executions:
        require(sha(relative(ROOT, row['raw'])) == row['raw_sha256'], 'Changed pipeline raw output')
    monitoring = load(ROOT / 'build/final_pipeline_status.json')
    require(monitoring.get('status') == 'passed' and monitoring.get('input_sha256') == SOURCE
            and monitoring.get('candidate_archive_inputs_frozen') is False
            and relative(ROOT, monitoring['result']) == RUN, 'Monitoring state not final/current')

    candidate = load(CAND / 'candidate.json')
    passed(candidate, 'Candidate', status=False)
    require(candidate.get('accepted') is False and candidate.get('promoted') is False
            and candidate.get('rebuilt_source_identity') == SOURCE, 'Candidate identity/release state differs')
    require(candidate.get('learner_excludes_assessor') is True, 'Candidate learner separation not verified')
    for name in ('execution', 'identity_execution'):
        row = candidate[name]
        require(type(row.get('exit_code')) is int and row['exit_code'] == 0, 'Candidate subprocess failed')
    driver = load(CAND / 'result.json')
    passed(driver, 'Candidate driver', identity=False, status=False)
    require(driver.get('phase') == 'candidate' and driver.get('accepted') is False
            and len(driver.get('commands', [])) == 2 and all(type(r.get('exit_code')) is int
            and r['exit_code'] == 0 for r in driver['commands']), 'Candidate driver incomplete')
    environment = load(CAND / 'author_environment_manifest.json')
    require(environment.get('source_identity_sha256') == SOURCE and len(environment.get('inputs', [])) == 976
            and len(environment.get('external_inputs_not_copied', [])) == 2, 'Qualified author environment changed')
    author_environment = candidate['author_environment']
    require(author_environment.get('provisioned') is True and author_environment.get('files') == 976
            and author_environment.get('archive_self_contained') is False
            and author_environment.get('manifest_file_sha256') == sha(CAND / 'author_environment_manifest.json'), 'Candidate environment result differs')

    require(sha(RETENTION) == args.retention_sha256, 'Retention manifest differs from reviewed identity')
    retention = load(RETENTION)
    require(retention.get('input_sha256') == SOURCE and retention.get('accepted') is False
            and retention.get('promoted') is False, 'Wrong retention scope')
    rows = retention.get('files')
    require(isinstance(rows, list) and rows and len(rows) == retention.get('retained_files')
            and all(isinstance(r, dict) and set(r) == {'original', 'copied_from', 'retained', 'sha256', 'bytes'} for r in rows), 'Malformed retention records')
    require(len({r['retained'] for r in rows}) == len(rows), 'Duplicate retained destinations')
    for row in rows:
        path = relative(RETENTION.parent, row['retained'])
        require(re.fullmatch('[0-9a-f]{64}', row['sha256']) and type(row['bytes']) is int
                and path.stat().st_size == row['bytes'] and sha(path) == row['sha256'], 'Retained evidence changed')
    retained = {r['retained']: r for r in rows}
    for name, original in [('runtime/result.json', RUN), ('candidate/candidate.json', CAND / 'candidate.json'),
                           ('candidate/driver_result.json', CAND / 'result.json')]:
        require(name in retained and retained[name]['sha256'] == sha(original), 'Retention not bound to current runtime/candidate')
    require({'extracted/publication.json', 'extracted/preservation.json'} <= retained.keys(),
            'Extracted completion evidence is absent from the retained map')
    extracted_publication = load(relative(RETENTION.parent, 'extracted/publication.json'))
    extracted_preservation = load(relative(RETENTION.parent, 'extracted/preservation.json'))
    passed(extracted_publication, 'Retained extracted publication')
    passed(extracted_preservation, 'Retained extracted preservation')
    require(extracted_preservation.get('changed_historical_paths') == [], 'Extracted historical preservation failed')

    publication = load(RECEIPTS / 'publication.json')
    validation = load(RECEIPTS / 'validation.json')
    release = load(RECEIPTS / 'release_guards.json')
    targets = load(RECEIPTS / 'targets_wrapper.json')
    for label, record in [('Root publication', publication), ('Integrated validation', validation),
                          ('Release guards', release), ('Targets', targets)]:
        passed(record, label)
    for label, record in [('Root publication', publication), ('Extracted publication', extracted_publication)]:
        checks = record.get('checks', [])
        require(len(checks) == 34 and len({r['check'] for r in checks}) == 34 and record.get('stale_checks') == []
                and all(r.get('exit_code') == 0 and r.get('status') == 'passed'
                        and r.get('started_input_sha256') == SOURCE and r.get('input_sha256') == SOURCE for r in checks), label + ' checks incomplete')
    checks = validation.get('checks', [])
    require(len(checks) == 12 and len({r['check'] for r in checks}) == 12
            and all(r.get('exit_code') == 0 for r in checks)
            and validation.get('final_source_sha256') == SOURCE
            and validation.get('source_changed_during_run') is False, 'Integrated twelve-check evidence incomplete')
    for row in checks:
        require(sha(relative(ROOT, row['log'])) == row['log_sha256'], 'Integrated check log changed')
    require(validation.get('publication_current') is True and validation.get('historical_receipts_current') is True
            and validation.get('campaign_summary', {}).get('passed') is True
            and validation['campaign_summary'].get('source_current') is True
            and validation['campaign_summary'].get('families') == 62, 'Integrated evidence scope differs')
    retained_receipts = runtime.get('retained_receipts', {})
    # Family selections share execution receipts; the actual supervisor retains
    # 59 distinct paths, not five plus 62 distinct receipt files.
    require(isinstance(retained_receipts, dict) and len(retained_receipts) == 59, 'Retained receipt selection differs')
    for name, digest in retained_receipts.items():
        require(sha(relative(ROOT, name)) == digest, 'Retained scoped execution identity changed')

    for path, digest in UNCHANGED.items():
        require(sha(path) == digest, 'Inventory/reconciliation input changed: ' + str(path))
    inventory = load(INVENTORY)['obligations']
    updates = load(UPDATES)['updates']
    inv = {r['id']: r for r in inventory}; upd = {r['id']: r for r in updates}
    require(len(inv) == len(inventory) == len(upd) == len(updates) == 6125 and inv.keys() == upd.keys(), 'Inventory IDs no longer match')
    for key in inv:
        require(set(inv[key]['dimensions']) == DIMENSION_KEYS and inv[key]['dimensions'] == upd[key]['dimensions'], 'Six-dimension discrepancy: ' + key)
    remaining = load(REMAINING)
    require(remaining.get('count') == 7 and len(remaining.get('rows', [])) == 7
            and {r['id'] for r in remaining['rows']} == {key for b in BOUNDARIES for key in b['ids']}, 'Remaining implementation boundary changed')
    prior_bytes = regular(CONTINUATION).read_bytes()
    require(hashlib.sha256(prior_bytes).hexdigest() == CONTINUATION_SHA, 'Active continuation changed since preparation')
    originals = {}
    for name, (identifier, digest) in ORIGINALS.items():
        path = relative(BASE, name); record = load(path)
        require(sha(path) == digest and record.get('id', record.get('finding')) == identifier
                and record.get('parent') == 'G-V5-DOMAIN-TOOLS', 'Original finding identity changed')
        originals[name] = ref(path)

    sys.path.insert(0, str(ROOT / 'tools'))
    from v2_common import source_identity
    require(source_identity()['sha256'] == SOURCE, 'Canonical source changed before finalization')
    now = datetime.now(timezone.utc).isoformat()
    evidence = dict(retention=ref(RETENTION), pipeline=ref(RUN), candidate=ref(CAND / 'candidate.json'),
                    candidate_driver=ref(CAND / 'result.json'), author_environment=ref(CAND / 'author_environment_manifest.json'),
                    publication=ref(RECEIPTS / 'publication.json'), validation=ref(RECEIPTS / 'validation.json'),
                    release_guards=ref(RECEIPTS / 'release_guards.json'), targets=ref(RECEIPTS / 'targets_wrapper.json'))
    qualifications = [
        'Completed author build, integrated validation and qualified extracted reproduction; not release acceptance or promotion.',
        '62 family selections use 55 distinct execution receipt paths. The supervisor retains 59 distinct paths in total, including named scope records; these remain byte-identical, not newly re-executed or relabelled.',
        '976 explicit local author inputs were provisioned outside archives; two absolute external inputs were not copied. No ZIP-alone reproduction claim.',
        'All 6125 obligations are represented across six dimensions; this is not a claim all obligations or dimensions are complete.',
        'Seven implementation rows reflect six boundary categories. Additional source/proof, author-validation, independent-review, learner and physical obligations remain individually qualified.',
        'No new source inspection, independent reviewer/learner result, physical observation, staging, commit or push is inferred.',
    ]
    outputs = {}
    closures = []
    for name, (identifier, digest) in ORIGINALS.items():
        destination = relative(BASE, str(Path(name).parent / 'closure.json').replace('\\', '/'))
        record = dict(schema_version=1, id=identifier, parent='G-V5-DOMAIN-TOOLS',
                      state='author-fix-validated-by-completed-current-publication-and-qualified-reproduction',
                      recorded_at=now, input_sha256=SOURCE,
                      active_supersession=dict(predecessor=originals[name], original_record_preserved=True,
                          scope='Supersedes only the original pending author-validation state. Failed attempts, original execution identities and technical scope remain historical.'),
                      evidence=evidence, qualifications=qualifications,
                      accepted=False, promoted=False, independent_external_review=False, learner_mastery='not-assessed',
                      physical_validation='not-inferred', commit=dict(record_timing='authored-before-enclosing-commit',
                          identity_authority='Enclosing Git history supplies the eventual commit identity; this record embeds none.',
                          commit_observed_by_writer=False))
        payload = encoded(record); outputs[destination] = payload
        closures.append(dict(id=identifier, path=destination.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(payload).hexdigest()))
    completion_path = BASE / 'completion_6decc40d.json'
    edition = load(ROOT / 'curriculum/edition.json')
    completion = dict(schema_version=1, kind='author-completion-record-with-explicit-evidence-boundaries',
        input_sha256=SOURCE, recorded_at=now, state='author-validation-complete-with-documented-boundaries',
        substantive_scope=dict(manifest=ref(ROOT / 'curriculum/edition.json'), counts=edition['expected'],
            required_additive_exercises=edition['exercise_expansion']['required_exercises'],
            scope=ref(ROOT / 'research/v5/expansion/followthrough/scope.md')),
        evidence=evidence, closure_records=closures,
        validation=dict(root_publication_checks=34, integrated_checks=12, extracted_publication_checks=34,
            refreshed_producers=['release_guards', 'targets'], runtime_executions=executions,
            retained_receipts_sha256=retained_receipts, family_selections=62,
            distinct_execution_receipt_paths=55, distinct_retained_paths_including_named_scope_records=59,
            retained_scope='62 family selections use 55 distinct execution receipt paths; 59 distinct retained paths in total include the named scope records. No retained execution is relabelled as freshly run.',
            not_rerun=runtime['not_run']),
        inventory=dict(obligations=6125, active_dispositions=6125, dimensions=sorted(DIMENSION_KEYS),
            source=ref(INVENTORY), dispositions=ref(UPDATES), reconciliation=ref(DIMENSIONS)),
        remaining=dict(report=ref(REMAINING), implementation_rows=remaining['rows'], distinct_boundary_categories=BOUNDARIES,
            caution='Seven implementation rows are not the full source/review/learner/physical backlog.'),
        qualifications=qualifications, accepted=False, promoted=False,
        learner_mastery='not-assessed', independent_review='separate-record-required', physical_validation='not-run',
        commit=dict(record_timing='authored-before-enclosing-commit',
            identity_authority='Enclosing Git history supplies the eventual commit identity; this record embeds none.',
            commit_observed_by_writer=False), pushed=False)
    outputs[completion_path] = encoded(completion)
    completion_ref = dict(path=completion_path.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(outputs[completion_path]).hexdigest())
    markdown = '# Completed author validation — release and commit remain separate\n\n'
    markdown += 'Current source: `' + SOURCE + '`. Root publication (34 checks), integrated validation (12 checks), refreshed release/target producers and actual qualified extracted publication (34 checks) passed. The completed candidate also verified source identity, HTML equality, generated fixed points and learner/assessor separation.\n\n'
    markdown += 'The manifest contains 76 domains, 609 projects, 2,542 objectives and 1,745 working units, with 238 required additive exercises. Original accepted records remain preserved. All 6,125 obligation IDs carry six separate dimensions. Counts do not establish learner mastery or closure of every dimension.\n\n'
    markdown += 'Completed author work includes criterion-specific legacy mechanisms and changed-condition transfer tasks; B06 compiler, kernel and ABI implementations; modern protocol, coroutine, object-runtime and replication mechanisms; real I/O cancellation and fallible-close composition; and checkpoint/capstone adapters that consume the actual prerequisite interfaces. Original strict-freestanding helpers and selected target link/execution audits retain their precise dependency and target limits. Learner scaffolds, separate assessor implementations, explicit rubrics, independent fixture oracles and causal negative controls accompany the mechanisms. The finite corpus companions cover concrete parser, indexing, storage, arithmetic, spatial and concurrent structures without treating a survey heading as an implementation.\n\n'
    markdown += '62 family selections use 55 distinct execution receipt paths. There are 59 distinct retained paths in total, including named scope records, preserved with their exact scoped identities and guards. The current run did not rerun those retained producers. Qualified reproduction supplied 976 declared local author inputs; two absolute external inputs were not copied. No self-contained archive or release acceptance is claimed.\n\n'
    markdown += 'Seven implementation rows retain six boundaries:\n\n'
    markdown += '\n'.join('- ' + ', '.join('`' + key + '`' for key in b['ids']) + ': ' + b['remaining'] for b in BOUNDARIES)
    markdown += '\n\nAdditional named source/proof gaps, hardware PMU and other physical observations, independent review, delayed independent learner results and time calibration remain separately qualified. Author checks do not supply that evidence.\n\n'
    markdown += 'Exact final evidence: `' + evidence['retention']['path'] + '` SHA256 `' + evidence['retention']['sha256'] + '`. Machine-readable completion: `' + completion_ref['path'] + '` SHA256 `' + completion_ref['sha256'] + '`.\n\nThis record is authored before its enclosing commit. Git history supplies the eventual commit identity; no commit identity or completed commit action is fabricated here. Author validation is complete within the documented boundaries; the remaining source, tool, reviewer, learner and physical conditions stay explicit. No push, release acceptance or promotion is claimed.\n'
    outputs[BASE / 'completion_6decc40d.md'] = markdown.encode('utf8')
    prior_path = HISTORY / 'continuation_before.json'
    outputs[prior_path] = prior_bytes
    outputs[HISTORY / 'finalize_records.py'] = Path(__file__).read_bytes()
    continuation = dict(schema_version=2, state='author-validation-complete-with-documented-boundaries',
        recorded_at=now, input_sha256=SOURCE,
        active_supersession=dict(original=ref(CONTINUATION), preserved_predecessor=dict(path=prior_path.relative_to(ROOT).as_posix(),sha256=CONTINUATION_SHA),
            scope='Replace obsolete pending runtime summary only; complete previous record retained byte-for-byte.'),
        canonical_continuation='research/v5/expansion/followthrough/continuation.json', completion=completion_ref,
        active_fix_closures=closures, final_evidence=evidence, active_rows=6125, remaining_implementation_rows=7,
        distinct_remaining_boundaries=6, qualifications=qualifications,
        next='Resolve the exact remaining source, SDK, tool-defect and physical prerequisites before claiming their dimensions; obtain separate independent reviewer and learner evidence where required. Git history is authoritative for the enclosing commit identity.',
        release_accepted=False, promoted=False,
        commit=dict(record_timing='authored-before-enclosing-commit',
            identity_authority='Enclosing Git history supplies the eventual commit identity; this record embeds none.',
            commit_observed_by_writer=False), pushed=False)
    continuation_bytes = encoded(continuation)
    temporary = BASE / '.continuation_current.6decc40d.tmp'
    require(not temporary.exists(), 'Continuation temporary already exists')
    for destination in outputs:
        require(destination.is_relative_to(BASE) and destination.resolve() == destination and not destination.exists(), 'Final metadata destination must be fresh: ' + str(destination))
    require(sha(CONTINUATION) == CONTINUATION_SHA, 'Continuation changed during admission')

    # Fresh immutable records first; only the active continuation pointer is
    # replaced, after its exact predecessor has been retained successfully.
    for destination, payload in outputs.items():
        destination.parent.mkdir(parents=True, exist_ok=True)
        with destination.open('xb') as stream:
            stream.write(payload)
        require(destination.read_bytes() == payload, 'Final metadata write differs')
    require(sha(prior_path) == CONTINUATION_SHA and sha(CONTINUATION) == CONTINUATION_SHA, 'Continuation predecessor not preserved')
    with temporary.open('xb') as stream:
        stream.write(continuation_bytes)
    os.replace(temporary, CONTINUATION)
    for name, (_, digest) in ORIGINALS.items():
        require(sha(relative(BASE, name)) == digest, 'Original finding changed')
    print(json.dumps(dict(completion=ref(completion_path), continuation=ref(CONTINUATION), closures=closures,
                         accepted=False, promoted=False, commit_record='authored-before-enclosing-commit;identity-in-Git-history'), indent=2))


if __name__ == '__main__':
    main()
