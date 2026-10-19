"""Preserve original pipeline identities while binding executed changed models."""
import argparse,copy,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_pipeline'
from research.v5.expansion.legacy_architecture_pipeline.author_support import attach
PROJECT='PR-T37-04';RECEIPT='research/v5/expansion/legacy_architecture_pipeline/validation.json';SUPPORT='research/v5/expansion/legacy_architecture_pipeline/completion_dispositions.json';EXERCISE='curriculum/v5/exercises/X-T37-001.md'
ARTIFACTS=['labs/architecture_pipeline/CONTRACTS.md','assessor/architecture_pipeline/ANALYSIS.md','assessor/architecture_pipeline/pipeline.c','assessor/architecture_pipeline/oracle.py','labs/architecture_pipeline/tests.c','labs/architecture_pipeline/distinctions.json','labs/architecture_pipeline/race_examples.c','assessor/architecture_pipeline/generate.py','assessor/b045_systems/systems.c']
RATIONALES={'T37.06.a':'Actual dynamic fetched-instance certificate and old-producer comparison, complete stage occupancy and availability, two-cycle load stage, changed register-file ordering and harmless intended defects.','T37.06.b':'Same retired work with distinct fetched/refetched instances, full forwarding matrix, explicit no-ID-forwarding branch resolution, held fetch versus new shared-port access and isolated structural conflict.','T37.06.c':'Source-inspected C data-race analysis kept separate from sequential RAW and pipeline timing; actual renamed out-of-order issue with per-version operands, nonpipelined load port and independently checked in-order retirement.'}
REMAINING=['PR-T37-05 through PR-T37-08 original and changed mechanisms remain separately required.','External review, independent seven-day learner, physical observation, exact final C17/C23 and new external textbook inspection remain unclaimed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T37.06.a':['.schedule','.matrix','.boundaries','.mutation.LOAD','.mutation.READ_ORDER','.mutation.PRIOR','thumb.'],'T37.06.b':['.schedule','.matrix','.mutation.SQUASH','.mutation.PORT','thumb.'],'T37.06.c':['distinctions.oracle','.race.compile','.ooo','.mutation.TAG','thumb.']}
 return dict(report=RECEIPT,checks=[n for n in commands if any(p in n for p in patterns[objective])])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def all_rows(r):
 d=read(canon());commands=[c['id'] for c in r['commands']];rows=[]
 def base(ident,objective,task,variant,days):return dict(id=ident,parent_project=PROJECT,objective=objective,original_task=task,original_delayed_variant=variant,minimum_days=days,dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,objective),closure_rationale=RATIONALES[objective],remaining_author_work=[],evidence_limits=r['limits'])
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(base(v['id'] if delayed else c['id'],c['objective'],v['task'] if delayed else c['task'],v['id'],v['minimum_days']))
 text=(ROOT/EXERCISE).read_text(encoding='utf8');meta=json.loads(text.split('```json\n',1)[1].split('\n```',1)[0]);problem=text.split('## Problem\n',1)[1].split('\n## ',1)[0].strip()
 root=base(meta['id'],'T37.06.a',problem,None,7);root.update(original_source=EXERCISE,original_source_id=meta['id'],objectives=meta['objectives'],author_validation=dict(report=RECEIPT,checks=commands),closure_rationale='All three explicit pipeline facets and both original changed-condition mechanisms are implemented and author-executed; finite model evidence only.');rows.append(root)
 for criterion in meta['rubric']:
  row=base(criterion['id'],criterion['objective'],criterion['criterion'],None,7);row.update(original_source=EXERCISE,original_source_id=criterion['id']);rows.append(row)
 variants=text.split('## Variants\n',1)[1].split('\n## ',1)[0].strip()
 for ident,objective in [('V1','T37.06.a'),('V2','T37.06.b')]:
  task=re.search(r'\b'+ident+r': (.*?)(?= V[12]:|$)',variants,re.S).group(1).strip();row=base(meta['id']+'-'+ident,objective,task,ident,meta['delayed_transfer']['minimum_days']);row.update(original_source=EXERCISE,original_source_id=ident,original_variant_id=ident);rows.append(row)
 return rows
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-pipeline.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/architecture_pipeline').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/b045_systems/include/systems.h','labs/b045_systems/CONTRACTS.md','labs/containers_depth/byte_helpers.c']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T37',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/b045_systems/CONTRACTS.md'],source_register='research/v5/expansion/legacy_architecture_pipeline/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact ordinary/changed pipeline mechanisms plus X-T37-001 three rubrics and original V1/V2 mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive pipeline attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_architecture_pipeline.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith(('attempt','root_'))}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-pipeline.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;rows=all_rows(r);assert len(rows)==12
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three original legacy criterion/V1 pairs, exact X-T37-001 root/three rubrics and namespaced V1/V2',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_architecture_pipeline/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(legacy_criteria=3,legacy_delayed=3,sidecar_root=1,sidecar_rubrics=3,sidecar_delayed=2,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact pipeline dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
