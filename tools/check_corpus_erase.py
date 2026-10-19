"""Adversarial exact-identity, execution and export evidence guard."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate
from expansion_evidence import require_batch_research
from corpus_export_bindings import errors as export_errors
HERE=ROOT/'research/v5/expansion/corpus_erase';IDS=['X-T14-017']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def check(data):
 errors=[];rows=data.get('updates',[]);expected={k for i in IDS for k in [i,i+'-DELAYED',*[i+'-R'+str(n) for n in [1,2,3]]]}
 if len(rows)!=5 or {r['id'] for r in rows}!=expected:errors.append('exact identities')
 if not data.get('source_sha256'):errors.append('missing pins')
 for n,h in data.get('source_sha256',{}).items():
  if not (ROOT/n).is_file() or sha(ROOT/n)!=h:errors.append('stale '+n)
 try:require_batch_research(HERE)
 except (ValueError,RuntimeError,AssertionError) as e:errors.append(str(e))
 for row in rows:
  ident=row['parent_exercise'];meta,body=parse_brief(ROOT/'curriculum/v5/exercises'/f'{ident}.md')
  try:validate(meta,body)
  except ValueError as e:errors.append(str(e))
  if row['parent_project']!=meta['project'] or row['original_delayed_variant']!=ident+'-DELAYED' or row['minimum_days']!=7:errors.append('binding/delay')
  if not row['artifacts'] or not row['evidence_limits']:errors.append('missing artifact/limits')
  for k,s in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:
   if row['dimensions'][k]!=dict(state=s,evidence=[]):errors.append('inflated external evidence')
 reports=data.get('current_reports',[])
 if len(reports)!=2:return errors+['missing reports']
 campaign=data.get('_campaign',json.loads((ROOT/reports[0]).read_text()));exports=data.get('_exports',json.loads((ROOT/reports[1]).read_text()))
 for report in [campaign,exports]:
  if report.get('passed') is not True:errors.append('failed report')
  for key in ['source_sha256','inspected_paths','outputs_sha256']:
   for n,h in report.get(key,{}).items():
    if not (ROOT/n).is_file() or sha(ROOT/n)!=h:errors.append('stale report pin')
  for c in report.get('commands',report.get('executions',[])):
   if 'log' in c and sha(ROOT/c['log'])!=c['log_sha256']:errors.append('stale log')
 anchors={'corpus_erase.compact'};profiles=campaign.get('profiles',[])
 if len(profiles)!=6 or {(p['compiler'],p['profile']) for p in profiles}!={(c,p) for c in ['gcc','clang'] for p in ['debug','release','asan']}:errors.append('profiles')
 for p in profiles:
  if not p['passed'] or set(p['anchors'])!=anchors:errors.append('anchors')
  for anchor in anchors:
   if not {anchor.replace('corpus_erase.','corpus_erase.mutation.')+'.'+str(i) for i in [1,2]}<=set(p['compiled_mutant_controls']):errors.append('mutants')
 ctests=[c for c in campaign.get('executions',[]) if c['command'][0]=='ctest' and not '--show-only=json-v1' in c['command'] and c['exit_code']==0]
 if len(ctests)!=6 or any('corpus_erase.cpp_comparison' not in (ROOT/c['log']).read_text() for c in ctests):errors.append('actual C++ comparison')
 if not campaign.get('scope_source_stable'):errors.append('unstable campaign')
 if {r['test'] for r in campaign.get('learner_stub_rejections',[]) if r['rejected']}!=anchors:errors.append('stub')
 oracle=[c for c in campaign.get('executions',[]) if '--driver' in c['command'] and any('corpus_erase/oracle.py' in str(s) for s in c['command'])]
 if len(oracle)!=6:errors.append('independent model')
 for c in oracle:
  result=json.loads((ROOT/c['log']).read_text().strip())
  if c['exit_code']!=0 or result.get('cases')!=4097:errors.append('model scope')
 errors+=export_errors(exports,IDS)
 if {r['exercise'] for r in exports.get('exports',[]) if r['assessor_files_absent'] and r['semantic_rejection'] and r['explicit_parent_and_family_providers']}!=set(IDS):errors.append('export scope')
 if len(exports.get('commands',[]))!=10 or not all(c['passed'] for c in exports.get('commands',[])):errors.append('actual exports')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report');a=p.parse_args();data=json.loads((HERE/'completion_dispositions.json').read_text());errs=check(data);controls=[]
 if a.self_test:
  cases=[('missing-root',lambda d:d['updates'].pop(0)),('missing-delayed',lambda d:d['updates'].pop()),('stale',lambda d:d['source_sha256'].update({next(iter(d['source_sha256'])):'0'*64})),('binding',lambda d:d['updates'][0].update(parent_project='PR-T15-01')),('delay',lambda d:d['updates'][0].update(minimum_days=0)),('limits',lambda d:d['updates'][0].update(evidence_limits=[]))]
  for k in ['external_review','independent_learner','physical_observation']:cases.append(('inflated-'+k,lambda d,k=k:d['updates'][0]['dimensions'][k].update(state='passed')))
  for name,fn in cases:
   d=copy.deepcopy(data);fn(d);controls.append(dict(control=name,rejected=bool(check(d))))
  for name,fn in [('profile',lambda r:r['profiles'].pop()),('mutant',lambda r:r['profiles'][0]['compiled_mutant_controls'].pop()),('stub',lambda r:r['learner_stub_rejections'].pop()),('oracle',lambda r:r.update(executions=[])),('unstable',lambda r:r.update(scope_source_stable=False))]:
   d=copy.deepcopy(data);d['_campaign']=json.loads((ROOT/data['current_reports'][0]).read_text());fn(d['_campaign']);controls.append(dict(control=name,rejected=bool(check(d))))
  for name,fn in [('assessor-leak',lambda r:r['exports'][0].update(assessor_files_absent=False)),('selected-contract',lambda r:r['selected_catalog_sha256'].update({'X-T14-017':'0'*64})),('missing-export',lambda r:r['commands'].pop())]:
   d=copy.deepcopy(data);d['_exports']=json.loads((ROOT/data['current_reports'][1]).read_text());fn(d['_exports']);controls.append(dict(control=name,rejected=bool(check(d))))
  errs+=['vacuous '+r['control'] for r in controls if not r['rejected']]
 result=dict(passed=not errs,updates=len(data['updates']),negative_controls=controls,errors=errs,limits='Bounded author execution only; independent and physical evidence remain separate.')
 if a.report:(ROOT/a.report).write_text(json.dumps(result,indent=2)+'\n')
 print(json.dumps(result,indent=2));raise SystemExit(bool(errs))
if __name__=='__main__':main()
