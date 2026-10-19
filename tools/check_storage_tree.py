"""Fail closed on missing policies, intended defects, oracle or source bindings."""
import argparse,copy,json
from check_replicated_replay_depth import ROOT,base,bindings,sha
REPORT='research/v5/expansion/legacy_storage_followthrough/tree_validation.json'
EXPORT='research/v5/expansion/legacy_storage_followthrough/tree_export_validation.json'
PROFILES={a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']}
CASES={'exercise','minimum','separator','shape','mixed','persistence'}
ANCHORS={f'cap{cap}.{policy}.{case}' for cap in [7,3] for policy in ['path','full'] for case in CASES}
CONTROLS={f'storage_followthrough.mutation.cap{cap}.{case}.{i}' for cap in [7,3] for i,case in enumerate(['exercise','minimum','exercise','separator','shape'],1)}
TRACE='9ebf7fd8c3e30d1bcdea68195edd948fbe4a1e7df534ddf2c71e6ea072294522'
def check_export(e,files=True):
 assert e['passed'] and e['status']=='passed' and e['source_stable'] and not e['private_sources_copied']
 assert e['input_sha256'] and e['exported_artifacts'] and e['explicit_actual_provider']=='assessor/storage_followthrough/tree/tree.c'
 assert e['physical_validation']=='not-run' and e['independent_review']=='not-run' and e['learner_mastery']=='not-assessed'
 assert len(e['executions'])==16 and e['projects']==['PR-T47-04','PR-T47-05']
 failed=[r for r in e['executions'] if r['exit_code']!=0]
 assert len(failed)==4 and {r['required_marker'] for r in failed}=={'TREE_CHECK','Supply the explicit tree fork assessor source'}
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
  assert p['passed'] and set(p['anchors'])==ANCHORS and len(p['controls'])==10
  assert {r['test'] for r in p['controls']}==CONTROLS and all(r['semantic_rejection'] for r in p['controls'])
 assert len(d['learner_stub_rejections'])==24 and {r['test'] for r in d['learner_stub_rejections']}==ANCHORS and all(r['semantic_rejection'] for r in d['learner_stub_rejections'])
 assert len(d['observations'])==6 and {r['profile'] for r in d['observations']}==PROFILES
 for row in d['observations']:
  r=row['result'];assert row['kind']=='independent-oracle' and r['passed'] and (r['histories'],r['transitions'],r['trace_sha256'])==(3495,10725,TRACE)
  assert r['canonical_pages']==4 and r['cross_format_rejections']==4 and len(r['profiles'])==4
  for offset,cap in [(0,7),(2,3)]:
   local,full=r['profiles'][offset:offset+2];assert local['capacity']==full['capacity']==cap
   for k in ['leaf_splits','internal_splits','left','right','merge_left','merge_right','shrink','store_bytes']:assert local[k]==full[k] and local[k]>0
   assert local['minimum_visits']>0 and local['repair_nodes']>0 and local['full_visits']==0
   assert full['full_visits']>0 and full['minimum_visits']==0 and full['repair_nodes']==0
 needed={'assessor/storage/store.c','labs/storage/include/store.h','labs/storage/FORMAT.md','assessor/storage_followthrough/tree/tree.c','labs/storage_followthrough/tree/include/store.h','labs/storage_followthrough/tree/include/tree.h','labs/storage_followthrough/tree/CONTRACTS.md','labs/storage_followthrough/tree/tests.c','tools/verify_storage_tree.py'};assert needed<=set(d['input_sha256'])
 assert {r['profile'] for r in d['compiler_dependency_closures']}==PROFILES|{'learner'}
 for c in d['compiler_dependency_closures']:
  assert c['repository_inputs'] and set(c['repository_inputs'])<=set(d['input_sha256']) and c['external_inputs']
  assert 'assessor/storage/store.c' not in c['repository_inputs'] # ancestry is pinned, not a false ABI import
 rows=d['executions'];mutants=[r for r in rows if 'storage-tree-mutant-' in r['command'][0]];learners=[r for r in rows if '-R' in r['command']];ctests=[r for r in rows if '--verbose' in r['command']]
 assert len(rows)==163 and len(mutants)==60 and all(r['exit_code']==1 for r in mutants) and len(learners)==24 and all(r['exit_code'] not in [0,None] for r in learners) and len(ctests)==6 and all(r['exit_code']==0 for r in ctests)
 allowed={id(r) for r in mutants+learners};assert all(r['exit_code']==0 or id(r) in allowed for r in rows)
 if files:
  bindings(d)
  for r in mutants+learners:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert 'TREE_CHECK' in text and not any(s in text for s in ['AddressSanitizer','runtime error:'])
  for r in ctests:
   text=(ROOT/r['log']).read_text(encoding='utf8');assert '100% tests passed' in text and all('TREE '+a+' validated' in text for a in CASES)
   assert 'TREE exercise validated capacity=3 all-leaf-and-internal-events' in text and 'TREE exercise validated capacity=7 all-leaf-and-internal-events' in text
  for r in d['generated_artifacts']:
   if r['path'].endswith('compile_commands.json'):
    commands=json.loads((ROOT/r['path']).read_text());assert commands and all('-std=c17' in c['command'] for c in commands)
    assert all(any(f'-DTREE_CAPACITY={cap}' in c['command'] and f'-DTREE_FULL_REFRESH={full}' in c['command'] for c in commands) for cap in [3,7] for full in [0,1])
  for r in json.loads((ROOT/'research/v5/expansion/legacy_storage_followthrough/tree_resources.json').read_text())['resources']:assert sha(ROOT/r['url'])==r['inspection']['local_sha256']
 return True
def selftest(d,e):
 edits=[lambda a:a.update(passed=False),lambda a:a['profiles'].pop(),lambda a:a['profiles'][0]['controls'].pop(),lambda a:a['learner_stub_rejections'].pop(),lambda a:a['observations'][0]['result'].update(histories=1),lambda a:a['observations'][0]['result'].update(trace_sha256='wrong'),lambda a:a.update(input_sha256={}),lambda a:a.update(learner_mastery='passed'),lambda a:a['compiler_dependency_closures'].pop(),lambda a:next(r for r in a['executions'] if 'mutant-' in r['command'][0]).update(exit_code=-11),lambda a:a['observations'].pop(),lambda a:a.update(source_stable=False),lambda a:a.update(physical_validation='passed'),lambda a:a['observations'][0]['result'].update(transitions=0),lambda a:a['profiles'][0].update(anchors=['exercise']),lambda a:a.update(external_inputs_stable=False),lambda a:a['profiles'][0]['controls'][0].update(semantic_rejection=False),lambda a:a.update(independent_review='passed'),lambda a:a['observations'][0]['result']['profiles'][1].update(full_visits=0),lambda a:a['observations'][0]['result'].update(cross_format_rejections=0)]
 for edit in edits:
  a=copy.deepcopy(d);edit(a)
  try:check(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported evidence')
 export_edits=[lambda a:a.update(private_sources_copied=True),lambda a:a.update(explicit_actual_provider='assessor/storage/store.c'),lambda a:a['executions'].pop(),lambda a:a.update(learner_mastery='passed')]
 for edit in export_edits:
  a=copy.deepcopy(e);edit(a)
  try:check_export(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported export')
 return len(edits)+len(export_edits)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());e=json.loads((ROOT/EXPORT).read_text());check(d);check_export(e);print(json.dumps(dict(passed=True,negative_controls=selftest(d,e) if a.self_test else 0,report_sha256=sha(ROOT/REPORT),export_sha256=sha(ROOT/EXPORT))))
