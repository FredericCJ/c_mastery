"""Add and bind the three original T25.02 mechanisms and exact variants."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_thread_sharing';CANON=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-01.json'
from research.v5.expansion.legacy_threads.author_support import attach
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def integrate():
 before=read(CANON);snapshot=HERE/'PR-T25-01.before-sharing.json'
 if not snapshot.exists():snapshot.write_bytes(CANON.read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/thread_depth/sharing').rglob('*') if p.is_file())
 write(HERE/'attachment.json',dict(project=before['project'],key='thread_sharing_successor',supplied_artifacts=public,shared_contracts=['labs/thread_depth/sharing/CONTRACTS.md'],source_register='research/v5/expansion/legacy_thread_sharing/resources.json',support='research/v5/expansion/legacy_thread_sharing/completion_dispositions.json',criterion_ids=[c['id'] for c in before['criteria'] if c['objective'].startswith('T25.02.')],scope='Three original shared-state tasks and changed-condition snapshot/failure/cache-budget experiments; software execution and physical/exact-source limits separate.'))
 after=attach(copy.deepcopy(before));assert before['criteria']==after['criteria'] and before['baseline_binding']==after['baseline_binding'];assert attach(copy.deepcopy(after))==after;write(CANON,after);print('PASS additive sharing attachment')
def package():
 d=read(CANON);before=read(HERE/'PR-T25-01.before-sharing.json');assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(d))==d
 report=read(HERE/'validation.json');exports=read(HERE/'exports.json');files={CANON,pathlib.Path(__file__),ROOT/'tools/check_thread_sharing.py',ROOT/'research/v5/expansion/legacy_threads/author_support.py'}
 files|={p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in [report,exports]:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for name,h in data[key].items():assert sha(ROOT/name)==h,('stale',name)
  files|={ROOT/p for p in data['source_sha256']}
 commands=[x['id'] for x in report['commands']];receipt='research/v5/expansion/legacy_thread_sharing/validation.json';rows=[];artifacts=['labs/thread_depth/sharing/CONTRACTS.md','assessor/thread_depth/sharing/ANALYSIS.md']
 for c in d['criteria']:
  if not c['objective'].startswith('T25.02.'):continue
  v=c['variants'][0];case={'a':'partition','b':'ownership','c':'histogram'}[c['objective'][-1]]
  for delayed in [False,True]:
   checks=[x for x in commands if x.endswith('.'+case) and '.learner.' not in x]
   if case=='histogram':checks+=[x for x in commands if x.endswith('.measurement')]
   dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',[receipt]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-observed' if case=='histogram' else 'not-applicable-to-declared-host-contract',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,artifacts=artifacts,author_validation=dict(report=receipt,checks=checks),closure_rationale={'partition':'Actual private/shared partition comparison and captured snapshot under concurrent source mutation, preserving the old version.','ownership':'Actual bounded ownership transitions and a gated post-dequeue worker failure with separate queued cancellation and rejected caller ownership.','histogram':'Four original algorithms under one numeric-buffer budget, scalar exact-count oracle and96 raw full-call timing samples across two reported cache regimes and distributions.'}[case],remaining_author_work=[],evidence_limits=report['limits']))
 result=dict(schema_version=1,scope='Six exact original T25.02 criterion/variant rows; T25.01 published-text gap remains separate',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[receipt,'research/v5/expansion/legacy_thread_sharing/exports.json'],updates=rows,remaining_parents=['LRUN-14 lifetime/sharing mechanisms now have separate current packages; exact published-edition delayed source comparison remains open in lifecycle package.','LRUN-15/16/17 are not covered by this shared-state sub-batch.'],summary=dict(criteria=3,delayed=3,commands=len(report['commands']),raw_timing_samples=sum(len(o['samples']) for o in report['observations']),export_commands=len(exports['commands'])))
 write(HERE/'completion_dispositions.json',result);print(result['summary'])
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
