"""Bind all three exact original hardening criteria and delayed variants."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_security_hardening'
from research.v5.expansion.legacy_security_commands.author_support import attach
PROJECT='PR-T35-04';RECEIPT='research/v5/expansion/legacy_security_hardening/validation.json';SUPPORT='research/v5/expansion/legacy_security_hardening/completion_dispositions.json'
ARTIFACTS=['labs/security_depth/hardening/CONTRACTS.md','assessor/security_depth/hardening/ANALYSIS.md','assessor/security_depth/hardening/repair.c','assessor/security_depth/hardening/audit.py','assessor/security_depth/hardening/target.c','labs/security_depth/hardening/flawed.c','labs/security_depth/hardening/guard_control.c']
RATIONALES={'T35.07.a':'Actual distinct Linux ELF/maps and eligible check sites, bounded fortify/ASan defect diagnostics plus repaired controls, defined actual guard mismatch, and data-only synthetic authority violation surviving all selected variants.','T35.07.b':'Actual GCC/Clang aggregate support diagnostics, post-optimization site absence, read-only ASLR/maps,31 timing samples per variant and actual musl1.2.5 header/runtime contrast with installed glibc.','T35.07.c':'Actual hosted libc/loader/startup traces, strict Arm unresolved guard interface rejection then original fixed-guard/terminal-handler link closure, native bounded record execution and intended missing-record rejection.'}
REMAINING=['T35 PR-T35-05/06 secret lifetime, artifact identity/authenticity and review remain required separate work.','T36/T37 implementation and changed tasks remain required separate packages.','External review, independent delayed learner, physical observations and exact final published C17/C23 remain open.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 if objective.endswith('.a'):checks=[n for n in commands if n.endswith(('.bounds','.policy','.defect','.guard','.headers','.programs','.dynamic','.symbols','.disassembly')) or '.maps.' in n]
 elif objective.endswith('.b'):checks=[n for n in commands if n in ['gcc.hardened.options','clang.unsupported','aslr.policy','page.size'] or n.endswith(('.preprocessor','.disassembly','.bench','.size')) or n.startswith('musl.static.')]
 else:checks=[n for n in commands if n.startswith('arm.') or n.endswith(('.handler','.handler.mutant','.handler.stub','.link_trace','.dynamic','.programs'))]
 return dict(report=RECEIPT,checks=['fixture',*checks])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-hardening.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/security_depth/hardening').rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T35',key='security_hardening_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_security_hardening/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact original T35.07 ordinary and changed tasks with actual binaries/maps/libc comparison and strict target link closure; no target startup/physical or independent assessment claim.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive hardening attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_security_hardening.py',ROOT/'research/v5/expansion/legacy_security_commands/author_support.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-hardening.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three exact original hardening criteria and three delayed identities; bounded author implementation/execution only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_security_hardening/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact hardening dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
