"""Preserve original T25.06–.09 tasks and bind precise author dimensions."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_thread_atomics';CANON=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-03.json'
from research.v5.expansion.legacy_threads.author_support import attach
CASES=dict(zip(['T25.'+t+'.'+leaf for t in ['06','07','08','09'] for leaf in 'abc'],['graph','conflict','snapshot','operations','lockfree','closure','model','handoff','dependency','orders','retry','ownership']))
ARTIFACTS=['labs/thread_depth/atomics/CONTRACTS.md','assessor/thread_depth/atomics/ANALYSIS.md','assessor/thread_depth/atomics/publication.c','assessor/thread_depth/atomics/models.c','assessor/thread_depth/atomics/cas.c','assessor/thread_depth/atomics/handoff_model.py']
RECEIPT='research/v5/expansion/legacy_thread_atomics/validation.json'
REMAINING='Inspect exact final C17 and published C23 passages for the original T25.08.c edition comparison; retain draft/implementation distinctions until accessible.'
RATIONALES={
 'graph':'Named publication/initial-read and reverse-retirement graphs, all65536 four-node closure inputs checked by an independent traversal.',
 'conflict':'Explicit same-location/atomic/HB classifications and adjacent-bit-field changed examples; undefined conflicting expressions are analysis-only.',
 'snapshot':'Actual forced atomic-balance total90 compared with mutex exclusion, plus single-writer bounded SC sequence snapshot with nonwrap admission.',
 'operations':'Actual old-value/wrap atomic operations and initialized multi-field atomic aggregate transfers; no representation-copy shortcut.',
 'lockfree':'Actual native/i386 per-object width/alignment and aggregate queries, macro metadata, and explicit helper/context admission policy.',
 'closure':'Two compilers cross-build/link the complete M4 core without helpers and reject unprovided M0/wide/aggregate helper dependencies; assembly/maps retained without ISR or target-execution claims.',
 'model':'All32 restricted MP/SB/order/outcome combinations with explicit RF/HB arguments and independent SC permutation enumeration; mixed loads remain outside the SC order.',
 'handoff':'Actual1/2-reader reuse for2000 generations, reverse RMW-chain proof and finite pause-at-each-transition model with an early-retirement counterexample.',
 'dependency':'Actual release/consume and fence communication, direct-versus-control dependency classification and selected lowering; exact final-edition source comparison remains partial.',
 'orders':'All36 success/failure order pairs checked against14 explicit valid pairs, including actual release-success/relaxed-failure changed operations.',
 'retry':'Scripted spurious/mismatch/success with recomputed desired value, actual C CAS, and a strong one-attempt comparison.',
 'ownership':'Failed publication retains caller ownership; protected observed-node derivation and actual C pointer-CAS receiver-free terminal path preserve lifetime boundaries.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def checks_for(commands,case):
 if case=='closure':return [x for x in commands if '.cortex-' in x]
 result=[x for x in commands if x.endswith('.'+case) and '.learner.' not in x and '.mutation.' not in x]
 if case=='handoff':result+=['handoff.model']
 if case=='dependency':result += [x for x in commands if any(x.endswith('.'+s) for s in ['target_consume','target_acquire','target_control','target_release_fence','target_signal_fence'])]
 return result
def dimensions(partial=False):
 return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','partial' if partial else 'complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','partial' if partial else 'passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(CANON);snapshot=HERE/'PR-T25-03.before-atomics.json'
 if not snapshot.exists():snapshot.write_bytes(CANON.read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/thread_depth/atomics').rglob('*') if p.is_file())
 write(HERE/'attachment.json',dict(project=d['project'],key='thread_atomics_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_thread_atomics/resources.json',support='research/v5/expansion/legacy_thread_atomics/completion_dispositions.json',criterion_ids=[c['id'] for c in d['criteria']],scope='All twelve original atomic/CAS ordinary and changed packets; exact final-edition source comparison stays partial and external evidence is separate.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'];assert attach(copy.deepcopy(after))==after;write(CANON,after);print('PASS additive atomic successor, unchanged original tasks')
def package():
 d=read(CANON);before=read(HERE/'PR-T25-03.before-atomics.json');assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(d))==d
 report=read(HERE/'validation.json');exports=read(HERE/'exports.json');files={CANON,pathlib.Path(__file__),ROOT/'tools/check_thread_atomics.py',ROOT/'research/v5/expansion/legacy_threads/author_support.py',ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json'}
 files|={p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in [report,exports]:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for name,h in data[key].items():assert sha(ROOT/name)==h,('stale',name)
  files|={ROOT/p for p in data['source_sha256']}
 for p,h in report['external_sha256'].items():assert sha(pathlib.Path(p))==h,('external',p)
 commands=[c['id'] for c in report['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0];case=CASES[c['objective']];partial=case=='dependency'
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(partial),artifacts=ARTIFACTS,author_validation=dict(report=RECEIPT,checks=checks_for(commands,case)),closure_rationale=RATIONALES[case],remaining_author_work=[REMAINING] if partial else [],evidence_limits=report['limits']))
 parent=next(p for p in read(ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json')['gaps'] if p['id']=='LRUN-16')
 rows.append(dict(id='LRUN-16',original_gap=parent,dimensions=dimensions(True),artifacts=ARTIFACTS,author_validation=dict(report=RECEIPT,checks=list(dict.fromkeys(x for case in CASES.values() for x in checks_for(commands,case)))),closure_rationale='All twelve original ordinary/changed author mechanisms have code or explicit analysis plus independent checks; exact final-edition comparison is the remaining source obligation.',remaining_author_work=[REMAINING],evidence_limits=report['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Twelve exact criterion IDs, twelve original delayed IDs and existing parent LRUN-16; author dimensions with explicit source partials',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_thread_atomics/exports.json'],updates=rows,remaining_parents=['LRUN-17: T25.10–.12 progress, performance and broader validation remain separate.','LRUN-02: exact final C17/C23 source obligations remain open.'],summary=dict(criteria=12,delayed=12,parents=1,source_partial_rows=3,commands=len(report['commands']),export_commands=len(exports['commands']))));print('PASS atomic package25 original updates,3 source partials')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
