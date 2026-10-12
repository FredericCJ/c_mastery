"""Terminology ledger: accepted teaching prose may differ only through logged, reviewed hunks.

curriculum/v5/terminology_ledger.json lists entries
  {"file": "curriculum/modules/T25.md", "term": "data race", "before": "...", "after": "...",
   "authority": {"resource_id": "...", "locator": "..."}, "rationale": "...", "reviewer": "..."}
Reverse-applying a file's hunks (replacing each `after` with its `before`, in
reverse order) must reproduce the byte-exact accepted file recorded in
curriculum/v5/baseline.json. No git access is needed, so the check also runs in
an extracted-source rebuild.
"""
from __future__ import annotations
import hashlib
from common import ROOT, dump, read_json
from v5_common import RECEIPTS, V5

LEDGER = V5 / 'terminology_ledger.json'


def reverse_applies(relative, expected_sha, ledger):
    path = ROOT / relative
    if not path.is_file():
        return False
    text = path.read_text(encoding='utf-8', newline='')
    hunks = [h for h in ledger.get('entries', []) if h['file'] == relative]
    if not hunks:
        return False
    for hunk in reversed(hunks):
        if not hunk.get('after') or hunk['after'] not in text:
            return False
        text = text.replace(hunk['after'], hunk['before'], 1)
    return hashlib.sha256(text.encode('utf-8')).hexdigest() == expected_sha


def main():
    baseline = read_json(V5 / 'baseline.json')
    ledger = read_json(LEDGER) if LEDGER.is_file() else {'schema_version': 5, 'entries': []}
    errors = []
    files = sorted({h['file'] for h in ledger['entries']})
    for hunk in ledger['entries']:
        for key in ('file', 'term', 'before', 'after', 'authority', 'rationale', 'reviewer'):
            if not hunk.get(key):
                errors.append('Ledger entry missing ' + key + ': ' + str(hunk.get('file')))
        if hunk.get('before') == hunk.get('after'):
            errors.append('Ledger entry changes nothing: ' + str(hunk.get('file')))
    for relative in files:
        expected = baseline['teaching'].get(relative)
        if expected is None:
            errors.append('Ledger names a file outside the accepted teaching set: ' + relative)
        elif not reverse_applies(relative, expected, ledger):
            errors.append('Ledger hunks do not reverse-apply to the accepted bytes: ' + relative)
    report = {'schema_version': 5, 'status': 'passed' if not errors else 'failed', 'passed': not errors,
              'files': files, 'hunks': len(ledger['entries']), 'errors': errors,
              'scope': 'Every difference between accepted teaching prose and the current text is a logged, reviewed terminology hunk.'}
    dump(ROOT / RECEIPTS / 'terminology_ledger.json', report)
    print(report)
    if errors:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
