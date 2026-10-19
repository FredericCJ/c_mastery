"""Six exact original worker objectives and their changed-condition identities."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_security_worker'
from research.v5.expansion.legacy_security_commands.author_support import attach
PROJECT='PR-T35-02';RECEIPT='research/v5/expansion/legacy_security_worker/validation.json';SUPPORT='research/v5/expansion/legacy_security_worker/completion_dispositions.json'
ARTIFACTS=['labs/security_depth/worker/CONTRACTS.md','assessor/security_depth/worker/ANALYSIS.md','assessor/security_depth/worker/worker.c','assessor/security_depth/worker/models.py','labs/security_depth/worker/echo_worker.c','assessor/posix_lifecycle_depth/namespace/path.c','labs/security_depth/command/process_boundary.c','research/v5/expansion/b06/boot_pair_validation.json']
CASES={'T35.05.a':['race'],'T35.05.b':['inheritance'],'T35.05.c':['inheritance'],'T35.08.a':['policy','inheritance','process'],'T35.08.b':['policy','inheritance'],'T35.08.c':['process']}
RATIONALES={'T35.05.a':'Actual acknowledged final-name replacement and changed intermediate-link resolution expose wrong synthetic object; retained descriptor, exclusive creation and actual pn_open_confined repair under precise policy.','T35.05.b':'Actual exec fixture inventory0/4, excluded3 and high parent descriptor; four acquisition faults and owned missing-executable cleanup, plus complete three-order logical open/fcntl/fork counterexample.','T35.05.c':'Actual explicit argv/env transports spaces/semicolon/dollar-parenthesis literally; original child --admin parser distinguishes option interpretation from shell interpretation and -- delimiter.','T35.08.a':'Actual worker read and EBADF through O_RDONLY, same-process/private-child comparison and source-bound MPU design; actual child reopens only its owned writable path, exposing narrow guarantee.','T35.08.b':'Actual application read-log gate denies write-config; revoked read returns policy EACCES even with invalid descriptor before kernel access, outputs preserved.','T35.08.c':'Explicit ISO/diagnostic/runtime/OS/discipline enforcement account, actual private mappings and descriptor rights, verified nonoverlapping MPU regions and retained actual MPS2 faults with privileged/DMA limits.'}
REMAINING=['T35 PR-T35-04/05/06 hardening, secret lifetime, artifact identity/authenticity and review remain required separate work.','T36/T37 implementation and changed tasks remain required separate packages.','External review, independent delayed learner, physical observations and exact final published C17/C23 remain open.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):return dict(report=RECEIPT,checks=['models']+[n for n in commands if n in [cc+'.'+p+'.'+case for cc in ['gcc','clang'] for p in ['debug','optimized','sanitized'] for case in CASES[objective]]])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-worker.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/security_depth/worker').rglob('*') if p.is_file())+['labs/posix_lifecycle_depth/namespace/path.h','labs/posix_lifecycle_depth/namespace/path_stub.c','labs/posix_lifecycle_depth/namespace/CONTRACTS.md','labs/security_depth/command/process_boundary.c','labs/firmware_lifecycle/targets/boot_guest.S']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T35',key='security_worker_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/posix_lifecycle_depth/namespace/CONTRACTS.md'],source_register='research/v5/expansion/legacy_security_worker/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact original T35.05/.08 ordinary and changed tasks, actual local OS authority and source-bound prior selected MPU observations; no complete isolation or independent assessment claim.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive worker attachment, original criteria unchanged')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_security_worker.py',ROOT/'research/v5/expansion/legacy_security_commands/author_support.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-worker.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Six exact original worker criteria and six delayed identities; bounded author implementation/execution only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_security_worker/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact worker dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
