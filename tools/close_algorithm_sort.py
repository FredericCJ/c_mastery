"""Original three sorting criteria and three exact changed-condition identities."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_sort'
from research.v5.expansion.legacy_algorithm_sort.author_support import attach
PROJECT='PR-T36-03';RECEIPT='research/v5/expansion/legacy_algorithm_sort/validation.json';SUPPORT='research/v5/expansion/legacy_algorithm_sort/completion_dispositions.json'
ARTIFACTS=['labs/algorithm_sort/CONTRACTS.md','assessor/algorithm_sort/ANALYSIS.md','assessor/algorithm_sort/fallible.c','assessor/algorithm_sort/adapter.c','assessor/algorithm_sort/wide_x86.S','assessor/algorithm_sort/wide_thumb.S','assessor/algorithm_sort/generate.py','assessor/algorithm_sort/oracle.py']
RATIONALES={'T36.05.a':'Actual assembly prefix observations plus full-record rank/multiset and pending-hole arguments; original stable left-tie merge checkpoint contrasted with inspected source right-tie, executed descending stable variant.','T36.05.b':'Actual frozen T15 insertion/merge/heap producers composed through a derived unsigned-to-signed order bijection on n<=16; new n<=32 assembly and 64bit tags with revised16byte stride execute on both ABIs.','T36.05.c':'Actual132-row assembly-derived count tables independently checked by inversion/stopping-predecessor oracle, separate shifts/placements/comparisons;18 staged callback failures, explicit context rollback limits and intended mutants.'}
REMAINING=['T36 PR04–08 original and changed obligations remain independently required; no whole-domain closure.','All T37 original/changed obligations remain separate.','External review, independent seven-day learner, physical observation and final published C17/C23 remain unclaimed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T36.05.a':['.insertion','.descending','.prefix.oracle','.prior','.mutation.UNSTABLE','thumb.'],'T36.05.b':['.prior','.wide','.mutation.WIDE','.disassembly','.kernel.','thumb.'],'T36.05.c':['.counts','.metric.oracle','.failure','.mutation.COUNTS','.mutation.FAILURE','.core.','thumb.']}
 return dict(report=RECEIPT,checks=['fixture','generate',*[n for n in commands if any(p in n for p in patterns[objective])]])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-sort.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/algorithm_sort').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/algorithms/include/algo.h','labs/algorithms/src/x86_64.S','labs/algorithms/src/thumb.S','labs/algorithms/tests/abi_x86_64.S','labs/algorithms/tests/abi_thumb.S','labs/algorithms/adapters/checked_asm.c','labs/algorithms/thumb/link.ld','labs/containers_depth/byte_helpers.c','labs/algorithm_depth/algorithms.h','labs/algorithm_depth/CONTRACTS.md','labs/containers_depth/containers.h']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T36',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_algorithm_sort/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact three original/changed sorting tasks; actual native/target kernels and frozen prior-sort reuse with explicit domain adapters.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive sorting attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_algorithm_sort.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-sort.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Exact three original T36.05 criteria and three original delayed identities',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_algorithm_sort/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact sorting dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
