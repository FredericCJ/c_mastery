"""Exact legacy and sidecar identity bindings for actual commit extensions."""
import argparse,copy,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_commit'
from research.v5.expansion.legacy_architecture_commit.author_support import attach
PROJECT='PR-T37-03';RECEIPT='research/v5/expansion/legacy_architecture_commit/validation.json';SUPPORT='research/v5/expansion/legacy_architecture_commit/completion_dispositions.json';EXERCISE='curriculum/v5/exercises/X-T37-002.md'
ARTIFACTS=['labs/architecture_commit/CONTRACTS.md','assessor/architecture_commit/ANALYSIS.md','assessor/architecture_commit/commit.c','assessor/architecture_commit/oracle.py','labs/architecture_commit/tests.c','labs/architecture_commit/evidence.json','labs/systems_certificates/include/systems_certificates.h','assessor/systems_certificates/systems_certificates.c','assessor/architecture_alu/x86.S','assessor/architecture_alu/thumb.S']
RATIONALES={'T37.01.a':'Actual unchanged eleven-transition producer and stable HALT, with explicit ordered external-input counterexample and independently reconstructed trace.','T37.01.b':'Source-inspected zero-status extension and65536 actual native/emulated comparisons, with exact toy update policy distinct from real register/PC/timing semantics.','T37.01.c':'Source-bound host, link, disassembly and emulator evidence classified separately; clearly synthetic contradictory-board investigation preserves bounded-model results without inventing physical evidence.','T37.05.a':'Actual old-state alias/rollback producer plus owned pending load, ready/stall/cancellation precedence, postcommit no-pending response and tick/commit separation.','T37.05.b':'Full operand/control/commit trace, budget4+7 and invalid-third partial progress; failed clock preserves state/control and actual mutation exposes premature PC publication.','T37.05.c':'Owned speculative candidate and deferred fault, separate branch resolve/successor retirement, wrong-path store squash and stale-origin/retirement detection, independently checked against committed transitions.'}
REMAINING=['PR-T37-04 through PR-T37-08 original and changed mechanisms remain separately required.','External review, independent seven-day learner, physical observation and exact final C17/C23 remain unclaimed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T37.01.a':['.baseline','.input','thumb.'],'T37.01.b':['.status','.mutation.ZERO','.disassembly','thumb.'],'T37.01.c':['evidence.oracle','.baseline','.disassembly','thumb.'],'T37.05.a':['.baseline','.load','.clocked','.mutation.CANCEL','thumb.'],'T37.05.b':['.baseline','.clocked','.mutation.ALIAS','.mutation.ATOMIC','thumb.'],'T37.05.c':['.speculation','.mutation.SQUASH','thumb.']}
 return dict(report=RECEIPT,checks=[n for n in commands if any(p in n for p in patterns[objective])])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def all_rows(r):
 d=read(canon());commands=[c['id'] for c in r['commands']];rows=[]
 def base(ident,objective,task,variant,days):return dict(id=ident,parent_project=PROJECT,objective=objective,original_task=task,original_delayed_variant=variant,minimum_days=days,dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,objective),closure_rationale=RATIONALES[objective],remaining_author_work=[],evidence_limits=r['limits'])
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(base(v['id'] if delayed else c['id'],c['objective'],v['task'] if delayed else c['task'],v['id'],v['minimum_days']))
 text=(ROOT/EXERCISE).read_text(encoding='utf8');meta=json.loads(text.split('```json\n',1)[1].split('\n```',1)[0]);problem=text.split('## Problem\n',1)[1].split('\n## ',1)[0].strip()
 root=base(meta['id'],'T37.01.a',problem,None,7);root.update(original_source=EXERCISE,original_source_id=meta['id'],objectives=meta['objectives'],author_validation=dict(report=RECEIPT,checks=commands),closure_rationale='Actual original core plus all six explicitly mapped objective facets and both original changed mechanisms; finite author evidence only.');rows.append(root)
 for criterion in meta['rubric']:
  row=base(criterion['id'],criterion['objective'],criterion['criterion'],None,7);row.update(original_source=EXERCISE,original_source_id=criterion['id']);rows.append(row)
 variants=text.split('## Variants\n',1)[1].split('\n## ',1)[0].strip()
 for ident,objective in [('V1','T37.01.a'),('V2','T37.05.a')]:
  task=re.search(r'\b'+ident+r': (.*?)(?= V[12]:|$)',variants,re.S).group(1).strip();row=base(meta['id']+'-'+ident,objective,task,ident,meta['delayed_transfer']['minimum_days']);row.update(original_source=EXERCISE,original_source_id=ident,original_variant_id=ident);rows.append(row)
 return rows
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-commit.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/architecture_commit').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/systems_certificates/include/systems_certificates.h','labs/systems_certificates/CONTRACTS.md','labs/systems_certificates/src/systems_certificates.c','labs/architecture_alu/alu.h','labs/architecture_alu/learner_x86.S','labs/architecture_alu/learner_thumb.S','labs/algorithms/include/algo.h','labs/containers_depth/byte_helpers.c']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T37',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/systems_certificates/CONTRACTS.md'],source_register='research/v5/expansion/legacy_architecture_commit/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Six exact ordinary/changed clocked-state mechanisms plus X-T37-002 six rubrics and original V1/V2 mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive commit attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_architecture_commit.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith(('attempt','root_'))}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-commit.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;rows=all_rows(r);assert len(rows)==21
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Six original legacy criterion/V1 pairs, exact X-T37-002 root/six rubrics and namespaced V1/V2',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_architecture_commit/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(legacy_criteria=6,legacy_delayed=6,sidecar_root=1,sidecar_rubrics=6,sidecar_delayed=2,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS21 exact commit dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
