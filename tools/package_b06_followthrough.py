"""Produce a current source-bound envelope from the detailed B06 dispositions."""
from pathlib import Path
import datetime,hashlib,json
from check_b06_followthrough import validate
ROOT=Path(__file__).resolve().parents[1]
def main():
 folder=ROOT/'research/v5/expansion/b06';canonical=folder/'completion_dispositions.json';data=json.loads(canonical.read_text(encoding='utf8'));errors,count=validate(data)
 if errors:raise SystemExit(chr(10).join(errors))
 reports=sorted({v['path'] for row in data['updates'] for b in row.get('criterion_bindings',[]) for v in b.get('author_validation',[])})
 assert count==len(reports)
 files={canonical,Path(__file__),ROOT/'tools/check_b06_followthrough.py',ROOT/'tools/check_b06_authoring.py',folder/'followup_resources.json',folder/'followup_sources.md',folder/'research_gate.json'}
 for rel in reports:
  p=ROOT/rel;files.add(p);report=json.loads(p.read_text(encoding='utf8'));pins=report.get('input_sha256',report.get('source_sha256',report.get('inspected_paths',{})));files.update(ROOT/s for s in pins)
 out=dict(schema_version=1,scope='B06 current author implementation envelope; detailed original77 dispositions preserved; global repository continues changing outside this scope',recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),canonical_dispositions=canonical.relative_to(ROOT).as_posix(),source_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(files)},current_reports=reports,updates=data['updates'],summary=data['summary'],evidence_limits=['Author executions and source-bound checks only; no external review, independent learner or physical observation inferred','Exact published-edition access gaps and physical gaps remain partial under their original IDs'])
 (folder/'completion_envelope.json').write_text(json.dumps(out,indent=2)+chr(10),encoding='utf8');print(json.dumps(dict(passed=True,original_ids=len(data['updates']),reports=count,pinned_files=len(files),partial_ids=data['summary']['implementation_partial_ids'])))
if __name__=='__main__':raise SystemExit(main())
