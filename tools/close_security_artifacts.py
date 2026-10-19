"""Six original artifact/review criteria plus six exact delayed identities."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_security_artifacts'
from research.v5.expansion.legacy_security_commands.author_support import attach
PROJECT='PR-T35-06';RECEIPT='research/v5/expansion/legacy_security_artifacts/validation.json';SUPPORT='research/v5/expansion/legacy_security_artifacts/completion_dispositions.json'
ARTIFACTS=['labs/security_depth/artifacts/CONTRACTS.md','assessor/security_depth/artifacts/ANALYSIS.md','assessor/security_depth/artifacts/artifacts.c','assessor/security_depth/artifacts/service.c','assessor/security_depth/artifacts/fixture.py','labs/security_depth/artifacts/release_probe.c']
RATIONALES={'T35.11.a':'Actual SHA256 fixture and byte/digest substitution, source/tool/dependency/binary identities and valid untrusted signer counterexample.','T35.11.b':'Actual Ed25519 distinct digest/signature/trust/version/epoch/local-commit predicates, four provider-boundary failures, authenticated monotonic rotation and immediate old-key revocation.','T35.11.c':'Two actual identical same-toolchain flawed builds reproduce unauthorized synthetic effect; repaired artifact differs, GCC/Clang instruction/section disagreement inspected without preferred-hash or trust inference.','T35.12.a':'Eight concrete reviewed boundaries in actual parser/queue/descriptor/clearing composition, new opcode with separate retained-byte quota and intended quota mutant killed.','T35.12.b':'Exact original packet/event/callback attacker table and live shared-process authority mutation gives only synthetic effect; host compromise excluded and logical boundary reopened.','T35.12.c':'Actual fresh-trust adapter, cached-report counterexample and independent operation authority, ordinary success plus malformed/denied/overbudget/stale/cancel/descriptor cleanup and changed count/width traces.'}
REMAINING=['T36/T37 implementation and changed tasks remain required separate packages.','External review, independent delayed learner, physical observations and exact final published C17/C23 remain open.','Artifact commit/rotation is logical serialized local state; durable installation, full provider security and process-isolation claims are outside this contract.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 suffix=objective.removeprefix('T35.');patterns={'11.a':['.artifact','.dynamic'],'11.b':['.artifact','.rotation','.failure.'],'11.c':['.release.'],'12.a':['.service','.mutation.QUOTA'],'12.b':['.shared'],'12.c':['.service','.integrated','.stub.','.mutation.']}
 checks=[n for n in commands if any(p in n for p in patterns[suffix])]
 return dict(report=RECEIPT,checks=['fixture',*checks])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-artifacts.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/security_depth/artifacts').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/security_depth/command/command.h','labs/security_depth/command/CONTRACTS.md','labs/security_depth/command/learner.c','labs/security_depth/worker/worker.h','labs/security_depth/worker/CONTRACTS.md','labs/security_depth/secrets/secrets.h','labs/security_depth/secrets/platform.c','labs/security_depth/secrets/CONTRACTS.md']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T35',key='security_artifacts_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_security_artifacts/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact six original artifact/review and delayed tasks; actual provider policy plus service composition; no physical install, provider certification or isolation inference.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive artifacts attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_security_artifacts.py',ROOT/'research/v5/expansion/legacy_security_commands/author_support.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-artifacts.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Six exact original artifact/review criteria and six delayed identities; bounded author implementation/execution only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_security_artifacts/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact artifacts dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
