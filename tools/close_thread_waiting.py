"""Additive original T25.03–.05 binding and dimensional author evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_thread_waiting';CANON=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-02.json'
from research.v5.expansion.legacy_threads.author_support import attach
CASES=dict(zip(['T25.'+t+'.'+leaf for t in ['03','04','05'] for leaf in 'abc'],['bank','order','lifecycle','queue','wake','deadline','barrier','fairness','permit']))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def checks_for(commands,case):
 cases=[case]+(['permit'] if case=='barrier' else [])
 return [x for x in commands if any(x.endswith('.'+c) for c in cases) and '.learner.' not in x]+([x for x in commands if x.endswith('.async-disassembly')] if case=='permit' else [])
def integrate():
 before=read(CANON);snapshot=HERE/'PR-T25-02.before-waiting.json'
 if not snapshot.exists():snapshot.write_bytes(CANON.read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/thread_depth/waiting').rglob('*') if p.is_file())
 write(HERE/'attachment.json',dict(project=before['project'],key='thread_waiting_successor',supplied_artifacts=public,shared_contracts=['labs/thread_depth/waiting/CONTRACTS.md'],source_register='research/v5/expansion/legacy_thread_waiting/resources.json',support='research/v5/expansion/legacy_thread_waiting/completion_dispositions.json',criterion_ids=[c['id'] for c in before['criteria']],scope='Nine original transaction/predicate/primitive tasks and exact changed-condition packets; source-author execution is separate from independent learner/reviewer evidence.'))
 after=attach(copy.deepcopy(before));assert before['criteria']==after['criteria'] and before['baseline_binding']==after['baseline_binding'];assert attach(copy.deepcopy(after))==after;write(CANON,after);print('PASS additive predicate attachment')
def dimensions(artifacts,receipt):
 return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',[receipt]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-applicable-to-declared-host-contract',[])]}
def package():
 d=read(CANON);before=read(HERE/'PR-T25-02.before-waiting.json');assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(d))==d
 report=read(HERE/'validation.json');exports=read(HERE/'exports.json');files={CANON,pathlib.Path(__file__),ROOT/'tools/check_thread_waiting.py',ROOT/'research/v5/expansion/legacy_threads/author_support.py',ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json'}
 files|={p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in [report,exports]:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for name,h in data[key].items():assert sha(ROOT/name)==h,('stale',name)
  files|={ROOT/p for p in data['source_sha256']}
 commands=[x['id'] for x in report['commands']];receipt='research/v5/expansion/legacy_thread_waiting/validation.json';rows=[];artifacts=['labs/thread_depth/waiting/CONTRACTS.md','assessor/thread_depth/waiting/ANALYSIS.md']
 rationales={
 'bank':'Forced split transaction and complete-lock repair, actual SC atomic snapshot reader/writer plus nonwrap argument.',
 'order':'Actual opposite acquisitions and callback self-edge via trylock; global-key repair, quiescent storage reuse and key exhaustion.',
 'lifecycle':'Five actual initialized-prefix fault cases, reference/admission root, close rejection and quiescent core retirement.',
 'queue':'Actual ring fixture, drain-close ownership and two enrolled consumers competing for one item.',
 'wake':'Signal before enrollment, actual empty broadcast, safe if-path model and controlled competing consumer theft with predicate recheck.',
 'deadline':'Repeated identical monotonic absolute deadline, actual close/timeout competitions and deferred cancellation cleanup after wait.',
 'barrier':'Actual three-worker two-phase publication and explicit departed-participant break, with original primitive comparison/rejected traces.',
 'fairness':'Three repeatable reader/permit/writer policy lassos with safety invariants and explicit limits on host scheduling inference.',
 'permit':'Original capacity3 token pool, once/read-write comparisons and actual async cancellation confined to disassembly-audited compute region.'}
 for c in d['criteria']:
  v=c['variants'][0];case=CASES[c['objective']]
  for delayed in [False,True]:
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(artifacts,receipt),artifacts=artifacts,author_validation=dict(report=receipt,checks=checks_for(commands,case)),closure_rationale=rationales[case],remaining_author_work=[],evidence_limits=report['limits']))
 ledger=read(ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json');parent=next(x for x in ledger['gaps'] if x['id']=='LRUN-15')
 rows.append(dict(id='LRUN-15',original_gap=parent,dimensions=dimensions(artifacts,receipt),artifacts=artifacts,author_validation=dict(report=receipt,checks=[x for x in commands if any(x.endswith('.'+c) for c in CASES.values()) and '.learner.' not in x]),closure_rationale='All nine original ordinary/changed author packets now provide actual controlled histories or explicit policy proofs; enrollment uses synchronized observations, not sleeps. Existing independent learner and source-edition gaps are not collapsed.',remaining_author_work=[],evidence_limits=report['limits']))
 result=dict(schema_version=1,scope='Eighteen exact original criterion/variant IDs plus existing parent LRUN-15; author dimensions only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[receipt,'research/v5/expansion/legacy_thread_waiting/exports.json'],updates=rows,remaining_parents=['LRUN-16: T25.06–.09 memory-model graphs, atomics/helpers, CAS and target comparisons remain separate.','LRUN-17: T25.10–.12 progress histories, false-sharing measurements and broader validation remain separate.','Exact published later-edition comparison remains open in LRUN-02 and the T25.01 lifecycle package.'],summary=dict(criteria=9,delayed=9,parents=1,commands=len(report['commands']),export_commands=len(exports['commands'])))
 write(HERE/'completion_dispositions.json',result);print(result['summary'])
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
