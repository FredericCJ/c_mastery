"""Refresh explicit external origins; fail if any disposition or gate changes."""
from pathlib import Path
from datetime import datetime, timezone
import copy
import hashlib
import json
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[5]
OUT = ROOT / 'reviews/v5/expansion/integrated/final_author_run' / ('external_origins_' + datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'))
OUT.mkdir()
PACKET = ROOT / 'research/v5/expansion/external_obligations/completion_dispositions.json'
old_bytes = PACKET.read_bytes()
(OUT / 'before.json').write_bytes(old_bytes)
old = json.loads(old_bytes)
command = [sys.executable, '-X', 'utf8', 'tools/reconcile_external_obligations.py']
result = subprocess.run(command, cwd=ROOT, capture_output=True)
(OUT / 'generate.log').write_bytes(result.stdout + result.stderr)
assert result.returncode == 0, result.returncode
new = json.loads(PACKET.read_bytes())
diffs = []
left, right = copy.deepcopy(old), copy.deepcopy(new)
assert len(old['updates']) == len(new['updates']) == 58
for a, b, aa, bb in zip(old['updates'], new['updates'], left['updates'], right['updates']):
    assert a['id'] == b['id']
    for path, digest in b['artifact_sha256'].items():
        actual = hashlib.sha256((ROOT / path).read_bytes()).hexdigest()
        assert digest == actual, path
        if a['artifact_sha256'][path] != digest:
            diffs.append(dict(id=b['id'], path=path, previous=a['artifact_sha256'][path], current=digest))
    aa.pop('artifact_sha256')
    bb.pop('artifact_sha256')
assert left == right, 'External gates, source records or scope changed; requires substantive review'
assert len(diffs) == 4 and {d['path'] for d in diffs} == {'curriculum/v5/expansion/gaps.json'}, diffs
report = dict(passed=True, rows=58, command=command, exit_code=result.returncode,
              input_sha256={PACKET.relative_to(ROOT).as_posix(): hashlib.sha256(PACKET.read_bytes()).hexdigest()},
              previous_sha256=hashlib.sha256(old_bytes).hexdigest(), changed_artifact_bindings=diffs,
              source_records_and_all_six_dimensions_unchanged=True,
              scope='Same explicit open source/external gates and originating records; regenerated gap-container byte identities only. No new inspection, review, learner or board evidence.')
(OUT / 'guard.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf8')
print(json.dumps(dict(report=(OUT/'guard.json').relative_to(ROOT).as_posix(), passed=True, rows=58, changes=4)))
