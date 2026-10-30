"""Add exact original T25.10–.12 ordinary/delayed author evidence, not acceptance."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_thread_progress'
from research.v5.expansion.legacy_threads.author_support import attach
PROJECTS=['PR-T25-04','PR-T25-05'];RECEIPT='research/v5/expansion/legacy_thread_progress/validation.json';SUPPORT='research/v5/expansion/legacy_thread_progress/completion_dispositions.json'
ARTIFACTS=['labs/thread_depth/progress/CONTRACTS.md','assessor/thread_depth/progress/ANALYSIS.md',*[f'assessor/thread_depth/progress/{p}' for p in ['progress.c','history.c','history_oracle.py','metrics.c','queue.c','hazards.py','ordering_model.py']]]
CASES={'T25.10.a':['donation','waiting.order'],'T25.10.b':['bound'],'T25.10.c':['effects'],'T25.11.a':['layout'],'T25.11.b':['aggregation','freshness'],'T25.11.c':['cost','aggregation'],'T25.12.a':['stress','waiting.queue','waiting.wake','waiting.deadline'],'T25.12.b':['history','stress'],'T25.12.c':['shutdown']}
EXTRA={'T25.10.a':['hazards.model'],'T25.10.b':['hazards.model'],'T25.10.c':['gcc.effect.imports','clang.effect.imports'],'T25.11.a':['gcc.metrics.disassembly','clang.metrics.disassembly'],'T25.11.b':[],'T25.11.c':['gcc.metrics.disassembly','clang.metrics.disassembly'],'T25.12.a':['gcc.tsan.race','clang.tsan.race','gcc.weak.model','clang.weak.model','handoff.model'],'T25.12.b':['gcc.history.oracle','clang.history.oracle'],'T25.12.c':['ordering.minimize','handoff.model']}
RATIONALES={
 'T25.10.a':'Four distinct finite hazard schedules with repeating-state arguments, actual lock-order repair,160000 independent donation/reachability cases and withdrawal.',
 'T25.10.b':'Counterexample to lock fairness from exclusion or CPU fairness, plus checked conditional deadline admission and refusal of unproved bounds.',
 'T25.10.c':'Whole-path cost/effect analysis, each nonsleepable dependency rejected, actual object allocator/mutex/output imports audited without ISR execution.',
 'T25.11.a':'Actual adjacent/separated addresses, checked byte-extent/line arithmetic, changed line/alignment models and repeated equal-update measurements with no compulsory speedup.',
 'T25.11.b':'Three exact final aggregation schemes with independent sum/checksum/update counts, changed work intensity and actual mutex current-total reader/overflow protocol.',
 'T25.11.c':'Matched empty/useful/composed workloads, clock resolution/overhead, retained optimized code and raw distributions under changed intensity; no additive cost identity assumed.',
 'T25.12.a':'Actual frozen queue forced enrollment,2000 exact-once payloads, prior wake/deadline cases, known race/logical sensitivity and explicit weak-order/reuse model variant.',
 'T25.12.b':'Capacity-three finite history implementation,22680 complete plus12 pending-policy oracle cases and20 recorded actual histories/profile; explicit extension rather than silently dropping pending calls.',
 'T25.12.c':'First-edge trace minimization and forward/reverse lifetime argument, plus real composed settle-then-discard queue shutdown with pending cancellation and external join before destruction.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(project):return ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
def checks_for(commands,objective):
 result=[x for x in commands if any(x.endswith('.'+c) for c in CASES[objective]) and '.learner.' not in x and '.mutation.' not in x]
 return list(dict.fromkeys(result+EXTRA[objective]))
def dimensions():
 return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/thread_depth/progress').iterdir() if p.is_file())+['labs/thread_depth/waiting/waiting.h','labs/thread_depth/waiting/learner.c','labs/thread_depth/waiting/CONTRACTS.md']
 for project in PROJECTS:
  d=read(canon(project));snapshot=HERE/(project+'.before-progress.json')
  if not snapshot.exists():snapshot.write_bytes(canon(project).read_bytes())
  folder=HERE if project==PROJECTS[0] else ROOT/'research/v5/expansion/legacy_thread_validation';folder.mkdir(parents=True,exist_ok=True)
  write(folder/'attachment.json',dict(project=project,key='thread_progress_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/thread_depth/waiting/CONTRACTS.md'],source_register='research/v5/expansion/legacy_thread_progress/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original ordinary and delayed progress/resource/history/shutdown obligations; author execution with separate external dimensions.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS two additive project attachments; original tasks unchanged')
def package():
 report=read(HERE/'validation.json');exports=read(HERE/'exports.json');files={pathlib.Path(__file__),ROOT/'tools/check_thread_progress.py',ROOT/'research/v5/expansion/legacy_threads/author_support.py',ROOT/'research/v5/expansion/legacy_thread_validation/attachment.json',ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json'}
 files|={p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in [report,exports]:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for name,h in data[key].items():assert sha(ROOT/name)==h,('stale',name)
  files|={ROOT/p for p in data['source_sha256']}
 commands=[c['id'] for c in report['commands']];rows=[]
 for project in PROJECTS:
  d=read(canon(project));before=read(HERE/(project+'.before-progress.json'));assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
  for c in d['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=dict(report=RECEIPT,checks=checks_for(commands,c['objective'])),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=report['limits']))
 parent=next(p for p in read(ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json')['gaps'] if p['id']=='LRUN-17')
 rows.append(dict(id='LRUN-17',original_gap=parent,dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=dict(report=RECEIPT,checks=list(dict.fromkeys(x for obj in CASES for x in checks_for(commands,obj)))),closure_rationale='All nine original ordinary/changed author mechanisms have concrete source, bounded actual experiments/models and specific negative controls; learner, external and physical evidence remain separate.',remaining_author_work=[],evidence_limits=report['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Nine original criterion IDs, nine exact delayed IDs and existing LRUN-17; no learner acceptance or historical rewrite',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_thread_progress/exports.json'],updates=rows,remaining_parents=['LRUN-02 and LRUN-16: exact final C17/C23 source obligations remain open.','LRUN-18/19: advanced concurrency/kernel obligations are separate.'],summary=dict(criteria=9,delayed=9,parents=1,commands=len(report['commands']),export_commands=len(exports['commands']))));print('PASS nineteen exact author dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
