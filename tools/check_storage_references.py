"""Require exact tagged format, derivation, retirement and unchanged-output evidence."""
import argparse,copy,json,subprocess,sys
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/expansion/legacy_storage_followthrough/references_validation.json'
EXPORT='research/v5/expansion/legacy_storage_followthrough/references_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
ANCHORS={'exercise','persistence','reuse','retirement','stale','rollback'}
CONTROL={1:'stale',2:'retirement',3:'rollback'}
TRACE='9ebf7fd8c3e30d1bcdea68195edd948fbe4a1e7df534ddf2c71e6ea072294522'
def check_export(e,files=True):
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied']
 assert e['input_sha256'] and e['exported_artifacts'] and e['explicit_actual_provider']=='assessor/storage_followthrough/references/refs.c'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==8 and e['projects']==['PR-T47-05']
 failed=[r for r in e['executions'] if r['exit_code']!=0];assert len(failed)==2 and {r['required_marker'] for r in failed}=={'TREE_CHECK','Supply the explicit reference fork assessor source'}
 assert all(r['expected_exit']==0 and r['exit_code']==0 or r['expected_exit'] is None and r['exit_code'] not in [0,None] for r in e['executions'])
 if files:
  for p,h in e['input_sha256'].items():assert sha(ROOT/p)==h
  for p,h in e['exported_artifacts'].items():assert p in e['input_sha256'] and h==e['input_sha256'][p] and 'assessor' not in p.split('/')
  for r in e['executions']:
   assert sha(ROOT/r['log'])==r['log_sha256']
   if r['required_marker']:assert r['required_marker'] in (ROOT/r['log']).read_text(encoding='utf8')
 return True
def check(d,files=True):
 base(d);assert len(d['profiles'])==6 and {p['profile'] for p in d['profiles']}==PROFILES
 for p in d['profiles']:
  assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==3
  assert {r['test'] for r in p['controls']}=={f'storage_followthrough.mutation.{a}.{i}' for i,a in CONTROL.items()} and all(r['semantic_rejection'] for r in p['controls'])
 assert len(d['learner_stub_rejections'])==6 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 assert len(d['observations'])==6 and {r['profile'] for r in d['observations']}==PROFILES
 for row in d['observations']:
  r=row['result'];assert row['kind']=='independent-oracle' and r['passed'] and (r['histories'],r['transitions'],r['trace_sha256'])==(3495,10725,TRACE)
  assert r['canonical_pages']==2 and r['foreign_rejection']==1
 needed={'assessor/storage/store.c','assessor/storage_followthrough/tree/tree.c','labs/storage_followthrough/tree/include/store.h','labs/storage/FORMAT.md','assessor/storage_followthrough/references/refs.c','labs/storage_followthrough/references/include/store.h','labs/storage_followthrough/references/include/refs.h','labs/storage_followthrough/references/CONTRACTS.md','labs/storage_followthrough/references/tests.c','tools/verify_storage_references.py','tools/author_storage_references.py'};assert needed<=set(d['input_sha256'])
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for c in d['compiler_dependency_closures']:
  assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
  assert 'assessor/storage/store.c' not in c['repository_inputs'] and 'assessor/storage_followthrough/tree/tree.c' not in c['repository_inputs']
 rows=d['executions'];mutants=[r for r in rows if 'storage-references-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']];derive=[r for r in rows if '--check' in r['command']]
 assert len(rows)==104 and len(mutants)==18 and all(r['exit_code']==1 for r in mutants) and len(learners)==6 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 assert len(derive)==1 and derive[0]['exit_code']==0 and any('author_storage_references.py' in p for p in derive[0]['command'])
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 if files:
  bindings(d)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert 'TREE_CHECK' in text and not any(s in text for s in ['AddressSanitizer','runtime error:'])
  for r in ctests:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in text and all('REF '+a+' validated' in text for a in ['reuse','retirement','stale','rollback'])
   assert 'TREE exercise validated capacity=7 all-leaf-and-internal-events' in text and 'TREE persistence validated commit-open-delete-reuse' in text
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):
    commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
  for r in json.loads((ROOT/'research/v5/expansion/legacy_storage_followthrough/references_resources.json').read_text())['resources']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
  p=subprocess.run([sys.executable,str(ROOT/'tools/author_storage_references.py'),'--check'],cwd=ROOT,text=True,capture_output=True);assert p.returncode==0,p.stdout+p.stderr
 return True
def selftest(d,e):
 edits=[lambda a:a.update(passed=False),lambda a:a['profiles'].pop(),lambda a:a['profiles'][0]['controls'].pop(),lambda a:a['learner_stub_rejections'].pop(),lambda a:a['observations'][0]['result'].update(histories=1),lambda a:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a:a.update(input_sha256={}),lambda a:a.update(learner_mastery='passed'),lambda a:a['compiler_dependency_closures'].pop(),lambda a:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a:a['observations'].pop(),lambda a:a.update(source_stable=False),lambda a:a.update(physical_validation='passed'),lambda a:a['observations'][0]['result'].update(transitions=0),lambda a:a['profiles'][0].update(anchors=['reuse']),lambda a:a.update(external_inputs_stable=False),lambda a:a['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda a:a.update(independent_review='passed'),lambda a:a['observations'][0]['result'].update(canonical_pages=0),lambda a:a['observations'][0]['result'].update(foreign_rejection=0),lambda a:next(r for r in a['executions'] if '--check' in r['command']).update(exit_code=1)]
 for edit in edits:
  a=copy.deepcopy(d);edit(a)
  try:check(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported evidence')
 for edit in [lambda a:a.update(private_sources_copied=True),lambda a:a['executions'].pop(),lambda a:a.update(exported_artifacts={}),lambda a:a.update(learner_mastery='passed')]:
  a=copy.deepcopy(e);edit(a)
  try:check_export(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported export')
 return len(edits)+4
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());e=json.loads((ROOT/EXPORT).read_text());check(d);check_export(e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,e) if a.self_test else 0,report_sha256=sha(ROOT/REPORT),export_sha256=sha(ROOT/EXPORT))))
