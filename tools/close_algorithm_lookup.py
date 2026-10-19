"""Package the three exact T36.08 criteria and their unchanged V1 tasks."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_lookup'
from research.v5.expansion.legacy_algorithm_lookup.author_support import attach
PROJECT='PR-T36-06';RECEIPT='research/v5/expansion/legacy_algorithm_lookup/validation.json';SUPPORT='research/v5/expansion/legacy_algorithm_lookup/completion_dispositions.json'
ARTIFACTS=['labs/algorithm_lookup/CONTRACTS.md','assessor/algorithm_lookup/ANALYSIS.md','assessor/algorithm_lookup/lookup.c','assessor/algorithm_lookup/x86.S','assessor/algorithm_lookup/thumb.S','assessor/algorithm_lookup/oracle.py','labs/algorithm_lookup/tests.c']
RATIONALES={'T36.08.a':'Actual typed12-byte System V/Thumb probes, bounded full/deleted and clustered prior tables, independently checked fixture paths and executed incomplete quadratic coverage trace.','T36.08.b':'Original all-overlap byte C implementation and two ABI kernels, distinct required-count versus unchanged-count failure translation, actual synchronous callback failure/cancellation with committed-prefix semantics.','T36.08.c':'Independent actual945 whole-span results/comparison packets and4815 streaming split packets per profile, all short capacities/canaries and actual prior prefix-function composition with overlap mutation rejection.'}
REMAINING=['PR-T36-07/08 original and changed tasks remain independently required; no whole-domain closure.','All T37 original/changed tasks remain separate.','External review, independent seven-day learner, physical observation and final published C17/C23 remain unclaimed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T36.08.a':['.lookup','.prior','.mutation.TOMBSTONE','thumb.'],'T36.08.b':['.matching','.callbacks','.mutation.COUNT','.mutation.CANCEL','.disassembly','.core','thumb.'],'T36.08.c':['.matching','.stream','.mutation.OVERLAP','.mutation.PREFIX','thumb.']}
 return dict(report=RECEIPT,checks=['fixture',*[n for n in commands if any(p in n for p in patterns[objective])]])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-lookup.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/algorithm_lookup').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','labs/containers_depth/byte_helpers.c','labs/algorithm_depth/algorithms.h','labs/algorithm_depth/CONTRACTS.md']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T36',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_algorithm_lookup/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact ordinary/changed T36.08 mechanisms: actual typed assembly, bounded quadratic trace, failure publication and actual prerequisite prefix composition.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive lookup attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_algorithm_lookup.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-lookup.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Exact three original T36.08 criteria and three original V1 identities',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_algorithm_lookup/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact lookup dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
