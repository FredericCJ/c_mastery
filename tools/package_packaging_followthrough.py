"""Bind all original T22 ordinary/delayed obligations to current author evidence."""
import datetime,hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_packaging'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def module(p):
 spec=importlib.util.spec_from_file_location('packaging_bindings',p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def main():
 report=json.loads((HERE/'validation.json').read_text(encoding='utf8'));exports=json.loads((HERE/'exports.json').read_text(encoding='utf8'));files={pathlib.Path(__file__),HERE/'bindings.py',HERE/'sources.md',HERE/'resources.json',HERE/'research_gate.json',HERE/'register_sources.py',ROOT/'tools/check_packaging_followthrough.py',ROOT/'tools/integrate_packaging_followthrough.py'}
 for name,d in [('validation.json',report),('exports.json',exports)]:
  assert d['status']=='pass' and d['commands'] and all(x['passed'] for x in d['commands']);files.add(HERE/name)
  for key in ['source_sha256','outputs_sha256']:
   for p,h in d[key].items():assert sha(ROOT/p)==h,('stale',key,p)
  files|={ROOT/p for p in d['source_sha256']}
 commands={x['id'] for x in report['commands']};bindings=module(HERE/'bindings.py').BINDINGS;updates=[];objectives=set();receipt='research/v5/expansion/legacy_packaging/validation.json';public='labs/api_depth/library/CONTRACTS.md';analysis='assessor/api_depth/library/ANALYSIS.md'
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T22-*.json')):
  d=json.loads(path.read_text(encoding='utf8'));files.add(path)
  for c in d['criteria']:
   objectives.add(c['objective']);normal,changed,rationale=bindings[c['objective']];v=c['variants'][0]
   for delayed,selectors in [(False,normal),(True,changed)]:
    checks=set()
    for selector in selectors:
     matches={x for x in commands if x.startswith(selector) and not x.endswith('.compile')};assert matches,(c['objective'],selector);checks|=matches
    physical=c['objective']=='T22.01.b';dimensions={
     'teaching':dict(state='complete',evidence=[path.relative_to(ROOT).as_posix(),public]),'implementation':dict(state='complete',evidence=[public,analysis]),'author_validation':dict(state='passed',evidence=[receipt]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-observed' if physical else 'not-applicable-to-host-or-analysis-contract',evidence=[public])}
    updates.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,closure_rationale=rationale,artifacts=[public,analysis,'labs/api_depth/library/ARCHITECTURE.md','labs/api_depth/library/RELEASES.md'],author_validation=dict(report=receipt,checks=sorted(checks),scope='Author implementation/execution and explicit bounded analysis; not independent learner delay, external review, legal distribution approval or physical board evidence'),remaining_author_work=[],evidence_limits=['Original learner criteria and seven-day delayed tasks remain mandatory.','License and exact source-access gaps stay explicit.','Finite controlled builds, mutants and original histories do not establish universal compatibility or mastery.']))
 assert objectives==set(bindings) and len(updates)==60
 result=dict(schema_version=1,scope='T22 all30 original criteria and30 delayed author tasks',recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[receipt,'research/v5/expansion/legacy_packaging/exports.json'],updates=updates,summary=dict(criteria=30,delayed=30,commands=len(report['commands']),export_commands=len(exports['commands']),external_review='not-run',independent_learner='not-assessed'),evidence_limits=report['limits'])
 (HERE/'completion_dispositions.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(result['summary']))
if __name__=='__main__':main()
