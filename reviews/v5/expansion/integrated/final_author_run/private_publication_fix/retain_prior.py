"""Retain current author receipts before the publication correction reruns."""
from pathlib import Path
import hashlib
import json
import shutil

ROOT = Path(__file__).resolve().parents[6]
OUT = Path(__file__).resolve().parent
INTEGRATED = ROOT/'reviews/v5/expansion/integrated'
prior = json.loads((INTEGRATED/'final_author_run/campaigns_20261018T210309585091Z/retained_receipts/manifest.json').read_text(encoding='utf8'))
names = {row['original'] for row in prior['files']}
names.update(p.relative_to(ROOT).as_posix() for p in INTEGRATED.glob('*.json'))
names.update('build/final_candidate_logs/20261019T030129449614Z-'+str(i)+'.log' for i in range(2))
rows = []
for name in sorted(names):
    source = ROOT/name
    if not source.is_file():
        raise ValueError('Prior receipt missing: '+name)
    destination = OUT/'retained_receipts/files'/name
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        raise ValueError('Never replace retained bytes: '+str(destination))
    shutil.copyfile(source, destination)
    with source.open('rb') as stream:
        digest = hashlib.file_digest(stream, 'sha256').hexdigest()
    with destination.open('rb') as stream:
        assert hashlib.file_digest(stream, 'sha256').hexdigest() == digest
    rows.append(dict(original=name, retained=destination.relative_to(ROOT).as_posix(),
                     sha256=digest, bytes=source.stat().st_size))
report = dict(source_identity='421460b61fe06a24168589f6ef0ffa960117d725231e6bbf5be12069113b5be1',
              files=rows, files_count=len(rows), bytes=sum(row['bytes'] for row in rows),
              scope='Exact prior author receipts and raw results; original identities retained, not current-source execution.')
(OUT/'retained_receipts/manifest.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf8')
print(json.dumps(dict(files=len(rows), bytes=report['bytes'])))
