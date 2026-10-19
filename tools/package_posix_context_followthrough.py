"""Bind exact new T23 author tasks; preserve all original identities and delays."""
import hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_posix_context'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def bindings():
 spec=importlib.util.spec_from_file_location('pcb',HERE/'bindings.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def main():
 m=bindings();report=json.loads((HERE/'validation.json').read_text(encoding='utf8'));exports=json.loads((HERE/'exports.json').read_text(encoding='utf8'));files={pathlib.Path(__file__),ROOT/'tools/check_posix_context_followthrough.py',ROOT/'tools/integrate_posix_context_followthrough.py',HERE/'bindings.py',HERE/'resources.json',HERE/'sources.md',HERE/'research_gate.json',HERE/'register_sources.py',HERE/'author_support.py',HERE/'launcher_binding_successor.json',ROOT/'tools/rebind_posix_launcher_metadata.py'}
 for name,d in [('validation.json',report),('exports.json',exports)]:
  assert d['status']=='pass' and d['commands'] and all(x['passed'] for x in d['commands']);files.add(HERE/name)
  for key in ['source_sha256','outputs_sha256']:
   for p,h in d[key].items():assert sha(ROOT/p)==h,('stale',key,p)
  files|={ROOT/p for p in d['source_sha256']}
 spec=importlib.util.spec_from_file_location('pcrebind',ROOT/'tools/rebind_posix_launcher_metadata.py');rebind=importlib.util.module_from_spec(spec);spec.loader.exec_module(rebind)
 preserved=json.loads((HERE/'launcher_binding_successor.json').read_text(encoding='utf8'));assert not rebind.validate(preserved)
 files|={ROOT/p for p in preserved['source_sha256']}
 prior='reviews/v5/expansion/legacy_posix_followthrough/launch_validation.json';files.add(ROOT/prior)
 commands={x['id'] for x in report['commands']};rows=[];receipt='research/v5/expansion/legacy_posix_context/validation.json'
 for number in ['01','05','06']:
  path=ROOT/f'curriculum/v5/expansion/legacy_assessments/PR-T23-{number}.json';d=json.loads(path.read_text(encoding='utf8'));files.add(path)
  for c in d['criteria']:
   if c['objective'] not in m.BINDINGS:continue
   normal,changed,why=m.BINDINGS[c['objective']];v=c['variants'][0]
   for delayed,kind in [(False,normal),(True,changed)]:
    checks=m.checks(commands,kind);assert checks and set(checks)<=commands
    dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',[path.relative_to(ROOT).as_posix(),m.ARTIFACTS[0]]),('implementation','complete',m.ARTIFACTS),('author_validation','passed',[receipt]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-observed' if c['objective'].startswith(('T23.12','T23.13')) else 'not-applicable-to-declared-host-or-analysis-contract',m.ARTIFACTS[:3])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],minimum_days=v['minimum_days'],original_delayed_variant=v['id'],dimensions=dims,closure_rationale=why,artifacts=m.ARTIFACTS,author_validation=dict(report=receipt,checks=checks,evidence_kind='Authored analysis plus selected independent logical oracle' if kind=='analysis' else 'Actual original target execution and/or explicitly labeled clock/capability models'),remaining_author_work=[],evidence_limits=report['limits']))
    if c['objective']=='T23.02.a':rows[-1]['preserved_evidence']=dict(binding='research/v5/expansion/legacy_posix_context/launcher_binding_successor.json',report=prior,scope='Unchanged launcher/error-channel execution plus the new owned-child hard-wait recovery adapter; no execution rerun or independent learner inference.')
 assert len(rows)==30
 original_path=ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json';files.add(original_path);original=next(g for g in json.loads(original_path.read_text(encoding='utf8'))['gaps'] if g['id']=='LRUN-08')
 rows.append(dict(id='LRUN-08',original_record=original,dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',[m.ARTIFACTS[0]]),('implementation','complete',m.ARTIFACTS),('author_validation','passed',[receipt]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-observed',[m.ARTIFACTS[2]])]},closure_rationale='Original learner/reference protocol, stage failures, process cuts and independently checked abstract fixture implemented and executed; physical durability remains unobserved.',artifacts=m.ARTIFACTS,author_validation=dict(report=receipt,checks=m.checks(commands,'storage')+['models']),evidence_limits=report['limits']))
 result=dict(schema_version=1,scope='Fourteen previously uncovered T23 criteria plus one prior partial, fifteen exact delayed author tasks and original LRUN-08; existing POSIX successor packages unchanged',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[receipt,'research/v5/expansion/legacy_posix_context/exports.json',prior],updates=rows,remaining_cross_scope=[dict(id='LRUN-09',completed_here=['T23.13','T23.14'],remaining=['T24.09 actual accumulated timerfd/eventfd counts'],reason='No whole-parent closure from only T23 child evidence.')],summary=dict(criteria=15,delayed=15,parent_ids=1,commands=len(report['commands']),export_commands=len(exports['commands'])))
 (HERE/'completion_dispositions.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(result['summary']))
if __name__=='__main__':main()
