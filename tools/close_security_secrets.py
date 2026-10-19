"""Exact three original secret-lifetime criteria and delayed variants."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_security_secrets'
from research.v5.expansion.legacy_security_commands.author_support import attach
PROJECT='PR-T35-05';RECEIPT='research/v5/expansion/legacy_security_secrets/validation.json';SUPPORT='research/v5/expansion/legacy_security_secrets/completion_dispositions.json'
ARTIFACTS=['labs/security_depth/secrets/CONTRACTS.md','assessor/security_depth/secrets/ANALYSIS.md','assessor/security_depth/secrets/secrets.c','assessor/security_depth/secrets/provider.c','assessor/security_depth/secrets/oracle.py','labs/security_depth/secrets/clearing.c','labs/security_depth/secrets/platform.c','assessor/replicated/hashing/hashing.c']
RATIONALES={'T35.09.a':'Actual separate original/temp/pool/allocated/unmanaged objects, pre-free zero observation, retained explicit primitive and optimized ordinary dead-store inspection; actual C23 positive and C17/musl declaration-negative probes.','T35.09.b':'Actual early1/4/4 versus fixed4/4/4 and1024 changed-byte cases; fixed-eight-access original indexed table trace exposes address dependence, with six compiler/optimization/diagnostic instruction observations and no machine timing inference.','T35.09.c':'Explicit educational/deployment claim boundary and separately declared profileA OpenSSL3.5.5 adapter;65 lengths compared against original first-principles HMAC and Python oracle, seven acquisition/provider/publication failures, exact handle cleanup and owned key-copy clearing.'}
REMAINING=['T35 PR-T35-06 artifact identity/authenticity and security review remain required separate work.','T36/T37 implementation and changed tasks remain required separate packages.','External review, independent delayed learner, physical observations and exact final published C17/C23 remain open.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 if objective.endswith('.a'):checks=[n for n in commands if n.endswith(('.lifetime','.disassembly','.undefined','.unavailable','.c23.execute'))]
 elif objective.endswith('.b'):checks=[n for n in commands if n.endswith(('.work','.addresses','.disassembly','.mutation.EARLY'))]
 else:checks=[n for n in commands if '.provider' in n or n=='openssl']
 return dict(report=RECEIPT,checks=['fixture',*checks])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-secrets.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/security_depth/secrets').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/replicated/hashing/hashing.h','labs/replicated/hashing/HASHING.md']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T35',key='security_secrets_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_security_secrets/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact original T35.09 ordinary/changed tasks, bounded actual copy/work/clearing and provider integration; no universal erasure, physical timing or deployment approval.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive secrets attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_security_secrets.py',ROOT/'research/v5/expansion/legacy_security_commands/author_support.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-secrets.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three exact original secret-lifetime criteria and three delayed identities; bounded author implementation/execution only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_security_secrets/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact secrets dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
