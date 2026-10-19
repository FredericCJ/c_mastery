"""Package the three exact T37.02 criteria and their unchanged V1 tasks."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_alu'
from research.v5.expansion.legacy_architecture_alu.author_support import attach
PROJECT='PR-T37-01';RECEIPT='research/v5/expansion/legacy_architecture_alu/validation.json';SUPPORT='research/v5/expansion/legacy_architecture_alu/completion_dispositions.json'
ARTIFACTS=['labs/architecture_alu/CONTRACTS.md','assessor/architecture_alu/ANALYSIS.md','assessor/architecture_alu/alu.c','assessor/architecture_alu/x86.S','assessor/architecture_alu/thumb.S','assessor/architecture_alu/generate.py','assessor/architecture_alu/oracle.py','labs/architecture_alu/tests.c']
RATIONALES={'T37.02.a':'Actual Boolean truth table and every carry connection, unchanged prior-function comparison, five-bit generalization with admitted promoted shift bounds.','T37.02.b':'All 5120 ordinary/extra-input model packets, independent modular/signed-range counts, Z/N and rejected-output preservation; ignored-borrow and swapped-wire mutants fail intended assertions.','T37.02.c':'Actual x86-64 and emulated Cortex-M4 32-bit comparisons, 1280 signed/unsigned relations/profile, precise carry polarity and native branch encodings; inspected LE table conflict falsified by equality.'}
REMAINING=['PR-T37-02 through PR-T37-08 original and changed mechanisms remain separately required.','External review, independent seven-day learner, physical observation and exact final published C17/C23 remain unclaimed.']

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T37.02.a':['.truth','.exhaustive','thumb.'],'T37.02.b':['.exhaustive','.admission','.mutation.','thumb.'],'T37.02.c':['.branches','.encoding.','.mutation.SIGNED','.disassembly','thumb.']}
 return dict(report=RECEIPT,checks=['generate',*[n for n in commands if any(p in n for p in patterns[objective])]])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-alu.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/architecture_alu').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/algorithms/include/algo.h','labs/containers_depth/byte_helpers.c']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T37',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_architecture_alu/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact ordinary/changed Boolean ALU, carry/borrow, admission and native/Thumb branch mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive ALU attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_architecture_alu.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-alu.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Exact three original T37.02 criteria and three original V1 identities',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_architecture_alu/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact ALU dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
