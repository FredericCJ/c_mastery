"""Exact original six assembly/interface criteria and six changed identities."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_abi'
from research.v5.expansion.legacy_algorithm_abi.author_support import attach
PROJECT='PR-T36-02';RECEIPT='research/v5/expansion/legacy_algorithm_abi/validation.json';SUPPORT='research/v5/expansion/legacy_algorithm_abi/completion_dispositions.json'
ARTIFACTS=['labs/algorithm_abi/CONTRACTS.md','assessor/algorithm_abi/ANALYSIS.md','assessor/algorithm_abi/bound.c','assessor/algorithm_abi/x86.S','assessor/algorithm_abi/thumb.S','assessor/algorithm_abi/generate.py','assessor/algorithm_abi/oracle.py']
RATIONALES={'T36.03.a':'Actual original SysV and Thumb lower kernels, native C/linear oracles and emulator execution; original Microsoft leaf with exercised register/home-store boundary, explicitly no Windows OS execution.','T36.03.b':'Complete original probe live-register analysis, actual nested callback kernels preserving all call-crossing state, both poison patterns and target alignment, intended callback-scratch mutant.','T36.03.c':'Separate preserved-register and extra-load defects; four protected-page child queries across two layouts, changed seed/scratch probes at both targets, exact bounded failure distinctions.','T36.04.a':'Actual complete fifteen-step certificate fixture checked independently by partition/bisect and source C; upper/lower duplicate distinction and progress derivation.','T36.04.b':'Every load index/scaling proof and actual eight-byte record kernels using target-width size_t; maximum-size rejection before access on native64 and target32.','T36.04.c':'Executed minimal one-high-bit-element versus zero signed-branch discriminator; separate4validation/3search counts and immutable trusted boundary with equivalent-contract cost derivation.'}
REMAINING=['Other T36 original/changed projects remain independently required; no whole-domain closure inferred.','All T37 original/changed obligations remain separate.','External review, independent seven-day learner, physical observation, Windows OS execution and final published C17/C23 remain unclaimed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T36.03.a':['.search','.windows','thumb.','.disassembly','.oracle'],'T36.03.b':['.abi','.callback','.mutation.CALLBACK','thumb.'],'T36.03.c':['.mutation.ABI','.overread.','.guard','.changed.','thumb.changed','thumb.ABI','generate'],'T36.04.a':['.search','.oracle','thumb.'],'T36.04.b':['.records','.scale','.mutation.SCALE','.core.','.kernel.','thumb.'],'T36.04.c':['.signedness','.mutation.SIGNED','.search','.oracle','thumb.SIGNED']}
 return dict(report=RECEIPT,checks=['fixture',*[n for n in commands if any(p in n for p in patterns[objective])]])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-abi.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/algorithm_abi').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/algorithms/include/algo.h','labs/algorithms/src/x86_64.S','labs/algorithms/src/thumb.S','labs/algorithms/tests/abi_x86_64.S','labs/algorithms/tests/abi_thumb.S','labs/algorithms/thumb/link.ld','labs/containers_depth/byte_helpers.c']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T36',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_algorithm_abi/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact six original/changed assembly/invariant/ABI tasks, bounded native and emulator author execution; not independent learner or physical acceptance.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive ABI attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_algorithm_abi.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-abi.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Exact six original T36.03/.04 criteria and six original delayed identities',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_algorithm_abi/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact ABI dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
