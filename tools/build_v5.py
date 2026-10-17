"""Build fifth-edition sources while preserving historical v2/v3/v4 receipts.

Every check binds to the source identity before and after it runs; a receipt
that does not match the final identity is stale and fails the publication.
"""
from datetime import datetime, timezone
import subprocess
import sys
from common import ROOT, dump
from v2_common import source_identity
from v5_common import HISTORICAL, RECEIPTS, V5

CHECKS = ['validate.py', 'check_fixtures.py', 'check_contract_guards.py', 'check_v3.py', 'check_v5.py',
          'check_learning_v4.py', 'check_release_v5.py', 'publish_v5.py', 'check_html.py', 'check_markdown.py']
OPTIONAL_CHECKS = ['check_authored_v5.py', 'check_vocabulary.py', 'check_terminology_ledger.py']
OPTIONAL_CHECKS += ['check_exercises_v5.py', 'check_exercise_workflow.py', 'check_graph_preservation_v5.py']
OPTIONAL_CHECKS += ['check_modern_coverage.py', 'check_v5_authoring_contracts.py']
OPTIONAL_CHECKS += ['check_legacy_assessments.py']
OPTIONAL_CHECKS += ['check_publication_links.py']


def run_tool(name, *arguments):
    return subprocess.run([sys.executable, '-X', 'utf8', str(ROOT / 'tools' / name), *arguments], cwd=ROOT)


def build():
    if not (V5 / 'baseline.json').is_file():
        raise SystemExit('Run tools/prepare_v5_baseline.py before the fifth-edition build')
    historical = {p: p.read_bytes() for folder in HISTORICAL for p in (ROOT / folder).rglob('*') if p.is_file()}
    receipts = ROOT / RECEIPTS
    try:
        if (ROOT / 'research/v5').is_dir() and run_tool('integrate_v5.py').returncode:
            raise SystemExit('integrate_v5 failed')
        if (ROOT / 'curriculum/v5_taxonomy.json').is_file() and run_tool('write_program.py').returncode:
            raise SystemExit('write_program failed')
        if run_tool('author_v5.py').returncode:
            raise SystemExit('author_v5 failed')
        from publish_v5 import prepare
        prepare()  # Materialize coverage before binding the checks to their inputs.
        records = []
        checks = CHECKS[:5] + [name for name in OPTIONAL_CHECKS if (ROOT / 'tools' / name).is_file()] + CHECKS[5:]
        for name in checks:
            before = source_identity()
            run = run_tool(name)
            after = source_identity()
            record = {'check': name, 'status': 'passed' if not run.returncode else 'failed', 'exit_code': run.returncode,
                      'started_input_sha256': before['sha256'], 'input_sha256': after['sha256']}
            records.append(record)
            dump(receipts / ('check_' + name.removesuffix('.py') + '.json'), record)
            if run.returncode:
                dump(receipts / 'publication.json', {'schema_version': 5, 'status': 'failed', 'passed': False, 'checks': records})
                raise SystemExit(run.returncode)
        final = source_identity()
        stale = [r['check'] for r in records
                 if r['input_sha256'] != final['sha256'] or r['started_input_sha256'] != final['sha256']]
        report = {'schema_version': 5, 'status': 'passed' if not stale else 'failed', 'passed': not stale,
                  'input_sha256': final['sha256'], 'recorded_at': datetime.now(timezone.utc).isoformat(),
                  'checks': records, 'stale_checks': stale,
                  'scope': 'Current fifth-edition publication, preserved fourth-edition identities, curriculum '
                           'structure and local learner workflow.'}
        dump(receipts / 'publication.json', report)
        dump(receipts / 'source_manifest.json', final)
        if stale:
            raise SystemExit('Source changed while validating: ' + str(stale))
    finally:
        for path, content in historical.items():
            path.write_bytes(content)


def main():
    from expansion_evidence import preserve_receipts
    with preserve_receipts(ROOT / RECEIPTS):
        build()


if __name__ == '__main__':
    main()
