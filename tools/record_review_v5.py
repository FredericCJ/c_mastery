"""Tracked recorder for independent review receipts (reviews/v5/review_*.json).

The reviewer writes an input file; this script validates it, pins every reviewed
path at its current hash, embeds the current publication check outcomes and
writes the receipt. It never resolves a finding on the reviewer's behalf.

Input shape:
  {"reviewer": "...", "scope": "...", "reviewed_paths": ["..."],
   "findings": [{"id": "F1", "severity": "low|medium|high|critical", "status": "open|resolved|closed",
                 "summary": "...", "resolution": "..."}],
   "checks": ["..."], "independent_validation": ["..."], "limitations": ["..."]}
"""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
from common import ROOT, dump, read_json, sha
from v2_common import source_identity
from v5_common import RECEIPTS

SEVERITIES = {'low', 'medium', 'high', 'critical'}
STATUSES = {'open', 'resolved', 'closed'}


def validate(value):
    errors = []
    for key in ('reviewer', 'scope', 'reviewed_paths', 'findings'):
        if not value.get(key) and key != 'findings':
            errors.append('Missing ' + key)
    if not isinstance(value.get('findings'), list):
        errors.append('findings must be a list')
    for finding in value.get('findings', []):
        if finding.get('severity') not in SEVERITIES or finding.get('status') not in STATUSES:
            errors.append('Finding needs a known severity and status: ' + str(finding.get('id')))
        if not finding.get('summary'):
            errors.append('Finding needs a summary: ' + str(finding.get('id')))
        if finding.get('status') in {'resolved', 'closed'} and not finding.get('resolution'):
            errors.append('Resolved/closed finding needs a resolution: ' + str(finding.get('id')))
    for relative in value.get('reviewed_paths', []):
        file = (ROOT / relative).resolve()
        if not file.is_relative_to(ROOT.resolve()) or not file.is_file():
            errors.append('Reviewed path is missing or escapes the repository: ' + relative)
    return errors


def observed_checks():
    result = {}
    for label, relative in [('html', 'reviews/html_checks.json'), ('markdown', 'reviews/markdown_checks.json'),
                            ('structure', RECEIPTS + '/structure.json'), ('publication', RECEIPTS + '/publication.json')]:
        path = ROOT / relative
        if path.is_file():
            record = read_json(path)
            result[label] = {k: record.get(k) for k in ('passed', 'status', 'input_sha256') if k in record}
            result[label]['sha256'] = sha(path)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('input')
    parser.add_argument('--output', default='review_delivery.json', help='Receipt name under reviews/v5')
    args = parser.parse_args()
    value = read_json(args.input)
    errors = validate(value)
    if errors:
        raise SystemExit('Invalid review input: ' + '; '.join(errors))
    identity = source_identity()
    unresolved = [f['id'] for f in value['findings'] if f['severity'] in {'medium', 'high', 'critical'} and f['status'] == 'open']
    record = {'schema_version': 5, 'status': 'passed' if not unresolved else 'failed', 'passed': not unresolved,
              'reviewed_at': datetime.now(timezone.utc).isoformat(), 'input_sha256': identity['sha256'],
              'reviewer': value['reviewer'], 'scope': value['scope'], 'findings': value['findings'],
              'unresolved_medium_or_higher': unresolved, 'checks': value.get('checks', []),
              'independent_validation': value.get('independent_validation', []),
              'observed_publication_checks': observed_checks(), 'limitations': value.get('limitations', []),
              'physical_observations': 'not-run', 'learner_mastery': 'not-assessed', 'learning_effectiveness': 'not-measured',
              'inspected_paths': {relative: sha(ROOT / relative) for relative in sorted(set(value['reviewed_paths']))}}
    dump(ROOT / RECEIPTS / args.output, record)
    print(args.output + ': ' + record['status'] + (' (unresolved: ' + ', '.join(unresolved) + ')' if unresolved else ''))
    return 0 if record['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
